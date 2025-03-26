#ifndef MYBS_PROTOCOL_H
#define MYBS_PROTOCOL_H

#include <stdint.h>

#define MAX_MESSAGE_SIZE 256
#define MAX_DATA_SIZE 200

typedef enum {
    MSG_LOGIN,
    MSG_LOGIN_OK,
    MSG_USER_LIST,
    MSG_USERS,
    MSG_USER_CONNECT,
    MSG_USER_DISCONNECT,
    MSG_INVITE,
    MSG_INVITATION_RECEIVED,
    MSG_INVITATION_ACCEPT,
    MSG_GAME_START,
    MSG_ATTACK,
    MSG_ATTACK_RESULT,
    MSG_TURN_OPPONENT,
    MSG_TURN_YOUR,
    MSG_GAME_OVER,
    MSG_LOGOUT,
    MSG_UNKNOWN
} MessageType;

typedef struct {
    MessageType type;
    char data[MAX_DATA_SIZE];
    uint8_t checksum;
} MyBSMessage;

const char *message_type_to_str(MessageType type);
MessageType get_message_type(const char *type_str);
uint8_t calculate_checksum(const MyBSMessage *msg);
int verify_checksum(const MyBSMessage *msg);
void create_message(MyBSMessage *msg, MessageType type, const char *data);
void serialize_message(const MyBSMessage *msg, char *buffer);
int parse_message(const char *buffer, MyBSMessage *msg);

#endif