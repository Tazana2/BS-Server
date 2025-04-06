#ifndef GAME_SESSION_H
#define GAME_SESSION_H

#include "board.h"
#include "../game_models/player.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_GAME_SESSION 50


typedef enum {
    PLACING_SHIPS,
    ATTACKING,
    GAME_OVER
} game_stage_t;

typedef struct game_session_t {
    Player *player1;
    Player *player2;
    int current_turn; // 0 for player1, 1 for player2
    int winner;      // 0 for player1, 1 for player2, -1 for draw
    board_t player1_board;
    board_t player2_board;
    game_stage_t stage;
    struct game_session_t *next;
} game_session_t;

typedef struct {
    game_session_t *head;
} GameSessionTable;

GameSessionTable *create_game_session_table();
game_session_t *add_game_session(GameSessionTable *table, Player *player1, Player *player2);
game_session_t *find_game_session(GameSessionTable *table, Player *player);
attack_result_t process_attack(game_session_t *session, int x, int y);
const char *attack_result_to_str(attack_result_t result);
void remove_game_session(GameSessionTable *table, Player *player);
void destroy_game_session_table(GameSessionTable *table);
void print_game_sessions(GameSessionTable *table); // For debugging purposes

#endif // GAME_SESSION_H