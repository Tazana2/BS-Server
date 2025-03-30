// player_table.h
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
void destroy_player_table(PlayerTable *table);
Player *add_player(PlayerTable *table, const char *username, int socket_fd);
Player *get_player(PlayerTable *table, const char *username);
int remove_player(PlayerTable *table, const char *username, const int socket_fd);
void get_user_list(PlayerTable *table, char *user_list);

#endif // PLAYER_TABLE_H