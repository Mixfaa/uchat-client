#ifndef CHAT_SOCKET_H
#define CHAT_SOCKET_H

#define CHAT_SOCKET_DEBUG_PRINTS
#define CHAT_SOCKET_WAIT_FOR_TRANSACTION_NEW_IMPL

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#ifdef __MACH__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

#include <arpa/inet.h>

#include <stdbool.h>
#include "json-c/json.h"
#include <time.h>
#include "array_list_utils.h"

typedef enum e_message_type
{
    MESSAGE_TYPE_TEXT,
    MESSAGE_TYPE_FILE
} e_message_type;

typedef enum e_transaction_type
{
    TRANSACTION_STATUS_RESPONSE,
    TRANSACTION_HEARTBEAT,
    REQUEST_LOGIN,
    REQUEST_MESSAGE,
    REQUEST_CREATE_CHAT,
    REQUEST_FETCH_ACCOUNTS,
    REQUEST_DELETE_MESSAGE,
    REQUEST_EDIT_MESSAGE,
    REQUEST_FETCH_CHATS,
    REQUEST_FETCH_CHAT_MESSAGES,
    REQUEST_DELETE_CHAT,
    REQUEST_FETCH_ACCOUNTS_BY_IDS,
    REQUEST_FETCH_CHATS_BY_IDS,

    RESPONSE_LOGIN,
    RESPONSE_CHAT,
    RESPONSE_DELETE_CHAT,
    RESPONSE_FETCH_ACCOUNTS,
    RESPONSE_CHAT_MESSAGE,
    RESPONSE_DELETE_MESSAGE,
    RESPONSE_EDIT_MESSAGE,
    RESPONSE_FETCH_CHATS,
    RESPONSE_FETCH_CHAT_MESSAGES,
} e_transaction_type;

typedef void (*transaction_handler_func)(e_transaction_type, json_object *);

typedef struct s_transaction_handler
{
    e_transaction_type target_transaction_type;
    transaction_handler_func handle_func;
    bool remove_after_handling;
} t_transaction_handler;

t_transaction_handler *transaction_handler_new(e_transaction_type target_type, transaction_handler_func handler_func, bool remove_after_handling);
void transaction_handler_free(t_transaction_handler *transaction_handler);

typedef struct s_chat_socket
{
    struct sockaddr_in server_addr;
    int socket_fd;

    array_list *transaction_handlers;
    array_list *json_queue;
} t_chat_socket;

t_chat_socket *chat_socket_connect(int port, in_addr_t addr, int timeout);
void chat_socket_close(t_chat_socket *chat_socket);
void chat_socket_send_json(t_chat_socket *chat_socket, json_object *json);
void chat_socket_add_transaction_handler(t_chat_socket *chat_socket, t_transaction_handler *handle_func);
int chat_socket_wait_for_data(t_chat_socket *chat_socket);
void chat_socket_receive_transactions(t_chat_socket *chat_socket, int length);
void chat_socket_handle_tick(t_chat_socket *chat_socket);
json_object *chat_socket_wait_for_transaction(t_chat_socket *chat_socket, e_transaction_type transaction);
void chat_socket_call_handlers(t_chat_socket *chat_socket, e_transaction_type transaction, json_object *json);

e_transaction_type json_get_transaction_type(json_object *obj);
void json_add_base(json_object *obj, e_transaction_type transaction_type);
void json_add_page_limit(json_object *obj, int page, int limit);
void json_free(json_object *json_obj);
t_id json_object_get_id(json_object* json);
json_object* json_object_new_id(t_id id);

typedef struct s_account
{
    t_id id;
    const char *username;
} t_account;

t_account *account_new(t_id id, const char *username);
void account_free(t_account *account);
t_account *json_to_account(json_object *json);
t_account json_to_account_stack(json_object *json);
array_list *json_accounts_to_array_list(json_object *json);

#include "requests.h"
#include "responses.h"

#endif
