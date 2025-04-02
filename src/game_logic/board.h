#ifndef BOARD_H
#define BOARD_H

typedef enum {
    HIT,
    MISS,
    SINK
} attack_result_t;

typedef struct {
    char grid[10][10]; // 10x10 grid for the board
    int ship_count;    // Number of ships on the board
    int sunk_count;    // Number of ships sunk
} board_t;

void initialize_board(board_t *board);

attack_result_t attack(board_t *board, int x, int y);

#endif // BOARD_H