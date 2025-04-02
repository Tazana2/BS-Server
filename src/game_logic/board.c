#include "board.h"
#include <string.h>

void initialize_board(board_t *board) {
    memset(board->grid, '~', sizeof(board->grid)); // Fill the grid with water
    board->ship_count = 3; // Example: 3 ships
    board->sunk_count = 0;

    // Placing ship in a predefined location for simplicity
    board->grid[1][1] = 'S'; board->grid[1][2] = 'S'; board->grid[1][3] = 'S'; // Ship 1
    board->grid[3][5] = 'S'; board->grid[4][5] = 'S'; // Ship 2
    board->grid[6][2] = 'S'; // Ship 3

    // This would be the representation of the ships on the board:
    // ~ ~ ~ ~ ~ ~ ~
    // ~ S S S ~ ~ ~
    // ~ ~ ~ ~ S ~ ~
    // ~ ~ ~ ~ S ~ ~
    // ~ ~ ~ ~ ~ ~ ~
    // ~ ~ S ~ ~ ~ ~
    // ~ ~ ~ ~ ~ ~ ~
    // ~ ~ ~ ~ ~ ~ ~
    // The rest of the grid is filled with water ('~')
}

attack_result_t attack(board_t *board, int x, int y) {
    if (x < 0 || x >= 10 || y < 0 || y >= 10) {
        return MISS; // Out of bounds
    }

    if (board->grid[x][y] == 'S') {
        board->grid[x][y] = 'X'; // Mark as hit
        int sunk = 1;

        // Check if the ship is sunk
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                if (board->grid[i][j] == 'S') {
                    sunk = 0; // Ship still exists
                    break;
                }
            }
            if (!sunk) break;
        }

        if (sunk) {
            board->sunk_count++;
            return SINK; // Ship sunk
        }
        return HIT; // Ship hit but not sunk
    } else if (board->grid[x][y] == '~') {
        // board->grid[x][y] = 'O'; // Mark as miss
        return MISS; // Missed
    }

    return MISS; // Already attacked this location
}