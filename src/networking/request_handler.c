#include "../utils/headers/request_handler.h"

int process_request_login(Server *server, int client_index, BSMessage *request) {
    BSMessage response;
    char response_buffer[BUFFER_SIZE];
    Player *existing_player = get_player_by_socket(server->player_table, server->clients[client_index].fd);
    if (existing_player) {
        create_message(&response, MSG_ERROR, "You are already logged in.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    }
    Player *player = get_player(server->player_table, request->data);
    if (player) {
        create_message(&response, MSG_ERROR, "Username already taken.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    }
    player = add_player(server->player_table, request->data, server->clients[client_index].fd);
    if (player) {
        create_message(&response, MSG_OK, "Login successful.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        printf("User (%s) connected on socket descriptor: %d\n", player->username, player->socket_fd);
        log_info("User (%s) connected on socket descriptor: %d\n", player->username, player->socket_fd);
        return 0;
    } else {
        create_message(&response, MSG_ERROR, "Failed to add player.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    }
}

int process_request_logout(Server *server, int client_index) {
    BSMessage response;
    char response_buffer[BUFFER_SIZE];
    Player *existing_player = get_player_by_socket(server->player_table, server->clients[client_index].fd);
    if (!existing_player) {
        create_message(&response, MSG_ERROR, "You are not logged in.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    } else {
        // Notify the other player if this player was in a game session
        game_session_t *session = find_game_session(server->game_session_table, existing_player);
        if (session) {
            BSMessage game_over;
            char game_over_buffer[BUFFER_SIZE];
            snprintf(game_over_buffer, BUFFER_SIZE, "%s", strcmp(session->player1->username, existing_player->username) == 0 ? session->player2->username : session->player1->username);
            create_message(&game_over, MSG_GAME_OVER, game_over_buffer);
            serialize_message(&game_over, game_over_buffer);
            send(session->player1->socket_fd, game_over_buffer, strlen(game_over_buffer), 0);
            send(session->player2->socket_fd, game_over_buffer, strlen(game_over_buffer), 0);
            log_info("%s\n", game_over_buffer);
            remove_game_session(server->game_session_table, existing_player);
        }
        if (remove_player(server->player_table, server->clients[client_index].fd)) {
            create_message(&response, MSG_OK, "Logout successful. Bye!");
            serialize_message(&response, response_buffer);
            send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
            log_info("%s\n", response_buffer);
        } else {
            create_message(&response, MSG_ERROR, "Failed to disconnect player.");
            serialize_message(&response, response_buffer);
            send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
            log_info("%s\n", response_buffer);
            return -1;
        }
    }
    return 0;
}

int process_request_user_list(Server *server, int client_index) {
    BSMessage response;
    char user_list[BUFFER_SIZE];
    char response_buffer[BUFFER_SIZE];
    get_user_list(server->player_table, user_list);
    create_message(&response, MSG_OK, user_list);
    serialize_message(&response, response_buffer);
    send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
    log_info("%s\n", response_buffer);
    return 0;
}

int process_request_invite_send(Server *server, int client_index, BSMessage *request) {
    BSMessage response;
    char response_buffer[BUFFER_SIZE];
    Player *sender_player = get_player_by_socket(server->player_table, server->clients[client_index].fd);
    Player *target_player = get_player(server->player_table, request->data);
    if (!sender_player || !target_player) {
        create_message(&response, MSG_ERROR, "User not found.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    }
    if (strcmp(sender_player->username, request->data) == 0) {
        create_message(&response, MSG_ERROR, "You cannot invite yourself.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    }
    if (find_invitation(server->invitation_table, sender_player->username, target_player->username) == 1) {
        create_message(&response, MSG_ERROR, "You have already sent an invitation to this user.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    }
    if (target_player->in_game) {
        create_message(&response, MSG_ERROR, "Target player is already in a game.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    }
    if (sender_player->in_game) {
        create_message(&response, MSG_ERROR, "You are already in a game.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    }
    add_invitation(server->invitation_table, sender_player->username, target_player->username);
    create_message(&response, MSG_OK, "Invitation sent.");
    serialize_message(&response, response_buffer);
    send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
    log_info("%s\n", response_buffer);
    
    BSMessage invite_notification;
    char invite_buffer[BUFFER_SIZE];
    create_message(&invite_notification, MSG_INVITE_RECEIVE, sender_player->username);
    serialize_message(&invite_notification, invite_buffer);
    send(target_player->socket_fd, invite_buffer, strlen(invite_buffer), 0);
    log_info("%s\n", invite_buffer);
    printf("(%s) invited (%s) to a game.\n", sender_player->username, target_player->username);
    log_info("(%s) invited (%s) to a game.\n", sender_player->username, target_player->username);
    return 0;
}

int process_request_invite_reply(Server *server, int client_index, BSMessage *request) {
    BSMessage response;
    char response_buffer[BUFFER_SIZE], original_sender[50], invite_status[10];
    sscanf(request->data, "%s %s", original_sender, invite_status);
    Player *original_sender_player = get_player(server->player_table, original_sender);
    Player *reply_player = get_player_by_socket(server->player_table, server->clients[client_index].fd);
    if (!original_sender_player || !reply_player) {
        create_message(&response, MSG_ERROR, "User not found.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    }
    if (strcmp(reply_player->username, original_sender) == 0) {
        create_message(&response, MSG_ERROR, "You cannot reply to your own invitation.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    }
    if (find_invitation(server->invitation_table, original_sender_player->username, reply_player->username) == 0) {
        create_message(&response, MSG_ERROR, "You have no invitation from this user.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    }
    if((strcmp(invite_status, "ACCEPT") == 0 || strcmp(invite_status, "REJECT") == 0 ) && original_sender_player->in_game == 0 && reply_player->in_game == 0) {
        create_message(&response, MSG_OK, "Response sent.");
        serialize_message(&response, response_buffer);
        send(reply_player->socket_fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        
        BSMessage invite_result;
        char invite_result_buffer[BUFFER_SIZE];
        snprintf(invite_result_buffer, BUFFER_SIZE, "%s %s", original_sender, invite_status);
        create_message(&invite_result, MSG_INVITE_RESULT, invite_result_buffer);
        serialize_message(&invite_result, invite_result_buffer);
        send(original_sender_player->socket_fd, invite_result_buffer, strlen(invite_result_buffer), 0);
        log_info("%s\n", invite_result_buffer);
        if (strcmp(invite_status, "ACCEPT") == 0) {
            // Create a new game session
            game_session_t *session = add_game_session(server->game_session_table, original_sender_player, reply_player);
            printf("(%s) accepted the invitation from (%s).\n", reply_player->username, original_sender);
            log_info("(%s) accepted the invitation from (%s).\n", reply_player->username, original_sender);
            BSMessage game_start;
            char game_start_buffer[BUFFER_SIZE];
            create_message(&game_start, MSG_GAME_START, NULL);
            serialize_message(&game_start, game_start_buffer);
            send(original_sender_player->socket_fd, game_start_buffer, strlen(game_start_buffer), 0);
            send(reply_player->socket_fd, game_start_buffer, strlen(game_start_buffer), 0);
            log_info("%s\n", game_start_buffer);
            // Notify the player that has the turn
            BSMessage turn_notification;
            char turn_buffer[BUFFER_SIZE];
            snprintf(turn_buffer, BUFFER_SIZE, "%s", session->current_turn == 0 ? original_sender_player->username : reply_player->username);
            create_message(&turn_notification, MSG_TURN, turn_buffer);
            serialize_message(&turn_notification, turn_buffer);
            send(original_sender_player->socket_fd, turn_buffer, strlen(turn_buffer), 0);
            send(reply_player->socket_fd, turn_buffer, strlen(turn_buffer), 0);
            log_info("%s\n", turn_buffer);
            printf("Game session created between (%s) and (%s).\n", original_sender_player->username, reply_player->username);
            log_info("Game session created between (%s) and (%s).\n", original_sender_player->username, reply_player->username);
        } else {
            printf("(%s) rejected the invitation from (%s).\n", reply_player->username, original_sender);
            log_info("(%s) rejected the invitation from (%s).\n", reply_player->username, original_sender);
        }
        remove_invitation(server->invitation_table, original_sender_player->username, reply_player->username);
    } else {
        create_message(&response, MSG_ERROR, "Invalid response format or one player is already in game.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    }
    return 0;
}

int process_request_attack(Server *server, int client_index, BSMessage *request) {
    BSMessage response;
    char response_buffer[BUFFER_SIZE];
    Player *existing_player = get_player_by_socket(server->player_table, server->clients[client_index].fd);
    if (!existing_player) {
        create_message(&response, MSG_ERROR, "You are not logged in.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    }
    game_session_t *session = find_game_session(server->game_session_table, existing_player);
    Player *defender = strcmp(existing_player->username, session->player1->username) == 0 ? session->player2 : session->player1;
    if (!session) {
        create_message(&response, MSG_ERROR, "You are not in a game.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    }
    if (strcmp(existing_player->username, session->current_turn == 0 ? session->player1->username : session->player2->username) != 0) {
        create_message(&response, MSG_ERROR, "It's not your turn.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    }
    int x, y;
    sscanf(request->data, "%d %d", &x, &y);
    attack_result_t result = process_attack(session, x, y);
    snprintf(response_buffer, BUFFER_SIZE, "%s %d %d %s",existing_player->username, x, y, attack_result_to_str(result));
    create_message(&response, MSG_ATTACK_RESULT, response_buffer);
    serialize_message(&response, response_buffer);
    send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
    send(defender->socket_fd, response_buffer, strlen(response_buffer), 0);
    log_info("%s\n", response_buffer);
    // Check if the game is over
    if (session->winner != -1) {
        char game_over_buffer[BUFFER_SIZE];
        snprintf(game_over_buffer, BUFFER_SIZE, "%s", session->winner == 0 ? session->player1->username : session->player2->username);
        print_game_sessions(server->game_session_table);
        create_message(&response, MSG_GAME_OVER, game_over_buffer);
        serialize_message(&response, response_buffer);
        send(session->player1->socket_fd, response_buffer, strlen(response_buffer), 0);
        send(session->player2->socket_fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        remove_game_session(server->game_session_table, existing_player);
        return 0;
    }
    // Notify the player that has the turn
    BSMessage turn_notification;
    char turn_buffer[BUFFER_SIZE];
    snprintf(turn_buffer, BUFFER_SIZE, "%s", session->current_turn == 0 ? session->player1->username : session->player2->username);
    create_message(&turn_notification, MSG_TURN, turn_buffer);
    serialize_message(&turn_notification, turn_buffer);
    send(session->player1->socket_fd, turn_buffer, strlen(turn_buffer), 0);
    send(session->player2->socket_fd, turn_buffer, strlen(turn_buffer), 0);
    log_info("%s\n", turn_buffer);
    return 0;
}

int process_request_place_ships(Server *server, int client_index) {
    BSMessage response;
    char response_buffer[BUFFER_SIZE];
    Player *existing_player = get_player_by_socket(server->player_table, server->clients[client_index].fd);
    if (!existing_player) {
        create_message(&response, MSG_ERROR, "You are not logged in.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    }
    game_session_t *session = find_game_session(server->game_session_table, existing_player);
    if (!session) {
        create_message(&response, MSG_ERROR, "You are not in a game.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    }
    if (strcmp(existing_player->username, session->player1->username) == 0) {
        char board_buffer[BUFFER_SIZE];
        // test the get_ships_str function
        create_message(&response, MSG_OK, get_ships_str(&session->player1_board));
        serialize_message(&response, board_buffer);
        send(server->clients[client_index].fd, board_buffer, strlen(board_buffer), 0);
        log_info("%s\n", board_buffer);
        return 0;
    } else {
        char board_buffer[BUFFER_SIZE];
        create_message(&response, MSG_OK, get_ships_str(&session->player2_board));
        serialize_message(&response, board_buffer);
        send(server->clients[client_index].fd, board_buffer, strlen(board_buffer), 0);
        log_info("%s\n", board_buffer);
        return 0;
    }
}

int process_request_surrender(Server *server, int client_index) {
    BSMessage response;
    char response_buffer[BUFFER_SIZE];
    Player *existing_player = get_player_by_socket(server->player_table, server->clients[client_index].fd);
    if (!existing_player) {
        create_message(&response, MSG_ERROR, "You are not logged in.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    }
    game_session_t *session = find_game_session(server->game_session_table, existing_player);
    if (!session) {
        create_message(&response, MSG_ERROR, "You are not in a game.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return -1;
    }
    if (strcmp(existing_player->username, session->player1->username) == 0) {
        session->winner = 1;
    } else {
        session->winner = 0;
    }
    char game_over_buffer[BUFFER_SIZE];
    snprintf(game_over_buffer, BUFFER_SIZE, "%s", session->winner == 0 ? session->player1->username : session->player2->username);
    create_message(&response, MSG_GAME_OVER, game_over_buffer);
    serialize_message(&response, response_buffer);
    send(session->player1->socket_fd, response_buffer, strlen(response_buffer), 0);
    send(session->player2->socket_fd, response_buffer, strlen(response_buffer), 0);
    log_info("%s\n", response_buffer);
    remove_game_session(server->game_session_table, existing_player);
    return 0;
}