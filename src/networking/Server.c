#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include "Server.h"
#include "protocol/BSProtocol.h"

typedef struct {
    int socket;
    int opponent;
    int in_game;
    char username[50];
} Player;

int server_sock;
struct pollfd clients[MAX_CLIENTS + 1];
Player players[MAX_CLIENTS];
int num_clients = 0;

void handle_new_connection() {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
    
    if (num_clients < MAX_CLIENTS) {
        clients[++num_clients].fd = client_sock;
        clients[num_clients].events = POLLIN;
        players[num_clients - 1] = (Player){client_sock, -1, 0, ""};
        printf("New Player connected on port: %d\n", client_sock);
    } else {
        close(client_sock);
    }
}

void broadcast_message(const char *message, int exclude_sock) {
    for (int i = 0; i < num_clients; i++) {
        if (players[i].socket != exclude_sock) {
            send(players[i].socket, message, strlen(message), 0);
        }
    }
}

void handle_client_message(int i) {
    char buffer[BUFFER_SIZE] = {0};
    int bytes_read = read(clients[i].fd, buffer, sizeof(buffer));

    if (bytes_read <= 0) {
        close(clients[i].fd);
        clients[i] = clients[num_clients];
        players[i - 1] = players[num_clients - 1];
        num_clients--;
    } else {
        printf("Message received on port (%d): %s\n",clients[i].fd, buffer);

        // Parsear el mensaje recibido
        MyBSMessage msg;
        if (parse_message(buffer, &msg) < 0) {
            printf("Error al parsear mensaje de %d.\n", clients[i].fd);
            return;
        }

        switch (msg.type) {
            case MSG_LOGIN: {
                printf("Player on port (%d) is trying to Log In.\n", clients[i].fd);
                MyBSMessage response;

                char welcome_msg[MAX_MESSAGE_SIZE] = "Welcome to BattleShip ";
                strcat(welcome_msg, msg.data);
                welcome_msg[MAX_DATA_SIZE-1] = '\0';

                create_message(&response, MSG_LOGIN_OK, welcome_msg);
                char response_buffer[MAX_MESSAGE_SIZE];
                serialize_message(&response, response_buffer);
                write(clients[i].fd, response_buffer, strlen(response_buffer));
                break;
            }
        
            default:
                break;
        }
    }
}

int main() {
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("ERROR - Create socket failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR - Binding failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_sock, MAX_CLIENTS) < 0) {
        perror("ERROR - Listening failed");
        exit(EXIT_FAILURE);
    }

    clients[0].fd = server_sock;
    clients[0].events = POLLIN;

    printf("BattleShip Server on port: %d\n", PORT);

    while (1) {
        int ret = poll(clients, num_clients + 1, -1);
        if (ret < 0) {
            perror("ERROR - Poll failed");
            break;
        }
        
        if (clients[0].revents & POLLIN) {
            handle_new_connection();
        }

        for (int i = 1; i <= num_clients; i++) {
            if (clients[i].revents & POLLIN) {
                handle_client_message(i);
            }
        }
    }
    close(server_sock);
    return 0;
}