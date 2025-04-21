#include "../utils/headers/player.h"

Player *create_player(const char *username, int socket_fd) {
    Player *player = (Player *)malloc(sizeof(Player));
    if (player) {
        strncpy(player->username, username, USERNAME_MAX_LEN - 1);
        player->username[USERNAME_MAX_LEN - 1] = '\0';
        player->socket_fd = socket_fd;
        player->in_game = 0;
        player->turn_start_time = 0;
    }
    return player;
}

void destroy_player(Player *player) {
    if (player) {
        free(player);
    }
}
