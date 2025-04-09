#ifndef PLAYER_H
#define PLAYER_H

#include <stdlib.h>
#include <string.h>
#include <time.h>

#define USERNAME_MAX_LEN 32

typedef struct {
    char username[USERNAME_MAX_LEN];
    int socket_fd;
    int in_game; // Indicates whether the player is in game (1) or not (0)
    time_t turn_start_time; // Timestamp of when the turn started
} Player;

Player *create_player(const char *username, int socket_fd);
void destroy_player(Player *player);

#endif // PLAYER_H