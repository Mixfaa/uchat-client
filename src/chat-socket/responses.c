#include <string.h>

#include "chat-socket/chat_socket.h"

array_list *json_message_reponses_to_array_list(json_object *json)
{
    array_list *messages_jsons = json_object_get_array(json);
    array_list *messages = array_list_new2((array_list_free_fn *)account_free, messages_jsons->length);

    for (size_t i = 0; i < array_list_length(messages_jsons); i++)
        array_list_add(messages, json_to_message_response(array_list_get_idx(messages_jsons, i)));

    return messages;
}

#pragma region login_response

t_status_response json_to_status_response_stack(json_object *json)
{
    t_status_response response;

    response.message = json_object_get_string(json_object_object_get(json, "message"));
    response.is_failed = json_object_get_boolean(json_object_object_get(json, "is_failed"));
    response.response_for = (e_transaction_type)json_object_get_int(json_object_object_get(json, "response_for"));

    return response;
}

t_login_response *login_responce_new(t_account *user)
{
    t_login_response *login_responce_ptr = (t_login_response *)malloc(sizeof(t_login_response));

    login_responce_ptr->user = user;

    return login_responce_ptr;
}

void login_response_free(t_login_response *login_responce)
{
    account_free(login_responce->user);
    free(login_responce);
}

t_login_response *json_to_login_response(json_object *json)
{
    t_login_response response = json_to_login_response_stack(json);
    return login_responce_new(response.user);
}

t_login_response json_to_login_response_stack(json_object *json)
{
    t_login_response response;
    response.user = json_to_account(json_object_object_get(json, "user"));

    return response;
}

void login_response_stack_free(t_login_response *login_response)
{
    account_free(login_response->user);
}

char *token_copy_from_login_response_json(json_object *json)
{
    const char *token = json_object_get_string(json_object_object_get(json, "token"));
    return strdup(token);
}

#pragma endregion

#pragma region chat_response

t_chat_response *chat_response_new(const char *name, t_id chat_id, t_id owner_id, array_of_ids *participants_ids, t_id first_message_id)
{
    t_chat_response *chat_response = (t_chat_response *)malloc(sizeof(t_chat_response));

    chat_response->chat_id = chat_id;
    chat_response->owner_id = owner_id;
    chat_response->name = strdup(name);
    chat_response->participants_ids = participants_ids;
    chat_response->first_message_id = first_message_id;

    return chat_response;
}

void chat_response_free(t_chat_response *chat_response) // dont call for stack
{
    free((void *)chat_response->name);
    array_of_ids_free(chat_response->participants_ids);
    free(chat_response);
}

t_chat_response *json_to_chat_response(json_object *json)
{
    t_chat_response response = json_to_chat_response_stack(json);
    return chat_response_new(response.name, response.chat_id, response.owner_id, response.participants_ids, response.first_message_id);
}

t_chat_response json_to_chat_response_stack(json_object *json)
{
    t_chat_response response;

    response.chat_id = json_object_get_id(json_object_object_get(json, "chat_id"));
    response.owner_id = json_object_get_id(json_object_object_get(json, "owner_id"));
    response.name = json_object_get_string(json_object_object_get(json, "name"));
    response.first_message_id = json_object_get_id(json_object_object_get(json, "first_message_id"));

    response.participants_ids = json_to_array_of_ids(json_object_object_get(json, "participants_ids"));

    return response;
}

void chat_response_stack_free(t_chat_response *chat_response)
{
    array_of_ids_free(chat_response->participants_ids);
}

#pragma endregion

t_fetch_accounts_response json_to_fetch_accounts_response_stack(json_object *json)
{
    t_fetch_accounts_response response;

    response.query = json_object_get_string(json_object_object_get(json, "query"));
    response.accounts = json_accounts_to_array_list(json_object_object_get(json, "accounts"));

    return response;
}

void fetch_accounts_response_stack_free(t_fetch_accounts_response *fetch_accounts_respnse)
{
    array_list_free(fetch_accounts_respnse->accounts);
}

t_message_response *json_to_message_response(json_object *json)
{
    t_message_response response = json_to_message_response_stack(json);
    return message_response_new(response.message_id,
                                response.owner_id,
                                response.chat_id,
                                response.timestamp,
                                response.message_type,
                                response.buffer,
                                response.is_edited);
}
t_message_response *message_response_new(t_id message_id, t_id owner_id, t_id chat_id, time_t timestamp, e_message_type type, const char *buffer, bool is_edited)
{
    t_message_response *message_response = (t_message_response *)malloc(sizeof(t_message_response));

    message_response->message_id = message_id;
    message_response->owner_id = owner_id;
    message_response->chat_id = chat_id;
    message_response->timestamp = timestamp;
    message_response->message_type = type;
    message_response->buffer = (const char *)strdup(buffer);
    message_response->is_edited = is_edited;

    return message_response;
}

void message_response_free(t_message_response *message_response)
{
    free((void *)message_response->buffer);
    free(message_response);
}

t_message_response json_to_message_response_stack(json_object *json)
{
    t_message_response response;

    response.message_id = json_object_get_id(json_object_object_get(json, "message_id"));
    response.owner_id = json_object_get_id(json_object_object_get(json, "owner_id"));
    response.chat_id = json_object_get_id(json_object_object_get(json, "chat_id"));

    response.timestamp = json_object_get_id(json_object_object_get(json, "timestamp"));
    response.message_type = (e_message_type)json_object_get_int(json_object_object_get(json, "message_type"));
    response.buffer = json_object_get_string(json_object_object_get(json, "buffer"));
    response.is_edited = json_object_get_boolean(json_object_object_get(json, "is_edited"));

    return response;
}

t_fetch_chats_response json_to_fetch_chats_response_stack(json_object *json)
{
    t_fetch_chats_response response;

    array_list *chats_json = json_object_get_array(json_object_object_get(json, "chats"));
    array_list *chat_responses = array_list_new2((array_list_free_fn *)chat_response_free, chats_json->length);

    for (size_t i = 0; i < array_list_length(chats_json); i++)
    {
        json_object *chat_json = array_list_get_idx(chats_json, i);

        t_chat_response *chat_response = json_to_chat_response(chat_json);
        array_list_add(chat_responses, chat_response);
    }

    response.chats = chat_responses;

    return response;
}

void fetch_chats_response_stack_free(t_fetch_chats_response *fetch_chats_response)
{
    array_list_free(fetch_chats_response->chats);
}

t_fetch_chat_messages_response json_to_fetch_chat_messages_response_stack(json_object *json)
{
    t_fetch_chat_messages_response response;

    response.chat_id = json_object_get_id(json_object_object_get(json, "chat_id"));
    response.messages = json_message_reponses_to_array_list(json_object_object_get(json, "messages"));

    return response;
}

void fetch_chat_messages_response_free_stack(t_fetch_chat_messages_response *response)
{
    array_list_free(response->messages);
}

t_edit_message_response json_to_edit_message_response_stack(json_object *json)
{
    t_edit_message_response response;
    response.chat_id = json_object_get_id(json_object_object_get(json, "chat_id"));
    response.message_id = json_object_get_id(json_object_object_get(json, "message_id"));
    response.new_buffer = json_object_get_string(json_object_object_get(json, "new_buffer"));

    return response;
}

t_delete_message_response json_to_delete_message_response_stack(json_object *json)
{
    t_delete_message_response response;

    response.chat_id = json_object_get_id(json_object_object_get(json, "chat_id"));
    response.message_id = json_object_get_id(json_object_object_get(json, "message_id"));

    return response;
}

t_delete_chat_response json_to_delete_chat_response_stack(json_object *json)
{
    t_delete_chat_response response;
    response.chat_id = json_object_get_id(json_object_object_get(json, "chat_id"));

    return response;
}
