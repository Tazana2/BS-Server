#ifndef BS_PROTOCOL_H
#define BS_PROTOCOL_H

#define MAX_MESSAGE_SIZE 256
#define MAX_DATA_SIZE 200

typedef enum {
    // Session
    MSG_LOGIN,
    MSG_LOGOUT,
    MSG_USER_LIST,
    // Invitation
    MSG_INVITE_SEND,
    MSG_INVITE_RECEIVE,
    MSG_INVITE_REPLY,
    MSG_INVITE_RESULT,
    // Game
    MSG_GAME_START,
    MSG_ATTACK,
    MSG_ATTACK_RESULT,
    MSG_ATTACK_INCOMING,
    MSG_TURN,
    MSG_GAME_OVER,
    // Status and error messages
    MSG_ERROR,
    MSG_OK,
    MSG_ACK,
    MSG_UNKNOWN
} MessageType;

typedef struct {
    MessageType header;
    char data[MAX_DATA_SIZE];
} BSMessage;

MessageType get_message_type(const char *header);
const char *message_type_to_str(MessageType type);
int parse_message(BSMessage *msg, const char *buffer);
void create_message(BSMessage *msg, MessageType type, const char *data);
void serialize_message(const BSMessage *msg, char *buffer);

#endif