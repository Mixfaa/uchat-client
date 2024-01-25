#include "uchat.h"

t_uchat *get_uchat_instance(void)
{
	static bool init = false;
	static t_uchat uchat;

	if (!init)
	{
		uchat.current_chat = NULL;
		uchat.chat_socket = NULL;
		uchat.glfw_window = NULL;
		uchat.accounts = array_list_new2((array_list_free_fn *)account_free, 128);
		uchat.chats = array_list_new2((array_list_free_fn *)chat_response_free, 32);
		uchat.messages = array_list_new2((array_list_free_fn *)message_response_free, 128);
		uchat.uchat_state = UCHAT_STATE_NEED_TO_LOGIN;
		uchat.my_id = 0;

		init = true;
	}

	return &uchat;
}

void uchat_destroy()
{
	t_uchat *uchat = get_uchat_instance();

	array_list_free(uchat->accounts);
	array_list_free(uchat->chats);
	array_list_free(uchat->messages);
	uchat->uchat_state = UCHAT_STATE_NEED_TO_LOGIN;
}

t_account *uchat_get_account_or_nul(t_id account_id)
{
	t_uchat *uchat = get_uchat_instance();

	for (size_t i = 0; i < array_list_length(uchat->accounts); i++)
	{
		t_account *account = array_list_get_idx(uchat->accounts, i);
		if (account->id == account_id)
			return account;
	}
	return NULL;
}

bool uchat_is_account_loaded(t_id account_id)
{
	return uchat_get_account_or_nul(account_id) != NULL;
}

void uchat_fetch_accounts_from_chat(t_chat_response *chat)
{
	t_uchat *uchat = get_uchat_instance();

	array_of_ids *ids_to_fetch = array_of_ids_new2(8);

	for (size_t i = 0; i < array_of_ids_length(chat->participants_ids); i++)
	{
		t_id id = array_of_ids_get(chat->participants_ids, i);
		if (uchat_is_account_loaded(id))
			continue;

		array_of_ids_add(ids_to_fetch, id);
	}

	fetch_accounts_by_ids_request_send(uchat->chat_socket, ids_to_fetch);
}

void uchat_add_account(t_account *account)
{
	t_uchat *uchat = get_uchat_instance();

	if (uchat_is_account_loaded(account->id))
		return; // implement replacing with new account

	array_list_add(uchat->accounts, account);
}

void uchat_add_fetched_accounts(t_fetch_accounts_response *response)
{
	t_uchat *uchat = get_uchat_instance();

	for (size_t i = 0; i < array_list_length(response->accounts); i++)
	{
		t_account *account = array_list_get_idx(response->accounts, i);

		if (uchat_is_account_loaded(account->id))
			continue; // do smth insted

		array_list_add(uchat->accounts, account);
		array_list_del_idx_no_free(response->accounts, i, 1);
		--i;
	}
}

t_message_response *uchat_get_message_or_null(t_id message_id)
{
	t_uchat *uchat = get_uchat_instance();

	for (size_t i = 0; i < array_list_length(uchat->messages); i++)
	{
		t_message_response *msg = array_list_get_idx(uchat->messages, i);
		if (msg->message_id == message_id)
			return msg;
	}
	return NULL;
}

t_chat_response *uchat_get_chat_or_null(t_id chat_id)
{
	UCHAT_GET_INST_TO(uchat);

	for (size_t i = 0; i < array_list_length(uchat->chats); i++)
	{
		t_chat_response *chat = array_list_get_idx(uchat->chats, i);
		if (chat->chat_id == chat_id)
			return chat;
	}
	return NULL;
}

void uchat_handle_fetch_chats(json_object *json)
{
	UCHAT_GET_INST_TO(uchat);
	t_fetch_chats_response fetch_chats = json_to_fetch_chats_response_stack(json);

	array_list_disable_free(fetch_chats.chats);

	array_list_clear(uchat->chats);
	array_list_add_all(uchat->chats, fetch_chats.chats);

	for (size_t i = 0; i < array_list_length(uchat->chats); i++)
	{
		t_chat_response *chat = array_list_get_idx(uchat->chats, i);
		fetch_chat_messages_request_send(uchat->chat_socket, chat->chat_id, 0, 20);
		uchat_fetch_accounts_from_chat(chat);
	}

	fetch_chats_response_stack_free(&fetch_chats);
}
void uchat_handle_chat_response(json_object *json)
{
	UCHAT_GET_INST_TO(uchat);

	t_chat_response *chat_resp = json_to_chat_response(json);

	array_list_add(uchat->chats, chat_resp);
	uchat_fetch_accounts_from_chat(chat_resp);
}

void uchat_handle_fetch_chat_messages(json_object *json)
{
	UCHAT_GET_INST_TO(uchat);

	t_fetch_chat_messages_response fetch_resp = json_to_fetch_chat_messages_response_stack(json);

	array_list_disable_free(fetch_resp.messages);

	array_list_add_all(uchat->messages, fetch_resp.messages);

	fetch_chat_messages_response_free_stack(&fetch_resp);
}

void uchat_handle_chat_message(json_object *json)
{
	UCHAT_GET_INST_TO(uchat);
	t_message_response *msg_resp = json_to_message_response(json);
	array_list_add(uchat->messages, msg_resp);
}

void uchat_handle_fetch_accounts(json_object *json)
{
	UCHAT_GET_INST_TO(uchat);
	t_fetch_accounts_response fetch_resp = json_to_fetch_accounts_response_stack(json);

	uchat_add_fetched_accounts(&fetch_resp);

	fetch_accounts_response_stack_free(&fetch_resp);
}

void uchat_handle_edit_message(json_object *json)
{
	UCHAT_GET_INST_TO(uchat);
	t_edit_message_response edit_msg = json_to_edit_message_response_stack(json);

	for (size_t i = 0; i < array_list_length(uchat->messages); i++)
	{
		t_message_response *msg = array_list_get_idx(uchat->messages, i);
		if (msg->message_id == edit_msg.message_id)
		{
			free((char *)msg->buffer);
			msg->buffer = strdup(edit_msg.new_buffer);
			msg->is_edited = true;
			return;
		}
	}
}

void uchat_handle_delete_message(json_object *json)
{
	UCHAT_GET_INST_TO(uchat);
	t_delete_message_response delete_msg = json_to_delete_message_response_stack(json);

	for (size_t i = 0; i < array_list_length(uchat->messages); i++)
	{
		t_message_response *msg = array_list_get_idx(uchat->messages, i);
		if (msg->message_id == delete_msg.message_id)
		{
			array_list_del_idx(uchat->messages, i, 1);
			return;
		}
	}
}

void uchat_handle_delete_chat(json_object *json)
{
	UCHAT_GET_INST_TO(uchat);
	t_delete_chat_response del_chat = json_to_delete_chat_response_stack(json);

	t_id chat_id = del_chat.chat_id;

	if (uchat->current_chat != NULL && uchat->current_chat->chat_id == chat_id)
		uchat->current_chat = NULL;

	for (size_t i = 0; i < array_list_length(uchat->chats); i++)
	{
		t_chat_response *chat = array_list_get_idx(uchat->chats, i);

		if (chat->chat_id != chat_id)
			continue;

		array_list_del_idx(uchat->chats, i, 1);
		--i; // remake deleting
	}

	for (size_t i = 0; i < array_list_length(uchat->messages); i++)
	{
		t_message_response *message = array_list_get_idx(uchat->messages, i);
		if (message->chat_id != chat_id)
			continue;

		array_list_del_idx(uchat->messages, i, 1);
		--i;
	}
}
