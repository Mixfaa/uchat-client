#ifndef RESPONSES_H
#define RESPONSES_H

#include "chat_socket.h"
#include "array_list_utils.h"

#include "requests.h"

typedef struct s_status_response
{
    const char *message;
    e_transaction_type response_for;
    bool is_failed;
} t_status_response;

t_status_response json_to_status_response_stack(json_object *json);

typedef struct s_login_responce
{
    t_account *user;
} t_login_response;

t_login_response *login_responce_new(t_account *user);
void login_response_free(t_login_response *login_responce);
t_login_response *json_to_login_response(json_object *json);

t_login_response json_to_login_response_stack(json_object *json);
void login_response_stack_free(t_login_response *login_response);
char *token_copy_from_login_response_json(json_object *json);

typedef struct s_chat_response
{
    const char *name;
    t_id chat_id;
    t_id owner_id;
    array_of_ids *participants_ids;
    t_id first_message_id;
} t_chat_response;

t_chat_response *chat_response_new(const char *name, t_id chat_id, t_id owner_id, array_of_ids *participants_ids, t_id first_message_id);
void chat_response_free(t_chat_response *chat_response);
t_chat_response *json_to_chat_response(json_object *json);

t_chat_response json_to_chat_response_stack(json_object *json);
void chat_response_stack_free(t_chat_response *chat_response);

typedef struct s_fetch_accounts_response
{
    const char *query;
    array_list *accounts; // array of t_account
} t_fetch_accounts_response;

t_fetch_accounts_response json_to_fetch_accounts_response_stack(json_object *json);
void fetch_accounts_response_stack_free(t_fetch_accounts_response *fetch_accounts_respnse);

typedef struct s_message_response
{
    t_id message_id;
    t_id owner_id;
    t_id chat_id;
    time_t timestamp;
    e_message_type message_type;
    const char *buffer;
    bool is_edited;
} t_message_response;

t_message_response *json_to_message_response(json_object *json);
t_message_response *message_response_new(t_id message_id, t_id owner_id, t_id chat_id, time_t timestamp, e_message_type type, const char *buffer, bool is_edited);
void message_response_free(t_message_response *);
t_message_response json_to_message_response_stack(json_object *json);

typedef struct s_fetch_chats_response
{
    array_list *chats; // t_chat_response
} t_fetch_chats_response;

t_fetch_chats_response json_to_fetch_chats_response_stack(json_object *json);
void fetch_chats_response_stack_free(t_fetch_chats_response *fetch_chats_response);

typedef struct s_fetch_chat_messages_response
{
    t_id chat_id;
    array_list *messages;
} t_fetch_chat_messages_response;

array_list *json_message_reponses_to_array_list(json_object *json);
t_fetch_chat_messages_response json_to_fetch_chat_messages_response_stack(json_object *json);
void fetch_chat_messages_response_free_stack(t_fetch_chat_messages_response *response);

typedef struct s_edit_message_response
{
    t_id message_id;
    t_id chat_id;
    const char *new_buffer;
} t_edit_message_response;

t_edit_message_response json_to_edit_message_response_stack(json_object *json);

typedef struct s_delete_message_response
{
    t_id message_id;
    t_id chat_id;
} t_delete_message_response;

t_delete_message_response json_to_delete_message_response_stack(json_object *json);

typedef struct s_delete_chat_response
{
    t_id chat_id;
} t_delete_chat_response;

t_delete_chat_response json_to_delete_chat_response_stack(json_object *json);

#endif
