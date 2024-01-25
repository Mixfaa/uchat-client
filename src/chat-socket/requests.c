#include "chat-socket/requests.h"

json_object *array_of_ids_to_json(array_of_ids *ids)
{
    json_object *json = json_object_new_array();

    for (size_t i = 0; i < array_of_ids_length(ids); i++)
        json_object_array_add(json, json_object_new_id(array_of_ids_get(ids, i)));

    return json;
}

void message_request_send(t_chat_socket *chat_socket, t_id chat_id, e_message_type message_type, char *message_buffer)
{
    json_object *json = json_object_new_object();

    json_add_base(json, REQUEST_MESSAGE);

    json_object_object_add(json, "chat_id", json_object_new_int(chat_id));
    json_object_object_add(json, "message_type", json_object_new_int((int)message_type));
    json_object_object_add(json, "message_buffer", json_object_new_string(message_buffer));

    chat_socket_send_json(chat_socket, json);

    json_free(json);
}

void login_request_send(t_chat_socket *chat_socket, const char *username, const char *password)
{
    json_object *json = json_object_new_object();

    json_add_base(json, REQUEST_LOGIN);

    json_object_object_add(json, "username", json_object_new_string(username));
    json_object_object_add(json, "password", json_object_new_string(password));

    chat_socket_send_json(chat_socket, json);

    json_free(json);
}

void create_chat_request_send(t_chat_socket *chat_socket, const char *chat_name, array_of_ids *participants_ids)
{
    json_object *json = json_object_new_object();

    json_add_base(json, REQUEST_CREATE_CHAT);

    json_object_object_add(json, "name", json_object_new_string(chat_name));

    if (participants_ids != NULL && array_of_ids_length(participants_ids) != 0)
    {
        json_object *json_participants_ids = array_of_ids_to_json(participants_ids);
        json_object_object_add(json, "participants_ids", json_participants_ids);
    }

    chat_socket_send_json(chat_socket, json);

    json_free(json);
}

void fetch_accounts_request_send(t_chat_socket *chat_socket, const char *query, int page, int limit)
{
    json_object *json = json_object_new_object();

    json_add_base(json, REQUEST_FETCH_ACCOUNTS);

    if (query != NULL)
        json_object_object_add(json, "query", json_object_new_string(query));

    json_add_page_limit(json, page, limit);

    chat_socket_send_json(chat_socket, json);

    json_free(json);
}

void message_delete_request_send(t_chat_socket *chat_socket, t_id message_id)
{
    json_object *json = json_object_new_object();

    json_add_base(json, REQUEST_DELETE_MESSAGE);

    json_object_object_add(json, "message_id", json_object_new_id(message_id));

    chat_socket_send_json(chat_socket, json);

    json_free(json);
}

void message_edit_request_send(t_chat_socket *chat_socket, t_id message_id, const char *buffer)
{
    json_object *json = json_object_new_object();

    json_add_base(json, REQUEST_EDIT_MESSAGE);

    json_object_object_add(json, "message_id", json_object_new_id(message_id));
    json_object_object_add(json, "buffer", json_object_new_string(buffer));

    chat_socket_send_json(chat_socket, json);

    json_free(json);
}

void fetch_chats_request_send(t_chat_socket *chat_socket, int page, int limit)
{
    json_object *json = json_object_new_object();

    json_add_base(json, REQUEST_FETCH_CHATS);

    json_add_page_limit(json, page, limit);

    chat_socket_send_json(chat_socket, json);

    json_free(json);
}

void fetch_chat_messages_request_send(t_chat_socket *chat_socket, t_id chat_id, int page, int limit)
{
    json_object *json = json_object_new_object();

    json_add_base(json, REQUEST_FETCH_CHAT_MESSAGES);

    json_object_object_add(json, "chat_id", json_object_new_id(chat_id));

    json_add_page_limit(json, page, limit);

    chat_socket_send_json(chat_socket, json);

    json_free(json);
}

void delete_chat_request_send(t_chat_socket *chat_socket, t_id chat_id)
{
    json_object *json = json_object_new_object();

    json_add_base(json, REQUEST_DELETE_CHAT);

    json_object_object_add(json, "chat_id", json_object_new_id(chat_id));

    chat_socket_send_json(chat_socket, json);

    json_free(json);
}

void fetch_accounts_by_ids_request_send(t_chat_socket *chat_socket, array_of_ids *ids)
{
    json_object *json = json_object_new_object();

    json_add_base(json, REQUEST_FETCH_ACCOUNTS_BY_IDS);

    json_object_object_add(json, "accounts_ids", array_of_ids_to_json(ids));

    chat_socket_send_json(chat_socket, json);

    json_free(json);
}

void fetch_chats_by_ids_request_send(t_chat_socket *chat_socket, array_of_ids *ids)
{
    json_object *json = json_object_new_object();

    json_add_base(json, REQUEST_FETCH_CHATS_BY_IDS);

    json_object_object_add(json, "chat_ids", array_of_ids_to_json(ids));

    chat_socket_send_json(chat_socket, json);

    json_free(json);
}

// void fetch_accounts_data_request_send(t_chat_socket* chat_socket, array_of_ids* ids)
// {
//     json_object* json = json_object_new_object();

//     json_add_base(json, REQUEST_FETCH_ACCOUNTS_DATA);

//     json_object_object_add(json, "accounts_ids", array_of_ids_to_json(ids));

//     json_free(json);
// }