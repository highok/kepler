.POSIX:

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic
LIBS = -lncurses -lm

BIN = kepler
SRC = editor.c main.c
OBJ = $(SRC:.c=.o)

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ) $(LIBS)

main.o: main.c editor.h
	$(CC) $(CFLAGS) -c main.c

editor.o: editor.c editor.h
	$(CC) $(CFLAGS) -c editor.c

clean:
	rm -f $(OBJ) $(BIN)

.PHONY: all clean
