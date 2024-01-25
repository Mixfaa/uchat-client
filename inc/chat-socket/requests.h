#ifndef MESSAGE_H
#define MESSAGE_H

#define TRANSACTION_TYPE_FIELD "transaction_type"

#include "chat_socket.h"
#include "array_list_utils.h"

e_transaction_type json_get_transaction_type(json_object *obj);
void json_free(json_object *json_obj);

void message_request_send(t_chat_socket *chat_socket, t_id chat_id, e_message_type message_type, char *message_buffer);
void login_request_send(t_chat_socket *chat_socket, const char *username, const char *password);
void create_chat_request_send(t_chat_socket *chat_socket, const char *chat_name, array_of_ids *participants_ids);
void fetch_accounts_request_send(t_chat_socket *chat_socket, const char *query, int page, int limit);
void message_delete_request_send(t_chat_socket *chat_socket, t_id message_id);
void message_edit_request_send(t_chat_socket *chat_socket, t_id message_id, const char *buffer);
void fetch_chats_request_send(t_chat_socket *chat_socket, int page, int limit);
void fetch_chat_messages_request_send(t_chat_socket *chat_socket, t_id chat_id, int page, int limit);
void delete_chat_request_send(t_chat_socket *chat_socket, t_id chat_id);
void fetch_accounts_by_ids_request_send(t_chat_socket *chat_socket, array_of_ids *ids);
void fetch_chats_by_ids_request_send(t_chat_socket* chat_socket, array_of_ids* ids);
#endif
