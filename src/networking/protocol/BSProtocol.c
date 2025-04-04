#include <stdio.h>
#include <string.h>
#include "BSProtocol.h"

const char *message_type_to_str(MessageType header) {
    switch (header) {
        // Session
        case MSG_LOGIN:               return "LOGIN";
        case MSG_LOGOUT:              return "LOGOUT";
        case MSG_USER_LIST:           return "USER_LIST";
        // Invitation
        case MSG_INVITE_SEND:         return "INVITE_SEND";
        case MSG_INVITE_RECEIVE:      return "INVITE_RECEIVE";
        case MSG_INVITE_REPLY:        return "INVITE_REPLY";
        case MSG_INVITE_RESULT:       return "INVITE_RESULT";
        // Game
        case MSG_GAME_START:          return "GAME_START";
        case MSG_ATTACK:              return "ATTACK";
        case MSG_ATTACK_RESULT:       return "ATTACK_RESULT";
        case MSG_TURN:                return "TURN";
        case MSG_ATTACK_INCOMING:     return "ATTACK_INCOMING";
        case MSG_GAME_OVER:           return "GAME_OVER";
        // Status and error messages
        case MSG_OK:                  return "OK";
        case MSG_ACK:                 return "ACK";
        case MSG_ERROR:               return "ERROR";
        case MSG_UNKNOWN:             return "UNKNOWN";
        default:                      return "UNKNOWN_MESSAGE_TYPE";
    }
}

MessageType get_message_type(const char *header) {
    // Session
    if (strcmp(header, "LOGIN") == 0)               return MSG_LOGIN;
    if (strcmp(header, "LOGOUT") == 0)              return MSG_LOGOUT;
    if (strcmp(header, "USER_LIST") == 0)           return MSG_USER_LIST;
    // Invitation
    if (strcmp(header, "INVITE_SEND") == 0)         return MSG_INVITE_SEND;
    if (strcmp(header, "INVITE_RECEIVE") == 0)      return MSG_INVITE_RECEIVE;
    if (strcmp(header, "INVITE_REPLY") == 0)        return MSG_INVITE_REPLY;
    if (strcmp(header, "INVITE_RESULT") == 0)       return MSG_INVITE_RESULT;
    // Game
    if (strcmp(header, "GAME_START") == 0)          return MSG_GAME_START;
    if (strcmp(header, "TURN") == 0)                return MSG_TURN;
    if (strcmp(header, "ATTACK") == 0)              return MSG_ATTACK;
    if (strcmp(header, "ATTACK_RESULT") == 0)       return MSG_ATTACK_RESULT;
    if (strcmp(header, "ATTACK_INCOMING") == 0)     return MSG_ATTACK_INCOMING;
    if (strcmp(header, "GAME_OVER") == 0)           return MSG_GAME_OVER;
    // Status and error messages
    if (strcmp(header, "OK") == 0)                  return MSG_OK;
    if (strcmp(header, "ACK") == 0)                 return MSG_ACK;
    if (strcmp(header, "ERROR") == 0)               return MSG_ERROR;
    return MSG_UNKNOWN; 
}

void create_message(BSMessage *msg, MessageType type, const char *data) {
    msg->header = type;
    if (data) {
        strncpy(msg->data, data, MAX_DATA_SIZE - 1);
        // msg->data[strlen(msg->data) - 1] = '\0';
    } else {
        msg->data[0] = '\0';  // If data is NULL, set it to an empty string
    }
}

// Ensures that the server can understand the message.
void serialize_message(const BSMessage *msg, char *buffer) {
    if (strlen(msg->data) > 0) {
        snprintf(buffer, MAX_MESSAGE_SIZE, "%s|%s<EOF>\n", message_type_to_str(msg->header), msg->data);
    } else {
        snprintf(buffer, MAX_MESSAGE_SIZE, "%s|<EOF>\n", message_type_to_str(msg->header));
    }
}

// Split the message into its components and store them in the BSMessage struct
// The message format is "type|data" (e.g., "LOGIN|username")
int parse_message(BSMessage *msg, const char *buffer) {
    char header[50], data[MAX_DATA_SIZE];

    if (sscanf(buffer, "%49[^|]|%199[^|]", header, data) != 2) {
        return -1;
    }

    msg->header = get_message_type(header);

    if (msg->header != MSG_UNKNOWN && strlen(data) > 0) {
        strncpy(msg->data, data, MAX_DATA_SIZE - 1);
        msg->data[MAX_DATA_SIZE - 1] = '\0';
    } else {
        msg->data[0] = '\0';
    }

    return 0;
}