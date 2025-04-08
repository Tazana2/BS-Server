#include "../utils/headers/invitation_table.h"

InvitationTable *create_invitation_table() {
    InvitationTable *table = (InvitationTable *)malloc(sizeof(InvitationTable));
    if (!table) {
        return NULL;
    }
    table->head = NULL;
    return table;
}

int find_invitation(InvitationTable *table, const char *sender, const char *receiver) {
    if (!table) return 0;
    Invitation *current = table->head;
    while (current) {
        if (strcmp(current->sender, sender) == 0 && strcmp(current->receiver, receiver) == 0) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

void add_invitation(InvitationTable *table, const char *sender, const char *receiver) {
    if (!table) return;
    Invitation *new_inv = (Invitation *)malloc(sizeof(Invitation));
    if (!new_inv) return;
    
    strncpy(new_inv->sender, sender, 49);
    new_inv->sender[49] = '\0';
    strncpy(new_inv->receiver, receiver, 49);
    new_inv->receiver[49] = '\0';
    
    new_inv->next = table->head;
    table->head = new_inv;
}

void remove_invitation(InvitationTable *table, const char *sender, const char *receiver) {
    if (!table) return;
    Invitation *current = table->head, *prev = NULL;
    while (current) {
        if (strcmp(current->sender, sender) == 0 && strcmp(current->receiver, receiver) == 0) {
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

void destroy_invitation_table(InvitationTable *table) {
    if (!table) return;
    Invitation *current = table->head;
    while (current) {
        Invitation *temp = current;
        current = current->next;
        free(temp);
    }
    free(table);
}

void print_invitations(InvitationTable *table) {
    if (!table) return;
    Invitation *current = table->head;
    if (!current) {
        printf("No invitations.\n");
        return;
    }
    printf("Active Invitations:\n");
    while (current) {
        printf("- (%s) invited (%s)\n", current->sender, current->receiver);
        current = current->next;
    }
}
