# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -g

# Directorios
GAME_LOGIC_DIR = game_logic
GAME_MODELS_DIR = game_models
NETWORKING_DIR = networking
PROTOCOL_DIR = $(NETWORKING_DIR)/protocol

# Archivos fuente
SRCS = $(GAME_LOGIC_DIR)/board.c \
       $(GAME_LOGIC_DIR)/game_session.c \
       $(GAME_MODELS_DIR)/player_table.c \
       $(GAME_MODELS_DIR)/invitation_table.c \
       $(GAME_MODELS_DIR)/player.c \
       $(PROTOCOL_DIR)/BSProtocol.c \
       $(NETWORKING_DIR)/server.c 

# Archivos objeto
OBJS = $(SRCS:.c=.o)

# Ejecutable
TARGET = server

# Regla principal
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Regla para compilar archivos .c a .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpieza
clean:
	rm -f $(OBJS) $(TARGET)

# Limpieza profunda
mrproper: clean
	rm -f *~
