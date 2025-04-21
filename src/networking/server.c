#include "../utils/headers/server.h"

Server *create_server(const char *ip, int port) {
    Server *server = (Server *)malloc(sizeof(Server));
    if (!server) {
        log_error("Couldn't allocate memory for server.\n");
        perror("ERROR: Couldn't allocate memory for server.");
        return NULL;
    }
    
    server->player_table = create_player_table();
    server->invitation_table = create_invitation_table();
    server->game_session_table = create_game_session_table();
    if (!server->player_table || !server->invitation_table || !server->game_session_table) {
        log_error("Something went wrong with player, game or invitation table.\n");
        perror("ERROR: Something went wrong with player, game or invitation table.");
        destroy_player_table(server->player_table);
        destroy_invitation_table(server->invitation_table);
        destroy_game_session_table(server->game_session_table);
        free(server);
        return NULL;
    }
    
    server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_fd == -1) {
        log_error("Something went wrong with socket.\n");
        perror("ERROR: Something went wrong with socket.");
        destroy_player_table(server->player_table);
        destroy_invitation_table(server->invitation_table);
        destroy_game_session_table(server->game_session_table);
        free(server);
        return NULL;
    }
    int opt = 1;
    setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        log_error("Invalid IP address.\n");
        perror("ERROR: Invalid IP address.");
        close(server->server_fd);
        destroy_player_table(server->player_table);
        destroy_invitation_table(server->invitation_table);
        destroy_game_session_table(server->game_session_table);
        free(server);
        return NULL;
    }
    
    if (bind(server->server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        log_error("Something went wrong with bind.\n");
        perror("ERROR: Something went wrong with bind.");
        close(server->server_fd);
        destroy_player_table(server->player_table);
        destroy_invitation_table(server->invitation_table);
        destroy_game_session_table(server->game_session_table);
        free(server);
        return NULL;
    }
    if (listen(server->server_fd, MAX_CLIENTS) == -1) {
        log_error("Something went wrong with listen.\n");
        perror("ERROR: Something went wrong with listen.");
        close(server->server_fd);
        destroy_player_table(server->player_table);
        destroy_invitation_table(server->invitation_table);
        destroy_game_session_table(server->game_session_table);
        free(server);
        return NULL;
    }
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        server->clients[i].fd = -1;
        server->clients[i].events = POLLIN;
    }
    
    printf("Server running on: %s:%d.\n", ip, port);
    log_info("Server running on: %s:%d.\n", ip, port);
    return server;
}

void run_server(Server *server) {
    if (!server) return;
    
    while (1) {
        server->clients[0].fd = server->server_fd;
        server->clients[0].events = POLLIN;
        
        int activity = poll(server->clients, MAX_CLIENTS, 100);
        if (activity == -1) {
            perror("ERROR: Something went wrong with poll.");
            log_error("Something went wrong with poll.\n");
            break;
        }
        if (server->clients[0].revents & POLLIN) {
            accept_new_client(server);
        }
        
        for (int i = 1; i < MAX_CLIENTS; i++) {
            if (server->clients[i].fd != -1 && (server->clients[i].revents & POLLIN)) {
                char buffer[BUFFER_SIZE];
                Player *player = get_player_by_socket(server->player_table, server->clients[i].fd);
                int bytes_received = read(server->clients[i].fd, buffer, BUFFER_SIZE);
                
                if (bytes_received <= 0) {
                    if (player) {
                        remove_player(server->player_table, server->clients[i].fd);
                        game_session_t *session = find_game_session(server->game_session_table, player);
                        if (session) {
                            BSMessage game_over;
                            char game_over_buffer[BUFFER_SIZE];
                            snprintf(game_over_buffer, BUFFER_SIZE, "%s", strcmp(session->player1->username, player->username) == 0 ? session->player2->username : session->player1->username);
                            create_message(&game_over, MSG_GAME_OVER, game_over_buffer);
                            serialize_message(&game_over, game_over_buffer);
                            if (strcmp(session->player1->username, player->username) == 0) {
                                send(session->player2->socket_fd, game_over_buffer, strlen(game_over_buffer), 0);
                                log_info("%s\n", game_over_buffer);
                            } else {
                                send(session->player1->socket_fd, game_over_buffer, strlen(game_over_buffer), 0);
                                log_info("%s\n", game_over_buffer);
                            }
                            remove_game_session(server->game_session_table, player);
                        }
                    }
                    printf("Client disconnected on socket descriptor: %d.\n", server->clients[i].fd);
                    log_info("Client disconnected on socket descriptor: %d.\n", server->clients[i].fd);
                    close(server->clients[i].fd);
                    server->clients[i].fd = -1;
                } else {
                    buffer[bytes_received] = '\0';
                    process_message(server, i, buffer);
                    // print_game_sessions(server->game_session_table); // For debugging purposes
                    // print_invitations(server->invitation_table); // For debugging purposes
                }
            }
        }

        time_t now = time(NULL);

        for (int i = 1; i < MAX_CLIENTS; i++) {
            if (server->clients[i].fd != -1) {
                Player *player = get_player_by_socket(server->player_table, server->clients[i].fd);
                if (player) {
                    if (!player->in_game) continue;
                    game_session_t *session = find_game_session(server->game_session_table, player);
                    if (strcmp(player->username, session->current_turn == 0 ? session->player1->username : session->player2->username)) continue;
                    
                    double elapsed = difftime(now, player->turn_start_time);
                    if (elapsed > 30.0) {
                        BSMessage request;
                        strcpy(request.data, "-1 -1");
                        request.header = MSG_ATTACK;
                        process_request_attack(server, i, &request);
                        printf("Turn expired for player %s.\n", player->username);
                    } else {
                        // printf("Turn time left for player %s: %.0f seconds.\n", player->username, 30.0 - elapsed); // For debugging purposes
                    }
                }
            }
        }
    }
}

void stop_server(Server *server) {
    if (!server) return;
    close(server->server_fd);
    destroy_player_table(server->player_table);
    destroy_invitation_table(server->invitation_table);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].fd != -1) {
            close(server->clients[i].fd);
        }
    }
    
    free(server);
    printf("Server Stopped.\n");
    log_info("Server Stopped.\n");
}

void accept_new_client(Server *server) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int new_socket = accept(server->server_fd, (struct sockaddr *)&client_addr, &client_len);
    
    if (new_socket == -1) {
        perror("ERROR: Something went wrong with accept.");
        log_error("Something went wrong with accept.\n");
        return;
    }
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].fd == -1) {
            server->clients[i].fd = new_socket;
            printf("New connection on socket descriptor: %d.\n", new_socket);
            log_info("New connection on socket descriptor: %d.\n", new_socket);
            return;
        }
    }
    
    printf("Server is full, rejecting connection.\n");
    log_info("Server is full, rejecting connection.\n");
    close(new_socket);
}

void process_message(Server *server, int client_index, const char *buffer) {
    BSMessage request, response;
    if (parse_message(&request, buffer) == -1) {
        printf("Couldn't parse the request.\n");
        log_error("Couldn't parse the request.\n");
        char response_buffer[BUFFER_SIZE];
        create_message(&response, MSG_ERROR, "Invalid request format.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        log_info("%s\n", response_buffer);
        return;
    }
    request.data[strcspn(request.data, "\n")] = 0;
    char response_buffer[BUFFER_SIZE];
    // printf("Raw request:%s.\n", request.data); // For debugging purposes
    
    switch (request.header) {
        case MSG_LOGIN: {
            process_request_login(server, client_index, &request);
            break;
        }
        case MSG_LOGOUT: {
            process_request_logout(server, client_index);
            break;
        }
        case MSG_USER_LIST: {
            process_request_user_list(server, client_index);
            break;
        }
        case MSG_INVITE_SEND: {
            process_request_invite_send(server, client_index, &request);
            break;
        }
        case MSG_INVITE_REPLY: {
            process_request_invite_reply(server, client_index, &request);
            break;
        }
        case MSG_ATTACK: {
            process_request_attack(server, client_index, &request);
            break;
        }
        case MSG_PLACE_SHIPS: {
            process_request_place_ships(server, client_index);
            break;
        }
        case MSG_SURRENDER: {
            process_request_surrender(server, client_index);
            break;
        }
        default:
            printf("Unknown message: %s\n", request.data);
            log_info("Unknown message: %s\n", request.data);
            create_message(&response, MSG_ERROR, "Unknown command.");
            serialize_message(&response, response_buffer);
            send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
            log_info("%s\n", response_buffer);
            break;
    }
    memset(&response, 0, sizeof(BSMessage));
    memset(response_buffer, 0, sizeof(response_buffer));
}