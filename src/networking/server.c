#include "server.h"
#include "../game_logic/game_session.h"

Server *create_server(int port) {
    Server *server = (Server *)malloc(sizeof(Server));
    if (!server) {
        perror("ERROR: Couldn't allocate memory for server.");
        return NULL;
    }
    
    server->player_table = create_player_table();
    server->invitation_table = create_invitation_table();
    server->game_session_table = create_game_session_table();
    if (!server->player_table || !server->invitation_table || !server->game_session_table) {
        perror("ERROR: Something went wrong with player, game or invitation table.");
        destroy_player_table(server->player_table);
        destroy_invitation_table(server->invitation_table);
        destroy_game_session_table(server->game_session_table);
        free(server);
        return NULL;
    }
    
    server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_fd == -1) {
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
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server->server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("ERROR: Something went wrong with bind.");
        close(server->server_fd);
        destroy_player_table(server->player_table);
        destroy_invitation_table(server->invitation_table);
        destroy_game_session_table(server->game_session_table);
        free(server);
        return NULL;
    }
    
    if (listen(server->server_fd, MAX_CLIENTS) == -1) {
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
    
    printf("Server running on port: %d.\n", port);
    return server;
}

void run_server(Server *server) {
    if (!server) return;
    
    while (1) {
        server->clients[0].fd = server->server_fd;
        server->clients[0].events = POLLIN;
        
        int activity = poll(server->clients, MAX_CLIENTS, -1);
        if (activity == -1) {
            perror("ERROR: Something went wrong with poll.");
            break;
        }
        if (server->clients[0].revents & POLLIN) {
            accept_new_client(server);
        }
        
        for (int i = 1; i < MAX_CLIENTS; i++) {
            if (server->clients[i].fd != -1 && (server->clients[i].revents & POLLIN)) {
                char buffer[BUFFER_SIZE];
                int bytes_received = read(server->clients[i].fd, buffer, BUFFER_SIZE);
                
                if (bytes_received <= 0) {
                    Player *player = get_player_by_socket(server->player_table, server->clients[i].fd);
                    if (player) {
                        remove_player(server->player_table, server->clients[i].fd);
                    }
                    close(server->clients[i].fd);
                    server->clients[i].fd = -1;
                    printf("Client disconnected.\n");
                } else {
                    buffer[bytes_received] = '\0';
                    process_message(server, i, buffer);
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
}

void accept_new_client(Server *server) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int new_socket = accept(server->server_fd, (struct sockaddr *)&client_addr, &client_len);
    
    if (new_socket == -1) {
        perror("ERROR: Something went wrong with accept.");
        return;
    }
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].fd == -1) {
            server->clients[i].fd = new_socket;
            printf("New connection on socket: %d.\n", new_socket);
            return;
        }
    }
    
    printf("Server is full, rejecting connection.\n");
    close(new_socket);
}

void process_message(Server *server, int client_index, const char *buffer) {
    BSMessage request, response;
    if (parse_message(&request, buffer) == -1) {
        printf("Couldn't parse the request.\n");
        char response_buffer[BUFFER_SIZE];
        create_message(&response, MSG_ERROR, "Invalid request format.");
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
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
        case MSG_ATTACK: {
            process_request_attack(server, client_index, &request);
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
        default:
            printf("Unknown message: %s\n", request.data);
            create_message(&response, MSG_ERROR, "Unknown command.");
            serialize_message(&response, response_buffer);
            send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
            break;
    }
    memset(&response, 0, sizeof(BSMessage));
    memset(response_buffer, 0, sizeof(response_buffer));
}

int main() {
    Server *server = create_server(SERVER_PORT);
    
    if (!server) {
        fprintf(stderr, "Something went wrong while starting the server.\n");
        return EXIT_FAILURE;
    }

    run_server(server);
    stop_server(server);
    return EXIT_SUCCESS;
}