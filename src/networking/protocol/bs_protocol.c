#include "../../utils/headers/bs_protocol.h"

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
        case MSG_PLACE_SHIPS:         return "PLACE_SHIPS";
        case MSG_ATTACK:              return "ATTACK";
        case MSG_ATTACK_RESULT:       return "ATTACK_RESULT";
        case MSG_TURN:                return "TURN";
        case MSG_SURRENDER:           return "SURRENDER";
        case MSG_GAME_OVER:           return "GAME_OVER";
        // Status and error messages
        case MSG_OK:                  return "OK";
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
    if (strcmp(header, "PLACE_SHIPS") == 0)         return MSG_PLACE_SHIPS;
    if (strcmp(header, "ATTACK") == 0)              return MSG_ATTACK;
    if (strcmp(header, "ATTACK_RESULT") == 0)       return MSG_ATTACK_RESULT;
    if (strcmp(header, "TURN") == 0)                return MSG_TURN;
    if (strcmp(header, "SURRENDER") == 0)           return MSG_SURRENDER;
    if (strcmp(header, "GAME_OVER") == 0)           return MSG_GAME_OVER;
    // Status and error messages
    if (strcmp(header, "OK") == 0)                  return MSG_OK;
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
    const char *EOF_MARKER = "<EOF>";
    size_t eof_len = strlen(EOF_MARKER);

    if (sscanf(buffer, "%49[^|]|%199[^|]", header, data) != 2) {
        return -1;
    }
    data[strcspn(data, "\n")] = 0; // Remove newline character if present
    msg->header = get_message_type(header);
    
    size_t data_len = strlen(data);
    if (msg->header != MSG_UNKNOWN && data_len >= eof_len &&
        strcmp(data + data_len - eof_len, EOF_MARKER) == 0) {
            // Elimina el <EOF> del final
            data[data_len - eof_len] = '\0';
            
            strncpy(msg->data, data, MAX_DATA_SIZE - 1);
    } else {
        return -1;  // Error: no tiene <EOF> o tipo desconocido
    }

    return 0;
}
