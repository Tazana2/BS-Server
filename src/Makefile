CC = gcc
CFLAGS = -Wall -Wextra
PTL_DIR = protocol
NTW_DIR = networking
OBJ_DIR = build

SRCS = $(NTW_DIR)/Server.c $(NTW_DIR)/$(PTL_DIR)/BSProtocol.c
OBJS = $(SRCS:.c=.o)

TARGETS = server 

all: $(TARGETS)

server: networking/Server.o networking/protocol/BSProtocol.o
	$(CC) $(CFLAGS) -o server networking/Server.o networking/protocol/BSProtocol.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGETS)