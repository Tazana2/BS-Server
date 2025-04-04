#include "player_table.h"
#include <stdio.h>

// Simple hash function for the player username
static unsigned int hash_function(const char *username) {
    unsigned int hash = 0;
    while (*username) {
        hash = (hash * 31) + *username;
        username++;
    }
    return hash % MAX_PLAYERS;
}

PlayerTable *create_player_table() {
    PlayerTable *table = (PlayerTable *)malloc(sizeof(PlayerTable));
    if (table) {
        memset(table->table, 0, sizeof(table->table));
    }
    return table;
}

Player *add_player(PlayerTable *table, const char *username, int socket_fd) {
    if (!table) return NULL;
    
    unsigned int index = hash_function(username);
    // Check if the player already exists
    PlayerNode *node = table->table[index];
    while (node) {
        // printf("player soc: %d soc: %d\n",node->player->socket_fd, socket_fd); // Debugging line
        if (strcmp(node->player->username, username) == 0) {
            // The player already exists, return NULL
            return NULL;
        }
        node = node->next;
    }
    // printf("Adding player %s at index %u\n", username, index); // Debugging line
    PlayerNode *new_node = (PlayerNode *)malloc(sizeof(PlayerNode));
    if (!new_node) return NULL;
    
    new_node->player = create_player(username, socket_fd);
    new_node->next = table->table[index];
    table->table[index] = new_node;
    
    return new_node->player;
}

Player *get_player(PlayerTable *table, const char *username) {
    if (!table) return NULL;
    
    unsigned int index = hash_function(username);
    // printf("Getting player %s at index %u\n", username, index); // Debugging line
    PlayerNode *node = table->table[index];
    
    while (node) {
        if (strcmp(node->player->username, username) == 0) {
            return node->player;
        }
        node = node->next;
    }
    return NULL;
}

Player *get_player_by_socket(PlayerTable *table, int socket_fd) {
    if (!table) return NULL;
    
    for (int i = 0; i < MAX_PLAYERS; i++) {
        PlayerNode *node = table->table[i];
        while (node) {
            if (node->player->socket_fd == socket_fd) {
                return node->player;
            }
            node = node->next;
        }
    }
    return NULL;
}

int remove_player(PlayerTable *table, const int socket_fd) {
    if (!table) return 0;
    Player *player = get_player_by_socket(table, socket_fd);
    if (!player) return 0; // Player not found
    unsigned int index = hash_function(player->username);
    PlayerNode *node = table->table[index];
    PlayerNode *prev = NULL;
    
    while (node) {
        if (strcmp(node->player->username, player->username) == 0 && node->player->socket_fd == socket_fd) {
            if (prev) {
                prev->next = node->next;
            } else {
                table->table[index] = node->next;
            }
            printf("User (%s) disconnected.\n", node->player->username);
            destroy_player(node->player);
            free(node);
            return 1; // Player removed
        }
        prev = node;
        node = node->next;
    }
    return 0; // Player not found or the socket_fd does not match
}

void get_user_list(PlayerTable *table, char *user_list) {
    if (!table || !user_list) return;
    
    user_list[0] = '\0'; // Inicializar la cadena
    for (int i = 0; i < MAX_PLAYERS; i++) {
        PlayerNode *node = table->table[i];
        while (node) {
            strcat(user_list, node->player->username);
            strcat(user_list, ":");
            strcat(user_list, node->player->in_game ? "In Game" : "Available");
            strcat(user_list, " ");
            node = node->next;
        }
    }
}

void destroy_player_table(PlayerTable *table) {
    if (!table) return;
    
    for (int i = 0; i < MAX_PLAYERS; i++) {
        PlayerNode *node = table->table[i];
        while (node) {
            PlayerNode *temp = node;
            node = node->next;
            destroy_player(temp->player);
            free(temp);
        }
    }
    free(table);
}