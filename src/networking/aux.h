#ifndef AUX_H
#define AUX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include "../game_models/player_table.h"
#include "../game_models/invitation_table.h"
#include "../game_logic/game_session.h"
#include "protocol/BSProtocol.h"

#define MAX_CLIENTS 10
#define SERVER_PORT 8080
#define BUFFER_SIZE 256


typedef struct {
  int server_fd;
  struct pollfd clients[MAX_CLIENTS];
  InvitationTable *invitation_table;
  PlayerTable *player_table;
  GameSessionTable *game_session_table;
} Server;

#endif