// server.c
#include "server.h"

// Crear y configurar el servidor
Server *create_server(int port) {
    Server *server = (Server *)malloc(sizeof(Server));
    if (!server) {
        perror("Error al asignar memoria para el servidor");
        return NULL;
    }
    
    server->player_table = create_player_table();
    server->invitation_table = create_invitation_table();
    if (!server->player_table || !server->invitation_table) {
        perror("Error al crear estructuras de datos");
        free(server);
        return NULL;
    }
    
    server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_fd == -1) {
        perror("Error al crear el socket");
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
        perror("Error en bind");
        close(server->server_fd);
        destroy_player_table(server->player_table);
        destroy_invitation_table(server->invitation_table);
        free(server);
        return NULL;
    }
    
    if (listen(server->server_fd, MAX_CLIENTS) == -1) {
        perror("Error en listen");
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
    
    printf("Servidor iniciado en el puerto %d\n", port);
    return server;
}

// Aceptar nuevos clientes
void accept_new_client(Server *server) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int new_socket = accept(server->server_fd, (struct sockaddr *)&client_addr, &client_len);
    
    if (new_socket == -1) {
        perror("Error al aceptar nuevo cliente");
        return;
    }
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].fd == -1) {
            server->clients[i].fd = new_socket;
            printf("Nuevo cliente conectado: %d\n", new_socket);
            return;
        }
    }
    
    printf("Servidor lleno, rechazando nueva conexión\n");
    close(new_socket);
}

// Procesar mensajes recibidos
void process_message(Server *server, int client_index, const char *buffer) {
    MyBSMessage msg;
    if (parse_message(buffer, &msg) == -1) {
        printf("Error al parsear el mensaje\n");
        return;
    }
    
    char response_buffer[BUFFER_SIZE];
    MyBSMessage response;
    msg.data[strcspn(msg.data, "\n")] = 0; // Eliminar el salto de línea al final del mensaje
    
    switch (msg.type) {
        case MSG_LOGIN:
            printf("Usuario intentando iniciar sesión: %s\n", msg.data);
            Player *check = add_player(server->player_table, msg.data, server->clients[client_index].fd);
            if (!check) {
                create_message(&response, MSG_ERROR, "No se pudo iniciar sesión.");
                break;
            }
            printf("Usuario %s conectado con socket %d\n", check->username, check->socket_fd);
            create_message(&response, MSG_OK, "Bienvenido.");
            break;
        case MSG_LOGOUT:
            printf("Usuario cerrando sesión: %s\n", msg.data);
            if (remove_player(server->player_table, msg.data, server->clients[client_index].fd)) {
                create_message(&response, MSG_OK, "Sesión cerrada, Adiós.");
                printf("Usuario %s desconectado\n", msg.data);
            } else {
                printf("Error al cerrar sesión para %s\n", msg.data);
                create_message(&response, MSG_ERROR, "Error al cerrar sesión.");
            }
            break;
        case MSG_USER_LIST:
            printf("Solicitando lista de usuarios\n");
            char user_list[BUFFER_SIZE];
            get_user_list(server->player_table, user_list);
            create_message(&response, MSG_OK, user_list);
            break;
        case MSG_ATTACK:
            printf("Ataque recibido de %s: %s\n", msg.data, msg.data);
            create_message(&response, MSG_ATTACK_RESULT, "Ataque procesado.");
            break;
        case MSG_INVITE: {
            Player *sender_player = get_player_by_socket(server->player_table, server->clients[client_index].fd);
            char receiver[50];
            sscanf(msg.data, "%s", receiver);
            printf("Invitación de %s a %s\n", sender_player->username, receiver);
            Player *target_player = get_player(server->player_table, receiver);
            if (!sender_player || !target_player) {
                create_message(&response, MSG_ERROR, "Usuario no encontrado.");
                break;
            }
            if (strcmp(sender_player->username, receiver) == 0) {
                create_message(&response, MSG_ERROR, "No puedes invitarte a ti mismo.");
                break;
            }
            
            // add_invitation(server->invitation_table, sender, receiver);

            // Notificar al usuario que envía la invitación
            create_message(&response, MSG_OK, "Invitación enviada");

            // Notificar al usuario invitado
            MyBSMessage invite_notification;
            create_message(&invite_notification, MSG_INVITATION_RECEIVED, "Has recibido una invitación");
            char invite_buffer[BUFFER_SIZE];
            serialize_message(&invite_notification, invite_buffer);
            send(target_player->socket_fd, invite_buffer, strlen(invite_buffer), 0);
            break;
        }            
        default:
            printf("Mensaje desconocido: %s\n", msg.data);
            create_message(&response, MSG_ERROR, "Comando no reconocido.");
            break;
    }
    // Enviar respuesta al cliente
    if (response.type == MSG_LOGOUT) {
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        close(server->clients[client_index].fd);
        server->clients[client_index].fd = -1;
    } else {
        serialize_message(&response, response_buffer);
        send(server->clients[client_index].fd, response_buffer, strlen(response_buffer), 0);
        // Limpiar el mensaje de respuesta
        memset(&response, 0, sizeof(MyBSMessage));
        memset(response_buffer, 0, sizeof(response_buffer));
    }
}

// Bucle principal del servidor
void run_server(Server *server) {
    if (!server) return;
    
    while (1) {
        server->clients[0].fd = server->server_fd;
        server->clients[0].events = POLLIN;
        
        int activity = poll(server->clients, MAX_CLIENTS, -1);
        if (activity == -1) {
            perror("Error en poll");
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
                    printf("Cliente desconectado\n");
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

// Detener el servidor y liberar recursos
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
    printf("Servidor detenido\n");
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
