#ifndef BS_PROTOCOL_H
#define BS_PROTOCOL_H

#define MAX_MESSAGE_SIZE 256
#define MAX_DATA_SIZE 200

typedef enum {
    MSG_LOGIN,
    MSG_USER_LIST,
    MSG_USER_CONNECT,
    MSG_USER_DISCONNECT,
    MSG_INVITE,
    MSG_INVITATION_RECEIVED,
    MSG_INVITATION_ACCEPT,
    MSG_GAME_START,
    MSG_ATTACK,
    MSG_ATTACK_RESULT,
    MSG_ATTACK_INCOMING,
    MSG_TURN_OPPONENT,
    MSG_TURN_YOUR,
    MSG_GAME_OVER,
    MSG_LOGOUT,
    MSG_ERROR,
    MSG_OK,
    MSG_UNKNOWN
} MessageType;

typedef struct {
    MessageType type;
    char data[MAX_DATA_SIZE];
} MyBSMessage;

const char *message_type_to_str(MessageType type);
MessageType get_message_type(const char *type_str);
void create_message(MyBSMessage *msg, MessageType type, const char *data);
void serialize_message(const MyBSMessage *msg, char *buffer);
int parse_message(const char *buffer, MyBSMessage *msg);

#endif