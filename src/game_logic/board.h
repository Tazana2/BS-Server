#ifndef BOARD_H
#define BOARD_H

#define BOARD_SIZE 10
#define MAX_SHIPS 9

typedef enum {
    HIT,
    MISS,
    SINK
} attack_result_t;

typedef struct {
    int x_start, y_start;
    int x_end, y_end;
    int hits; 
    int size;
} Ship;

typedef struct {
    char grid[BOARD_SIZE][BOARD_SIZE]; // 10X10 grid for the board
    Ship ships[MAX_SHIPS];
    int ship_count;    // Number of ships on the board
    int sunk_count;    // Number of ships sunk
} board_t;

void initialize_board(board_t *board);
attack_result_t attack(board_t *board, int x, int y);
int is_ship_sunk(board_t *board, int ship_index);

#endif // BOARD_H