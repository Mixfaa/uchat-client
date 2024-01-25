#include "chat-socket/chat_socket.h"
#include <string.h>
#include <stdbool.h>

#include "json-c/arraylist.h"
#include "chat-socket/array_list_utils.h"

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>

typedef void *(*t_thread_func)(void *);

e_transaction_type json_get_transaction_type(json_object *obj)
{
    return (e_transaction_type)json_object_get_int(json_object_object_get(obj, TRANSACTION_TYPE_FIELD));
}

void json_add_base(json_object *obj, e_transaction_type transaction_type)
{
    json_object_object_add(obj, TRANSACTION_TYPE_FIELD, json_object_new_int((int)transaction_type));
}
void json_add_page_limit(json_object *obj, int page, int limit)
{
    json_object_object_add(obj, "page", json_object_new_int(page));
    json_object_object_add(obj, "limit", json_object_new_int(limit));
}
void json_free(json_object *json_obj)
{
    json_object_put(json_obj);
}
t_id json_object_get_id(json_object *json)
{
    return json_object_get_uint64(json);
}
json_object* json_object_new_id(t_id id)
{
    return json_object_new_uint64(id);
}
#pragma region transaction_handler

t_transaction_handler *transaction_handler_new(e_transaction_type target_type, transaction_handler_func handler_func, bool remove_after_handling)
{
    t_transaction_handler *transaction_handler = (t_transaction_handler *)malloc(sizeof(t_transaction_handler));

    if (handler_func == NULL)
        perror("Creating transaction handler with NULL handler\n");
    transaction_handler->handle_func = handler_func;
    transaction_handler->remove_after_handling = remove_after_handling;
    transaction_handler->target_transaction_type = target_type;

    return transaction_handler;
}
void transaction_handler_free(t_transaction_handler *transaction_handler)
{
    free(transaction_handler);
}

#pragma endregion

#pragma region chat_socket

static array_list *split_json_sequence(char *buffer, size_t length)
{
    array_list *list_of_jsons = array_list_new2(free, 5);
    size_t prev_json_end = 0;

    for (size_t i = 0; i < length; i++)
    {
        if (buffer[i] == '\n' && i == length - 1)
        {
            array_list_add(list_of_jsons, strndup(buffer + prev_json_end, i));
            prev_json_end = i;
        }
    }

    return list_of_jsons;
}

static void chat_socket_handle_queue(t_chat_socket *chat_socket)
{
    size_t queue_length = array_list_length(chat_socket->json_queue);
    for (size_t json_index = 0; json_index < queue_length; json_index++)
    {
        json_object *json = array_list_get_idx(chat_socket->json_queue, json_index);
        e_transaction_type transaction_type = json_get_transaction_type(json);

        chat_socket_call_handlers(chat_socket, transaction_type, json);
    }

    array_list_del_idx(chat_socket->json_queue, 0, queue_length);
}

#ifdef CHAT_SOCKET_WAIT_FOR_TRANSACTION_NEW_IMPL
json_object *chat_socket_wait_for_transaction(t_chat_socket *chat_socket, e_transaction_type transaction)
{
    chat_socket_handle_queue(chat_socket);

    json_object *copy_to_return = NULL;
    while (copy_to_return == NULL)
    {
        int data_length = chat_socket_wait_for_data(chat_socket);

        chat_socket_receive_transactions(chat_socket, data_length);

        for (size_t i = 0; i < array_list_length(chat_socket->json_queue); i++)
        {
            json_object *json = array_list_get_idx(chat_socket->json_queue, i);
            e_transaction_type transaction_type = json_get_transaction_type(json);

            if (transaction_type == transaction || transaction == TRANSACTION_STATUS_RESPONSE)
            {
                json_object_deep_copy(json, &copy_to_return, json_c_shallow_copy_default);
                array_list_del_idx(chat_socket->json_queue, i, 1);
                break;
            }
        }
    }

    return copy_to_return;
}
#else
json_object *chat_socket_wait_for_transaction(t_chat_socket *chat_socket, e_transaction_type transaction)
{
    size_t prev_queue_length = array_list_length(chat_socket->json_queue);

    json_object *copy_to_return = NULL;
    while (copy_to_return == NULL)
    {
        int data_length = chat_socket_wait_for_data(chat_socket);

        chat_socket_receive_transactions(chat_socket, data_length);

        for (size_t i = prev_queue_length; i < array_list_length(chat_socket->json_queue); i++)
        {
            json_object *json = array_list_get_idx(chat_socket->json_queue, i);
            e_transaction_type transaction_type = json_get_transaction_type(json);

            if (transaction_type == transaction || transaction == TRANSACTION_STATUS_RESPONSE)
            {
                json_object_deep_copy(json, &copy_to_return, json_c_shallow_copy_default);
                array_list_del_idx(chat_socket->json_queue, i, 1);
                break;
            }
        }
    }

    return copy_to_return;
}
#endif

int chat_socket_wait_for_data(t_chat_socket *chat_socket)
{
    int available = 0;
    while (available <= 0)
    {
        ioctl(chat_socket->socket_fd, FIONREAD, &available);
    }

    return available;
}

void chat_socket_receive_transactions(t_chat_socket *chat_socket, int length)
{
    void *buffer = malloc(length);
    memset(buffer, 0, length);

    int result = recv(chat_socket->socket_fd, buffer, length, 0);
    if (result == -1)
        perror("Error recv from socket\n");
#ifdef CHAT_SOCKET_DEBUG_PRINTS
    printf("msg recv: %s\n", (char *)buffer);
#endif

    array_list *list_of_json_strings = split_json_sequence((char *)buffer, length);

    for (size_t i = 0; i < array_list_length(list_of_json_strings); i++)
    {
        char *json_string = (char *)array_list_get_idx(list_of_json_strings, i);

        if (!strstr(json_string, "\"transaction_type\":\"1\""))
        {
            json_object *json = json_tokener_parse(json_string);
            array_list_add(chat_socket->json_queue, json);
        }

        free(json_string);
    }

    free(buffer);
}

void chat_socket_call_handlers(t_chat_socket *chat_socket, e_transaction_type transaction_type, json_object *json)
{
    for (size_t handler_index = 0; handler_index < array_list_length(chat_socket->transaction_handlers); handler_index++)
    {
        t_transaction_handler *transaction_handler = array_list_get_idx(chat_socket->transaction_handlers, handler_index);
        if (transaction_handler->target_transaction_type == transaction_type || transaction_handler->target_transaction_type == TRANSACTION_STATUS_RESPONSE)
        {
            transaction_handler->handle_func(transaction_type, json);

            if (transaction_handler->remove_after_handling)
                array_list_del_idx(chat_socket->transaction_handlers, handler_index, 1);

            break; // go to next transaction
        }
    }
}

void chat_socket_handle_tick(t_chat_socket *chat_socket)
{
    int available = 0;
    ioctl(chat_socket->socket_fd, FIONREAD, &available);

    if (available <= 0)
        return;

    chat_socket_receive_transactions(chat_socket, available);
    chat_socket_handle_queue(chat_socket);
}

void chat_socket_add_transaction_handler(t_chat_socket *chat_socket, t_transaction_handler *handler)
{
    array_list_add(chat_socket->transaction_handlers, handler);
}

t_chat_socket *chat_socket_connect(int port, in_addr_t addr, int timeout)
{
    t_chat_socket *chat_socket = (t_chat_socket *)malloc(sizeof(t_chat_socket));

    chat_socket->socket_fd = 0;
    memset(&chat_socket->server_addr, 0, sizeof(chat_socket->server_addr));

    if ((chat_socket->socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        perror("Socket creation failed");
        free(chat_socket);
        return NULL;
    }
    int iResult = 0;

    iResult = setsockopt(chat_socket->socket_fd, IPPROTO_TCP, TCP_USER_TIMEOUT, (char *)&timeout, sizeof(timeout));
    if (iResult == -1)
        perror("setsockopt for TCP_USER_TIMEOUT failed with error\n");

    int tcp_nodelay = 1;
    iResult = setsockopt(chat_socket->socket_fd, IPPROTO_TCP, TCP_NODELAY, &tcp_nodelay, sizeof(int));
    if (iResult == -1)
        perror("setsockopt for TCP_NODELAY failed with error\n");

    int sndbuf = 0;
    iResult = setsockopt(chat_socket->socket_fd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(int));
    if (iResult == -1)
        perror("setsockopt for SO_SNDBUF failed with error\n");

    // Initialize sockaddr_in structure
    chat_socket->server_addr.sin_family = AF_INET;
    chat_socket->server_addr.sin_port = htons(port);
    chat_socket->server_addr.sin_addr.s_addr = addr;

    // Connect to the server
    if (connect(chat_socket->socket_fd, (struct sockaddr *)&chat_socket->server_addr, sizeof(chat_socket->server_addr)) == -1)
    {
        perror("Connection failed");
        free(chat_socket);
        return NULL;
    }

    chat_socket->json_queue = array_list_new2((array_list_free_fn *)json_object_put, 32);
    chat_socket->transaction_handlers = array_list_new2((array_list_free_fn *)transaction_handler_free, 10);

    return chat_socket;
}

void chat_socket_send_json(t_chat_socket *chat_socket, json_object *json)
{
    size_t length = 0;
    const char *json_string = json_object_to_json_string_length(json, 0, &length);

    char *buffer = (char *)malloc(length + 2);
    strcpy(buffer, json_string);
    buffer[length] = '\n';
    buffer[length + 1] = '\0';

    send(chat_socket->socket_fd, buffer, length + 1, 0);
#ifdef CHAT_SOCKET_DEBUG_PRINTS
    printf("msg sent: %s\n", buffer);
#endif
    free(buffer);
}

void chat_socket_close(t_chat_socket *chat_socket)
{
    close(chat_socket->socket_fd);
    array_list_free(chat_socket->json_queue);
    array_list_free(chat_socket->transaction_handlers);
}

#pragma endregion

#pragma region account

t_account *account_new(t_id id, const char *username)
{
    t_account *account = (t_account *)malloc(sizeof(t_account));
    account->id = id;
    account->username = strdup(username);

    return account;
}

void account_free(t_account *account)
{
    free((void *)account->username);
    free(account);
}

t_account *json_to_account(json_object *json)
{
    t_account account = json_to_account_stack(json);
    return account_new(account.id, account.username);
}
t_account json_to_account_stack(json_object *json)
{
    t_id id = json_object_get_uint64(json_object_object_get(json, "id"));
    const char *username = json_object_get_string(json_object_object_get(json, "username"));

    t_account account;
    account.id = id;
    account.username = username;

    return account;
}
array_list *json_accounts_to_array_list(json_object *json)
{
    array_list *accounts_jsons = json_object_get_array(json);
    array_list *accounts = array_list_new2((array_list_free_fn *)account_free, accounts_jsons->length);

    for (size_t i = 0; i < array_list_length(accounts_jsons); i++)
        array_list_add(accounts, json_to_account(array_list_get_idx(accounts_jsons, i)));

    return accounts;
}

#pragma endregion
