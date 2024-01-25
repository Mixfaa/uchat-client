#ifndef UCHAT_H
#define UCHAT_H

#define UCHAT_GET_INST_TO(varname) t_uchat* varname = get_uchat_instance()

#include "gui_utils.h"
#include "chat-socket/chat_socket.h"

typedef enum e_uchat_state
{
	UCHAT_STATE_NEED_TO_LOGIN,
	UCHAT_STATE_LOGINED,
} e_uchat_state;

typedef struct s_uchat
{
	GLFWwindow *glfw_window;
	t_chat_socket *chat_socket;
	e_uchat_state uchat_state;

	array_list *chats;
	array_list *messages;
	array_list *accounts;

	t_chat_response *current_chat;
	t_id my_id;
} t_uchat;

t_uchat *get_uchat_instance(void);
void uchat_destroy(void);

void uchat_add_account(t_account *account);
bool uchat_is_account_loaded(t_id account_id);
void uchat_fetch_accounts_from_chat(t_chat_response *chat);
void uchat_add_fetched_accounts(t_fetch_accounts_response *response);
t_account *uchat_get_account_or_nul(t_id account_id);
t_message_response *uchat_get_message_or_null(t_id message_id);
t_chat_response *uchat_get_chat_or_null(t_id chat_id);

void uchat_handle_fetch_chats(json_object* json);
void uchat_handle_chat_response(json_object* json);
void uchat_handle_fetch_chat_messages(json_object* json);
void uchat_handle_chat_message(json_object* json);
void uchat_handle_fetch_accounts(json_object* json);
void uchat_handle_edit_message(json_object* json);
void uchat_handle_delete_message(json_object* json);
void uchat_handle_delete_chat(json_object* json);

#endif
