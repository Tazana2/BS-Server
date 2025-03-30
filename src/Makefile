CC = gcc
CFLAGS = -Wall -Wextra -Igame_models -Inetworking/protocol -lpthread
SRC = networking/server.c networking/protocol/BSProtocol.c game_models/player.c game_models/player_table.c game_models/invitation_table.c 
OBJ = $(SRC:.c=.o)
EXEC = server

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ)

clean:
	rm -f $(OBJ) $(EXEC)
