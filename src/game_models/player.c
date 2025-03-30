// player.c
#include "player.h"
#include <stdlib.h>
#include <string.h>

Player *create_player(const char *username, int socket_fd) {
    Player *player = (Player *)malloc(sizeof(Player));
    if (player) {
        strncpy(player->username, username, USERNAME_MAX_LEN - 1);
        player->username[USERNAME_MAX_LEN - 1] = '\0';
        player->socket_fd = socket_fd;
        player->in_game = false;
    }
    return player;
}

void destroy_player(Player *player) {
    if (player) {
        free(player);
    }
}
