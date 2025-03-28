#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include "protocol/BSProtocol.h"

#define PORT 8080
#define MAX_CLIENTS 50
#define BUFFER_SIZE 1024

struct Player {
    int socket;
    int opponent;
    int in_game;
    // TODO: LOGIN protocol
    // char username[100];
};

int server_sock;
struct pollfd clients[MAX_CLIENTS + 1];
struct Player players[MAX_CLIENTS];
int num_clients = 0;

void setup_server();
void handle_new_connection();
void handle_client_message(int i);
void broadcast_message(const char *message, int exclude_sock);

int main() {
    setup_server();
    
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

void setup_server() {
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
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
}

void handle_new_connection() {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
    
    if (num_clients < MAX_CLIENTS) {
        clients[++num_clients].fd = client_sock;
        clients[num_clients].events = POLLIN;
        players[num_clients - 1] = (struct Player){client_sock, -1, 0};
        printf("New Player connected on port: %d\n", client_sock);
    } else {
        close(client_sock);
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
        printf("Message received from: %d: %s\n", clients[i].fd, buffer);
        // Aquí se debe procesar el mensaje con mybsprotocol
    }
}

void broadcast_message(const char *message, int exclude_sock) {
    for (int i = 0; i < num_clients; i++) {
        if (players[i].socket != exclude_sock) {
            send(players[i].socket, message, strlen(message), 0);
        }
    }
}