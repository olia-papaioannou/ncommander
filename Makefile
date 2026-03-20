# --- Μεταβλητές ---
CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lncurses
TARGET = ncommander
SRC = ncommander.c


all: $(TARGET)


build: $(TARGET)
	@echo "Build successful: $(TARGET) is ready."

$(TARGET): $(SRC)
	@echo "Compiling $(SRC)..."
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)


run: build
	./$(TARGET)


clean:
	@echo "Cleaning up..."
	rm -f $(TARGET)

.PHONY: all build clean run
