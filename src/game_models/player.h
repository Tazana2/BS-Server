// player.h
#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>

#define USERNAME_MAX_LEN 32

typedef struct {
    char username[USERNAME_MAX_LEN];
    int socket_fd;  // Identificador del socket del jugador
    bool in_game;   // Indica si el jugador está en partida
} Player;

Player *create_player(const char *username, int socket_fd);
void destroy_player(Player *player);

#endif // PLAYER_H