#include "board.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>

void initialize_board(board_t *board) {
    memset(board->grid, '~', sizeof(board->grid)); // Fill the grid with water
    const int ship_sizes[NUM_SHIPS] = {5, 4, 3, 3, 2, 2, 1, 1, 1}; // The 9 ships that are requiered
    board->ship_count = NUM_SHIPS; // Example: 3 ships
    board->sunk_count = 0;

    srand(time(NULL));

    int placed_ships = 0;
    while (placed_ships < NUM_SHIPS) {
        int size = ship_sizes[placed_ships];
        int x_start, y_start, x_end, y_end;
        int horizontal = rand() % 2;

        do {
            x_start = rand() % 10;
            y_start = rand() % 10;

            if (horizontal) {
                x_end = x_start;
                y_end = y_start + size - 1;
            } else {
                x_end = x_start + size - 1;
                y_end = y_start;
            }
        } while (x_end >= 10 || y_end >= 10 || !is_valid_position(board, x_start, y_start, x_end, y_end));
        
        board->ships[placed_ships] = (Ship){x_start, y_start, x_end, y_end, 0, size};
        for (int x = x_start; x <= x_end; x++) {
            for (int y = y_start; y <= y_end; y++) {
                board->grid[x][y] = 'S';
            }
        }
        placed_ships++;
    }
}

int is_valid_position(board_t *board, int x_start, int y_start, int x_end, int y_end) {
    for (int x = x_start; x <= x_end; x++) {
        for (int y = y_start; y <= y_end; y++) {
            if (board->grid[x][y] != '~') {
                return 0;
            }
        }
    }
    return 1;
}

attack_result_t attack(board_t *board, int x, int y) {
    if (x < 0 || x >= 10 || y < 0 || y >= 10) {
        return MISS; // Out of bounds
    }

    if (board->grid[x][y] == 'S') {
        board->grid[x][y] = 'X';
        
        // Look for the ship hit
        for (int i = 0; i < board->ship_count; i++) { 
            Ship *ship = &board->ships[i];

            if (ship->x_start == ship->x_end) { // Horizontal
                if (x == ship->x_start && y >= ship->y_start && y <= ship->y_end) {
                    ship->hits++;
                }
            } else { // Vertical
                if (y == ship->y_start && x >= ship->x_start && x <= ship->x_end) {
                    ship->hits++;
                }
            }

            if (ship->hits == ship->size) {
                board->sunk_count++;
                return SINK;
            }
        }
        return HIT;
    } else {
        return MISS;
    }
}