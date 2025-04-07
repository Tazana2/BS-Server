#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "aux.h"

int process_request_login(Server *server, int client_index, BSMessage *request);
int process_request_logout(Server *server, int client_index);
int process_request_user_list(Server *server, int client_index);
int process_request_invite_send(Server *server, int client_index, BSMessage *request);
int process_request_invite_reply(Server *server, int client_index, BSMessage *request);
int process_request_place_ships(Server *server, int client_index);
int process_request_attack(Server *server, int client_index, BSMessage *request);

#endif