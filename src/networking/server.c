#include "server.h"

Server *create_server(int port) {
    Server *server = (Server *)malloc(sizeof(Server));
    if (!server) {
        perror("ERROR: Couldn't allocate memory for server.");
        return NULL;
    }
    
    server->player_table = create_player_table();
    server->invitation_table = create_invitation_table();
    if (!server->player_table || !server->invitation_table) {
        perror("ERROR: Something went wrong with player table or invitation table.");
        free(server);
        return NULL;
    }
    
    server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_fd == -1) {
        perror("ERROR: Something went wrong with socket.");
        destroy_player_table(server->player_table);
        free(server);
        return NULL;
    }
    int opt = 1;
    setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server->server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("ERROR: Something went wrong with bind.");
        close(server->server_fd);
        destroy_player_table(server->player_table);
        destroy_invitation_table(server->invitation_table);
        free(server);
        return NULL;
    }
    
    if (listen(server->server_fd, MAX_CLIENTS) == -1) {
        perror("ERROR: Something went wrong with listen.");
        close(server->server_fd);
        destroy_player_table(server->player_table);
        destroy_invitation_table(server->invitation_table);
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
                int bytes_received = recv(server->clients[i].fd, buffer, BUFFER_SIZE, 0);
                
                if (bytes_received <= 0) {
                    printf("Client disconnected.\n");
                    Player *player = get_player_by_socket(server->player_table, server->clients[i].fd);
                    remove_player(server->player_table, player->username, server->clients[i].fd);
                    close(server->clients[i].fd);
                    server->clients[i].fd = -1;
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
    BSMessage msg;
    if (parse_message(buffer, &msg) == -1) {
        printf("ERROR: Couldn't parse the message.\n");
        return;
    }
    
    char response_buffer[BUFFER_SIZE];
    BSMessage response;
    msg.data[strcspn(msg.data, "\n")] = 0; // If the message has a newline, remove it
    
    switch (msg.type) {
        case MSG_LOGIN:
            printf("Login try with username: %s\n", msg.data);
            Player *check = add_player(server->player_table, msg.data, server->clients[client_index].fd);
            if (!check) {
                create_message(&response, MSG_ERROR, "Couldn't add player.");
                break;
            }
            printf("User (%s) connected on socket: %d\n", check->username, check->socket_fd);
            create_message(&response, MSG_OK, "Welcome.");
            break;
        case MSG_LOGOUT:
            printf("User (%s) is trying to logout.\n", msg.data);
            if (remove_player(server->player_table, msg.data, server->clients[client_index].fd)) {
                create_message(&response, MSG_OK, "Session closed. Bye!");
                printf("User (%s) disconnected.\n", msg.data);
            } else {
                printf("ERROR: Couldn't disconnect (%s).\n", msg.data);
                create_message(&response, MSG_ERROR, "Couldn't disconnect.");
            }
            break;
        case MSG_USER_LIST:
            char user_list[BUFFER_SIZE];
            get_user_list(server->player_table, user_list);
            create_message(&response, MSG_OK, user_list);
            break;
        case MSG_ATTACK:
            printf("Attack command from (%s).\n", msg.data);
            create_message(&response, MSG_ATTACK_RESULT, "For the moment, this is a placeholder for the attack result.");
            break;
        case MSG_INVITE: {
            Player *sender_player = get_player_by_socket(server->player_table, server->clients[client_index].fd);
            char receiver[50];
            sscanf(msg.data, "%s", receiver);
            Player *target_player = get_player(server->player_table, receiver);
            if (!sender_player || !target_player) {
                create_message(&response, MSG_ERROR, "User not found.");
                break;
            }
            if (strcmp(sender_player->username, receiver) == 0) {
                create_message(&response, MSG_ERROR, "You cannot invite yourself.");
                break;
            }
            add_invitation(server->invitation_table, sender_player->username, target_player->username);
            
            // Server response to the sender
            create_message(&response, MSG_OK, "Invitation sent.");
            
            // Notify the target player
            BSMessage invite_notification;
            create_message(&invite_notification, MSG_INVITE_FROM, sender_player->username);
            char invite_buffer[BUFFER_SIZE];
            serialize_message(&invite_notification, invite_buffer);
            send(target_player->socket_fd, invite_buffer, strlen(invite_buffer), 0);
            printf("(%s) just invited (%s) to a game.\n", sender_player->username, receiver);
            break;
        }
        case MSG_INVITE_ACK: {
            char receiver[50], invite_status[10];
            sscanf(msg.data, "%s %s", receiver, invite_status);
            // Verify if the invitation exists
            Player *sender_player = get_player_by_socket(server->player_table, server->clients[client_index].fd);
            Player *target_player = get_player(server->player_table, receiver);
            if (find_invitation(server->invitation_table, target_player->username, sender_player->username) == 0) {
                create_message(&response, MSG_ERROR, "You have no invitation from this user.");
                break;
            }
            
            // Server response to the sender
            create_message(&response, MSG_OK, "Response sent.");
            
            // Notify the target player
            BSMessage invite_notification;
            char invite_buffer[BUFFER_SIZE];
            snprintf(invite_buffer, sizeof(invite_buffer), "%s %s", sender_player->username, invite_status);
            
            create_message(&invite_notification, MSG_INVITE_RESPONSE, invite_buffer);
            serialize_message(&invite_notification, invite_buffer);
            send(target_player->socket_fd, invite_buffer, strlen(invite_buffer), 0);
            printf("(%s) responded the invitation of (%s).\n", sender_player->username, receiver);
            break;
        }            
        default:
            printf("Unknown message: %s\n", msg.data);
            create_message(&response, MSG_ERROR, "Unknown command.");
            break;
    }
    // Send the response to the client
    if (response.type == MSG_LOGOUT) {
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        close(server->clients[client_index].fd);
        server->clients[client_index].fd = -1;
    } else {
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        // Reset the response message to avoid sending old data
        memset(&response, 0, sizeof(BSMessage));
        memset(response_buffer, 0, sizeof(response_buffer));
    }
}

int main() {
    Server *server = create_server(SERVER_PORT);
    
    if (!server) {
        fprintf(stderr, "Error al iniciar el servidor.\n");
        return EXIT_FAILURE;
    }

    run_server(server);
    stop_server(server);
    return EXIT_SUCCESS;
}
