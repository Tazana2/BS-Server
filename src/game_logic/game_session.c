#include "game_session.h"

GameSessionTable *create_game_session_table() {
    GameSessionTable *table = (GameSessionTable *)malloc(sizeof(GameSessionTable));
    if (!table) {
        return NULL;
    }
    table->head = NULL;
    return table;
}

game_session_t *find_game_session(GameSessionTable *table, Player *player) {
    if (!table) return NULL;
    game_session_t *current = table->head;
    while (current) {
        if (strcmp(current->player1->username, player->username) == 0 || strcmp(current->player2->username, player->username) == 0) {
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

game_session_t *add_game_session(GameSessionTable *table, Player *player1, Player *player2) {
    if (!table) return NULL;
    game_session_t *new_session = malloc(sizeof(game_session_t));
    if (!new_session) return NULL;

    new_session->player1 = player1;
    new_session->player2 = player2;
    new_session->player1->in_game = 1;
    new_session->player2->in_game = 1;

    printf("Player 1 board:\n");
    initialize_board(&new_session->player1_board);
    printf("Player 2 board:\n");
    initialize_board(&new_session->player2_board);
    
    new_session->current_turn = 0; // 0 for player1, 1 for player2
    new_session->winner = -1; // -1 indicates no winner yet
    new_session->stage = ATTACKING; // Assuming the game starts in the attacking stage for testing purposes
    new_session->next = table->head;
    table->head = new_session;
    return new_session;
}

const char *attack_result_to_str(attack_result_t result) {
    switch (result) {
        case HIT:
            return "HIT";
        case MISS:
            return "MISS";
        case SINK:
            return "SINK";
        default:
            return "UNKNOWN";
    }
}

void remove_game_session(GameSessionTable *table, Player *player) {
    if (!table) return;
    game_session_t *current = table->head, *prev = NULL;
    while (current) {
        if (strcmp(current->player1->username, player->username) == 0 || strcmp(current->player2->username, player->username) == 0) {
            current->player1->in_game = 0;
            current->player2->in_game = 0;
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
    if (!current) {
        printf("No active game sessions.\n");
        return;
    }
    printf("Active Game Sessions:\n");
    while (current) {
        printf("- Game Session: %s vs %s\n", current->player1->username, current->player2->username);
        printf("    Current Turn: %s\n", current->current_turn == 0 ? current->player1->username : current->player2->username);
        printf("    Stage: %d\n", current->stage);
        printf("    Winner: %s\n", current->winner == 0 ? current->player1->username : (current->winner == 1 ? current->player2->username : "None"));
        current = current->next;
    }
}