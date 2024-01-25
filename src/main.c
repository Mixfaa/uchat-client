#include "uchat.h"
static char *message = NULL;
void draw_status_response_popup()
{
	if (message != NULL && igBeginPopupModal("Status message", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		igText(message);
		if (igButton("OK", imVec2_zero()))
			igCloseCurrentPopup();
		igEndPopup();
	}
}

void draw_gui()
{
	t_uchat *uchat = get_uchat_instance();

	int width = 0;
	int height = 0;
	glfwGetWindowSize(uchat->glfw_window, &width, &height);
	igSetNextWindowPos(imVec2_zero(), ImGuiCond_Always, imVec2_zero());
	igSetNextWindowSize(imVec2(width, height), ImGuiCond_Always);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
	if (uchat->uchat_state != UCHAT_STATE_NEED_TO_LOGIN)
		window_flags |= ImGuiWindowFlags_MenuBar;

	igBegin("uchat main window", NULL, window_flags);

	switch (uchat->uchat_state)
	{
	case UCHAT_STATE_NEED_TO_LOGIN:
	{
		igText("Login first");

		static bool buffers_cleared = false;
		static char username_buff[128];
		static char password_buff[128];

		if (!buffers_cleared)
		{
			memset(username_buff, 0, 128);
			memset(password_buff, 0, 128);

			buffers_cleared = true;
		}

		igInputText("Username", username_buff, 128, 0, NULL, NULL);
		igInputText("Password", password_buff, 128, 0, NULL, NULL);

		if (strlen(username_buff) != 0 && strlen(password_buff) != 0 && igButton("Login!", imVec2_zero()))
		{
			login_request_send(uchat->chat_socket, username_buff, password_buff);

			json_object *json = chat_socket_wait_for_transaction(uchat->chat_socket, 0);

			switch (json_get_transaction_type(json))
			{
			case TRANSACTION_STATUS_RESPONSE:
			{
				t_status_response status_response = json_to_status_response_stack(json);

				printf("%d %s\n", status_response.response_for, status_response.message);
				message = strdup(status_response.message);
				igOpenPopup_Str("Status message", ImGuiPopupFlags_AnyPopup);
				break;
			}
			case RESPONSE_LOGIN:
			{
				t_login_response login_response = json_to_login_response_stack(json);
				uchat->uchat_state = UCHAT_STATE_LOGINED;

				t_account *my_account = account_new(login_response.user->id, login_response.user->username);

				uchat->my_id = my_account->id;
				uchat_add_account(my_account);

				login_response_stack_free(&login_response);

				fetch_chats_request_send(uchat->chat_socket, 0, 15);
				break;
			}
			default:
				break;
			}

			json_free(json);
		}
		break;
	}
	case UCHAT_STATE_LOGINED:
	{
		float window_heigth = ig_get_window_height();
		ImGuiStyle *style = igGetStyle();

		if (igBeginMenuBar())
		{
			if (igBeginMenu("Menu", true))
			{
				if (igBeginMenu("Create chat", true))
				{
					static bool initialized = false;
					static char chat_name_buffer[128];
					static char participant_name_buffer[128];
					static array_of_ids *participants_ids = NULL;
					static t_fetch_accounts_response fetched_accounts;

					if (!initialized)
					{
						memset(chat_name_buffer, 0, 128);
						memset(participant_name_buffer, 0, 128);
						memset(&fetched_accounts, 0, sizeof(t_fetch_accounts_response));
						participants_ids = array_of_ids_new2(32);
						initialized = true;
					}

					igInputText("Chat name", chat_name_buffer, 128, 0, NULL, NULL);

					igInputText("Search for users to add", participant_name_buffer, 128, 0, NULL, NULL);
					if (igButton("Search", imVec2_zero()))
					{
						fetch_accounts_request_send(uchat->chat_socket, participant_name_buffer, 0, 20);
						json_object *json = chat_socket_wait_for_transaction(uchat->chat_socket, RESPONSE_FETCH_ACCOUNTS);

						e_transaction_type type = json_get_transaction_type(json);

						if (type == RESPONSE_FETCH_ACCOUNTS)
						{
							fetched_accounts = json_to_fetch_accounts_response_stack(json);
						}

						json_free(json);
					}

					if (fetched_accounts.accounts != NULL)
						for (size_t i = 0; i < array_list_length(fetched_accounts.accounts); i++)
						{
							t_account *account = array_list_get_idx(fetched_accounts.accounts, i);

							if (uchat->my_id == account->id)
								continue;

							igText("%s (%d)", account->username, account->id);

							bool account_is_in_list = array_of_ids_contains(participants_ids, account->id);

							char btn_buffer[32];
							sprintf(btn_buffer, account_is_in_list ? "-###acc_id%ld" : "+###acc_id%ld", account->id);
							igSameLine(0.f, 0.f);
							if (igButton(btn_buffer, imVec2_zero()))
							{
								if (account_is_in_list)
									array_of_ids_remove(participants_ids, account->id);
								else
									array_of_ids_add(participants_ids, account->id);
							}
						}

					if (igButton("Create chat", imVec2_zero()))
					{
						create_chat_request_send(uchat->chat_socket, chat_name_buffer, participants_ids);
						fetch_accounts_response_stack_free(&fetched_accounts);
						array_of_ids_free(participants_ids);
						initialized = false;
					}
					igEndMenu();
				}
				igEndMenu();
			}
			igEndMenuBar();
		}

		igBeginChild_Str("Chats", imVec2(200.f, window_heigth), ImGuiChildFlags_Border, 0);
		size_t chats_count = array_list_length(uchat->chats);

		if (chats_count != 0)
		{
			for (size_t i = 0; i < chats_count; i++)
			{
				t_chat_response *chat = array_list_get_idx(uchat->chats, i);

				char chat_name_buff[512];
				sprintf(chat_name_buff, "%s###btn_chat%ld", chat->name, chat->chat_id);

				bool selected = uchat->current_chat == chat;
				if (igSelectable_Bool(chat_name_buff, selected, 0, imVec2_zero()))
					uchat->current_chat = chat;
			}
		}
		igEndChild();

		if (uchat->current_chat != NULL)
		{
			igSameLine(0.f, 7.f);

			igBeginGroup();
			float send_msg_child_height = 65.f;
			igBeginChild_Str("Chat", imVec2(0.f, window_heigth - send_msg_child_height), ImGuiChildFlags_Border, 0);

			static bool initlized = false;
			static char message_buffer[512];
			static char text_buffer[1024];

			if (!initlized)
			{
				memset(message_buffer, 0, 512);
				memset(text_buffer, 0, 1024);
				initlized = true;
			}

			igText("Chat: %s (%ld)", uchat->current_chat->name, uchat->current_chat->chat_id);

			if (uchat->current_chat->owner_id == uchat->my_id)
			{
				igSameLine(0.f, 0.f);
				if (igButton("delete chat", imVec2_zero()))
					delete_chat_request_send(uchat->chat_socket, uchat->current_chat->chat_id);
			}
			igSeparator();
			static bool editing_message = false;
			static t_id editing_message_id = 0;
			static char new_msg_buffer[512];
			for (size_t i = array_list_length(uchat->messages); i >= 0; --i)
			{
				t_message_response *message = array_list_get_idx(uchat->messages, i);
				if (message->chat_id != uchat->current_chat->chat_id)
					continue;

				t_account *message_owner = NULL;
				message_owner = uchat_get_account_or_nul(message->owner_id);

				if (message_owner == NULL)
					sprintf(text_buffer, "(unloaded user %ld):\n    %s", message->owner_id, message->buffer);
				else
					sprintf(text_buffer, "%s:\n    %s", message_owner->username, message->buffer);

				igText(text_buffer);

				bool text_item_clicked = igIsItemClicked(ImGuiMouseButton_Right);

				if (message->is_edited)
				{
					igSameLine(0.f, 0.f);
					igTextDisabled("(edited)");
				}

				if (uchat->my_id == message->owner_id && (text_item_clicked || igIsItemClicked(ImGuiMouseButton_Right)))
				{
					editing_message = true;
					editing_message_id = message->message_id;

					igOpenPopup_Str("Edit message###popup", ImGuiPopupFlags_NoOpenOverExistingPopup);
					memset(new_msg_buffer, 0, 512);
					strncpy(new_msg_buffer, message->buffer, 512);
				}
			}
			t_message_response *msg_to_edit = uchat_get_message_or_null(editing_message_id);
			if (msg_to_edit == NULL)
				editing_message = false;

			if (editing_message && igBeginPopup("Edit message###popup", ImGuiWindowFlags_AlwaysAutoResize))
			{
				if (igButton("delete message", imVec2_zero()))
					message_delete_request_send(uchat->chat_socket, msg_to_edit->message_id);

				igSeparator();

				igInputText("edit message", new_msg_buffer, 512, 0, NULL, NULL);
				if (igButton("edit", imVec2_zero()))
					message_edit_request_send(uchat->chat_socket, msg_to_edit->message_id, new_msg_buffer);
				igEndPopup();
			}

			igEndChild();

			igBeginChild_Str("Send message", imVec2(0.f, send_msg_child_height), ImGuiChildFlags_Border, 0);

			igInputText("Enter message", message_buffer, 512, 0, NULL, NULL);
			if (igButton("Send", imVec2_zero()))
				message_request_send(uchat->chat_socket, uchat->current_chat->chat_id, MESSAGE_TYPE_TEXT, message_buffer);

			igEndChild();
			igEndGroup();
		}

		break;
	}
	default:
		break;
	}

	ig_popups_draw();
	igEnd();
}

void handle_transaction(e_transaction_type type, json_object *json) // PLEASE ENSURE YOUR WROTE FUCKING BREAK;!!!!!!!!!!!!!!
{
	t_uchat *uchat = get_uchat_instance();
	switch (type)
	{
	case RESPONSE_FETCH_CHATS:
	{
		printf("RESPONSE_FETCH_CHATS\n");
		uchat_handle_fetch_chats(json);
		break;
	}
	case RESPONSE_CHAT:
	{
		printf("RESPONSE_CHAT\n");
		uchat_handle_chat_response(json);
		break;
	}
	case RESPONSE_FETCH_CHAT_MESSAGES:
	{
		printf("RESPONSE_FETCH_CHAT_MESSAGES\n");
		uchat_handle_fetch_chat_messages(json);
		break;
	}
	case RESPONSE_CHAT_MESSAGE:
	{
		printf("RESPONSE_CHAT_MESSAGE\n");
		uchat_handle_chat_message(json);
		break;
	}
	case RESPONSE_FETCH_ACCOUNTS:
	{
		printf("RESPONSE_FETCH_ACCOUNTS\n");
		uchat_handle_fetch_accounts(json);
		break;
	}
	case RESPONSE_EDIT_MESSAGE:
	{
		printf("RESPONSE_EDIT_MESSAGE\n"); 
		uchat_handle_edit_message(json);
		break;
	}
	case RESPONSE_DELETE_MESSAGE:
	{
		printf("RESPONSE_DELETE_MESSAGE\n");
		uchat_handle_delete_message(json);
		break;
	}
	case RESPONSE_DELETE_CHAT:
	{
		printf("RESPONSE_DELETE_CHAT\n");
		uchat_handle_delete_chat(json);
		break;
	}
	default:
		break;
	}
}

void mainloop(void)
{
	t_uchat *uchat = get_uchat_instance();
	chat_socket_handle_tick(uchat->chat_socket);
	draw_gui();
}

int main(int argc, char *argv[])
{
	t_uchat *uchat = get_uchat_instance();
	uchat->chat_socket = chat_socket_connect(8080, inet_addr("192.168.0.213"), 500);

	if (uchat->chat_socket == NULL)
		return 1;

	uchat->glfw_window = create_gui(700, 600, "uchat by depression");

	if (uchat->glfw_window == NULL)
	{
		chat_socket_close(uchat->chat_socket);
		return -1;
	}

	ig_popup_add(draw_status_response_popup);
	chat_socket_add_transaction_handler(uchat->chat_socket, transaction_handler_new(0, handle_transaction, false));
	mainloop_gui(uchat->glfw_window, mainloop);

	uchat_destroy();
	chat_socket_close(uchat->chat_socket);
	destroy_gui(uchat->glfw_window);
	return 0;
}
