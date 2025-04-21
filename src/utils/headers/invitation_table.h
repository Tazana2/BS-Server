#ifndef INVITATION_TABLE_H
#define INVITATION_TABLE_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_INVITATIONS 100

typedef struct Invitation {
    char sender[50];
    char receiver[50];
    struct Invitation *next;
} Invitation;

typedef struct {
    Invitation *head;
} InvitationTable;

InvitationTable *create_invitation_table();
int find_invitation(InvitationTable *table, const char *sender, const char *receiver);
void add_invitation(InvitationTable *table, const char *sender, const char *receiver);
void remove_invitation(InvitationTable *table, const char *sender, const char *receiver);
void destroy_invitation_table(InvitationTable *table);
void print_invitations(InvitationTable *table); // For debugging purposes

#endif // INVITATION_TABLE_H