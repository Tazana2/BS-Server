#include "game_session.h"

GameSessionTable *create_game_session_table() {
    GameSessionTable *table = (GameSessionTable *)malloc(sizeof(GameSessionTable));
    if (!table) {
        return NULL;
    }
    table->head = NULL;
    return table;
}

game_session_t *find_game_session(GameSessionTable *table, Player *player1, Player *player2) {
    if (!table) return NULL;
    game_session_t *current = table->head;
    while (current) {
        if ((strcmp(current->player1->username, player1->username) == 0 && strcmp(current->player2->username, player2->username) == 0) ||
            (strcmp(current->player1->username, player2->username) == 0 && strcmp(current->player2->username, player1->username) == 0)) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

attack_result_t process_attack(game_session_t *session, int x, int y) {
    if (session->stage == GAME_OVER) {
        return MISS; // Game is already over
    }

    attack_result_t result;
    if (session->current_turn == 0) {
        result = attack(&session->player2_board, x, y);
    } else {
        result = attack(&session->player1_board, x, y);
    }

    if (session->player1_board.sunk_count == session->player1_board.ship_count) {
        session->stage = GAME_OVER;
        session->winner = 1;
    } else if (session->player2_board.sunk_count == session->player2_board.ship_count) {
        session->stage = GAME_OVER;
        session->winner = 0;
    }

    session->current_turn = 1 - session->current_turn; // Switch turns
    return result;
}

void add_game_session(GameSessionTable *table, Player *player1, Player *player2) {
    if (!table) return;
    game_session_t *new_session = (game_session_t *)malloc(sizeof(game_session_t));
    if (!new_session) return;

    new_session->player1 = player1;
    new_session->player2 = player2;

    initialize_board(&new_session->player1_board);
    initialize_board(&new_session->player2_board);
    
    new_session->current_turn = 0; // 0 for player1, 1 for player2
    new_session->winner = -1; // -1 indicates no winner yet
    new_session->stage = ATTACKING; // Assuming the game starts in the attacking stage for testing purposes
    new_session->next = table->head;
    table->head = new_session;
}

void remove_game_session(GameSessionTable *table, Player *player1, Player *player2) {
    if (!table) return;
    game_session_t *current = table->head, *prev = NULL;
    while (current) {
        if ((strcmp(current->player1->username, player1->username) == 0 && strcmp(current->player2->username, player2->username) == 0) ||
            (strcmp(current->player1->username, player2->username) == 0 && strcmp(current->player2->username, player1->username) == 0)) {
            if (prev) {
                prev->next = current->next;
            } else {
                table->head = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

void destroy_game_session_table(GameSessionTable *table) {
    if (!table) return;
    game_session_t *current = table->head;
    while (current) {
        game_session_t *temp = current;
        current = current->next;
        free(temp);
    }
    free(table);
}

void print_game_sessions(GameSessionTable *table) {
    if (!table) return;
    game_session_t *current = table->head;
    while (current) {
        printf("- Game Session: %s vs %s\n", current->player1->username, current->player2->username);
        printf("    Current Turn: Player %d\n", current->current_turn + 1);
        printf("    Stage: %d\n", current->stage);
        printf("    Winner: %d\n", current->winner);
        current = current->next;
    }
}