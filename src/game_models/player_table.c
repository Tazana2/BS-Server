// player_table.c
#include "player_table.h"

#include <stdio.h>

// Función hash simple para calcular el índice basado en el nombre de usuario
static unsigned int hash_function(const char *username) {
    unsigned int hash = 0;
    while (*username) {
        hash = (hash * 31) + *username;
        username++;
    }
    return hash % MAX_PLAYERS;
}

// Crear una nueva tabla de jugadores
PlayerTable *create_player_table() {
    PlayerTable *table = (PlayerTable *)malloc(sizeof(PlayerTable));
    if (table) {
        memset(table->table, 0, sizeof(table->table));
    }
    return table;
}

// Destruir la tabla de jugadores y liberar memoria
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

// Agregar un jugador a la tabla
Player *add_player(PlayerTable *table, const char *username, int socket_fd) {
    if (!table) return NULL;
    
    unsigned int index = hash_function(username);
    // Verificar si el jugador ya existe
    PlayerNode *node = table->table[index];
    while (node) {
        if (strcmp(node->player->username, username) == 0) {
            // El jugador ya existe, no se agrega
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

// Obtener un jugador por su nombre de usuario
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

// Eliminar un jugador de la tabla
int remove_player(PlayerTable *table, const char *username, const int socket_fd) {
    if (!table) return 0;
    
    unsigned int index = hash_function(username);
    PlayerNode *node = table->table[index];
    PlayerNode *prev = NULL;
    
    while (node) {
        if (strcmp(node->player->username, username) == 0 && node->player->socket_fd == socket_fd) {
            if (prev) {
                prev->next = node->next;
            } else {
                table->table[index] = node->next;
            }
            destroy_player(node->player);
            free(node);
            return 1; // Jugador eliminado
        }
        prev = node;
        node = node->next;
    }
    return 0; // Jugador no encontrado o no coincide con el socket_fd
}

// Obtener la lista de jugadores en formato de cadena
void get_user_list(PlayerTable *table, char *user_list) {
    if (!table || !user_list) return;
    
    user_list[0] = '\0'; // Inicializar la cadena
    for (int i = 0; i < MAX_PLAYERS; i++) {
        PlayerNode *node = table->table[i];
        while (node) {
            strcat(user_list, node->player->username);
            strcat(user_list, " ");
            node = node->next;
        }
    }
}