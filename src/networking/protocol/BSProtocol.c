#include <stdio.h>
#include <string.h>
#include "BSProtocol.h"

const char *message_type_to_str(MessageType type) {
    switch (type) {
        // Session
        case MSG_LOGIN:               return "LOGIN";
        case MSG_LOGOUT:              return "LOGOUT";
        case MSG_USER_LIST:           return "USER_LIST";
        case MSG_USER_CONNECT:        return "USER_CONNECT";
        case MSG_USER_DISCONNECT:     return "USER_DISCONNECT";
        // Invitation
        case MSG_INVITE:              return "INVITE";
        case MSG_INVITE_FROM:         return "INVITE_FROM";
        case MSG_INVITE_ACK:          return "INVITE_ACK";
        case MSG_INVITE_RESPONSE:     return "INVITE_RESPONSE";
        // Game
        case MSG_GAME_START:          return "GAME_START";
        case MSG_ATTACK:              return "ATTACK";
        case MSG_ATTACK_RESULT:       return "ATTACK_RESULT";
        case MSG_TURN_OPPONENT:       return "TURN_OPPONENT";
        case MSG_ATTACK_INCOMING:     return "ATTACK_INCOMING";
        case MSG_TURN_YOUR:           return "TURN_YOUR";
        case MSG_GAME_OVER:           return "GAME_OVER";
        // Status and error messages
        case MSG_ERROR:               return "ERROR";
        case MSG_OK:                  return "OK";
        case MSG_UNKNOWN:             return "UNKNOWN";
        default:                      return "UNKNOWN_MESSAGE_TYPE";
    }
}

MessageType get_message_type(const char *type_str) {
    // Session
    if (strcmp(type_str, "LOGIN") == 0)               return MSG_LOGIN;
    if (strcmp(type_str, "LOGOUT") == 0)              return MSG_LOGOUT;
    if (strcmp(type_str, "USER_LIST") == 0)           return MSG_USER_LIST;
    if (strcmp(type_str, "USER_CONNECT") == 0)        return MSG_USER_CONNECT;
    if (strcmp(type_str, "USER_DISCONNECT") == 0)     return MSG_USER_DISCONNECT;
    // Invitation
    if (strcmp(type_str, "INVITE") == 0)              return MSG_INVITE;
    if (strcmp(type_str, "INVITE_FROM") == 0)         return MSG_INVITE_FROM;
    if (strcmp(type_str, "INVITE_ACK") == 0)          return MSG_INVITE_ACK;
    if (strcmp(type_str, "INVITE_RESPONSE") == 0)     return MSG_INVITE_RESPONSE;
    // Game
    if (strcmp(type_str, "GAME_START") == 0)          return MSG_GAME_START;
    if (strcmp(type_str, "ATTACK") == 0)              return MSG_ATTACK;
    if (strcmp(type_str, "ATTACK_RESULT") == 0)       return MSG_ATTACK_RESULT;
    if (strcmp(type_str, "TURN_OPPONENT") == 0)       return MSG_TURN_OPPONENT;
    if (strcmp(type_str, "ATTACK_INCOMING") == 0)     return MSG_ATTACK_INCOMING;
    if (strcmp(type_str, "TURN_YOUR") == 0)           return MSG_TURN_YOUR;
    if (strcmp(type_str, "GAME_OVER") == 0)           return MSG_GAME_OVER;
    // Status and error messages
    if (strcmp(type_str, "ERROR") == 0)               return MSG_ERROR;
    if (strcmp(type_str, "OK") == 0)                  return MSG_OK;
    return MSG_UNKNOWN; 
}

void create_message(MyBSMessage *msg, MessageType type, const char *data) {
    msg->type = type;
    if (data) {
        strncpy(msg->data, data, MAX_DATA_SIZE - 1);
        // msg->data[strlen(msg->data) - 1] = '\0';
    } else {
        msg->data[0] = '\0';  // If data is NULL, set it to an empty string
    }
}

// Ensures that the server can understand the message.
void serialize_message(const MyBSMessage *msg, char *buffer) {
    if (strlen(msg->data) > 0) {
        snprintf(buffer, MAX_MESSAGE_SIZE, "%s|%s\n", message_type_to_str(msg->type), msg->data);
    } else {
        snprintf(buffer, MAX_MESSAGE_SIZE, "%s\n", message_type_to_str(msg->type));
    }
}

// Split the message into its components and store them in the MyBSMessage struct
// The message format is "type|data" (e.g., "LOGIN|username")
int parse_message(const char *buffer, MyBSMessage *msg) {
    char type_str[50], data[MAX_DATA_SIZE];

    if (sscanf(buffer, "%49[^|]|%199[^|]", type_str, data) != 2) {
        return -1;
    }    

    msg->type = get_message_type(type_str);

    if (msg->type != MSG_UNKNOWN && strlen(data) > 0) {
        strncpy(msg->data, data, MAX_DATA_SIZE - 1);
        msg->data[MAX_DATA_SIZE - 1] = '\0';
    } else {
        msg->data[0] = '\0';
    }

    return 0;
}