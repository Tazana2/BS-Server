#ifndef SERVER_H
#define SERVER_H

#define PORT 8080
#define MAX_CLIENTS 50
#define BUFFER_SIZE 1024


void handle_new_connection();
void handle_client_message(int i);
void broadcast_message(const char *message, int exclude_sock);


#endif