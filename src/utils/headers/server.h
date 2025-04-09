#ifndef SERVER_H
#define SERVER_H

#include "aux.h"
#include "request_handler.h"
#include "logger.h"

Server *create_server(const char *ip, int port);
void run_server(Server *server);
void stop_server(Server *server);
void accept_new_client(Server *server);
void process_message(Server *server, int client_index, const char *buffer);

#endif // SERVER_H