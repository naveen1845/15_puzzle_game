CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = 15_puzzle_game
SRC = main.c backend.c frontend.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET) saved_game.dat

.PHONY: all clean
