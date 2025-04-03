#include "board.h"
#include <string.h>

void initialize_board(board_t *board) {
    memset(board->grid, '~', sizeof(board->grid)); // Fill the grid with water
    board->ship_count = 3; // Example: 3 ships
    board->sunk_count = 0;

    // Placing ship in a predefined location for simplicity
    board->ships[0] = (Ship){1, 1, 1, 3, 0, 3}; 
    board->ships[1] = (Ship){3, 5, 4, 5, 0, 2}; 
    board->ships[2] = (Ship){6, 2, 6, 2, 0, 1}; 

    // Ships are placed, checking if they are vertical or horizontal
    for (int i = 0; i < board->ship_count; i++) {
        Ship ship = board->ships[i];
        if (ship.x_start == ship.x_end) {
            for (int y = ship.y_start; y <= ship.y_end;y++) {
                board->grid[ship.x_start][y] = 'S';
            }
        } else {
            for (int x = ship.x_start; x<= ship.x_end; x++) {
                board->grid[x][ship.y_start] = 'S';
            }
        }
    }
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