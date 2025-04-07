#include "board.h"

void initialize_board(board_t *board) {
    memset(board->grid, '~', sizeof(board->grid)); // Fill the grid with water
    const int ship_sizes[NUM_SHIPS] = {5, 4, 3, 3, 2, 2, 1, 1, 1}; // The 9 ships that are requiered
    // const int ship_sizes[NUM_SHIPS] = {1}; // Debugging purposes
    board->sunk_count = 0;
    
    int placed_ships = 0;
    while (placed_ships < NUM_SHIPS) {
        int size = ship_sizes[placed_ships];
        int x_start, y_start, x_end, y_end;
        int horizontal = rand() % 2;
        
        do {
            x_start = rand() % 10;
            y_start = rand() % 10;
            // printf("X = %d \n", x_start);
            // printf("Y = %d \n", y_start);
            
            if (horizontal) {
                x_end = x_start;
                y_end = y_start + size - 1;
            } else {
                x_end = x_start + size - 1;
                y_end = y_start;
            }
        } while (x_end >= 10 || y_end >= 10 || !is_valid_position(board, x_start, y_start, x_end, y_end));
        
        board->ships[placed_ships] = (Ship){x_start, y_start, x_end, y_end, 0, size};
        if (x_start == x_end) {
            // Barco horizontal
            for (int y = y_start; y <= y_end; y++) {
                board->grid[x_start][y] = 'S';
            }
        } else {
            // Barco vertical
            for (int x = x_start; x <= x_end; x++) {
                board->grid[x][y_start] = 'S';
            }
        }
        
        placed_ships++;
    }

    board->ship_count = placed_ships;
    printf("Ships Count: %d\n", board->ship_count);

    // Print the board for debugging
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            printf("%c ", board->grid[i][j]);
        }
        printf("\n");
    }
    printf("\n");
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

            int x_min = (ship->x_start < ship->x_end) ? ship->x_start : ship->x_end;
            int x_max = (ship->x_start > ship->x_end) ? ship->x_start : ship->x_end;
            int y_min = (ship->y_start < ship->y_end) ? ship->y_start : ship->y_end;
            int y_max = (ship->y_start > ship->y_end) ? ship->y_start : ship->y_end;

            // Check if the (x, y) falls within the ship's range
            if (x >= x_min && x <= x_max && y >= y_min && y <= y_max) {
                ship->hits++;

                if (ship->hits == ship->size) {
                    board->sunk_count++;
                    return SINK;
                }
                return HIT;
            }
        }
    }
    return MISS;
}

// Returns the ships in the format "x_start,y_start,x_end,y_end-..."
char *get_ships_str(board_t *board) {
    char *ships_str = malloc(MAX_DATA_SIZE);
    ships_str[0] = '\0'; // Initialize the string

    for (int i = 0; i < board->ship_count; i++) {
        Ship *ship = &board->ships[i];
        char ship_info[64];
        snprintf(ship_info, sizeof(ship_info), "%d,%d,%d,%d-", ship->x_start, ship->y_start, ship->x_end, ship->y_end);
        strcat(ships_str, ship_info);
    }

    // Remove the last '-' character
    if (strlen(ships_str) > 0) {
        ships_str[strlen(ships_str) - 1] = '\0';
    }

    return ships_str;
}
