#ifndef PLAYER_TABLE_H
#define PLAYER_TABLE_H

#include "player.h"
#include <stdlib.h>
#include <string.h>

#define MAX_PLAYERS 100

typedef struct PlayerNode {
    Player *player;
    struct PlayerNode *next;
} PlayerNode;

typedef struct {
    PlayerNode *table[MAX_PLAYERS];
} PlayerTable;

PlayerTable *create_player_table();
Player *add_player(PlayerTable *table, const char *username, int socket_fd);
Player *get_player(PlayerTable *table, const char *username);
Player *get_player_by_socket(PlayerTable *table, int socket_fd);
int remove_player(PlayerTable *table, const char *username, const int socket_fd);
void get_user_list(PlayerTable *table, char *user_list);
void destroy_player_table(PlayerTable *table);

#endif // PLAYER_TABLE_H