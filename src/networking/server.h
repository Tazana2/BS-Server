#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include "../game_models/player_table.h"
#include "../game_models/invitation_table.h"
#include "protocol/BSProtocol.h"

#define MAX_CLIENTS 10
#define SERVER_PORT 12345
#define BUFFER_SIZE 256

typedef struct {
    int server_fd;
    struct pollfd clients[MAX_CLIENTS];
    InvitationTable *invitation_table;
    PlayerTable *player_table;
} Server;

Server *create_server(int port);
void run_server(Server *server);
void stop_server(Server *server);
void accept_new_client(Server *server);

#endif // SERVER_H