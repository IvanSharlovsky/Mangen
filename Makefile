# Makefile для сборки утилиты mangen

CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = mangen

all: $(TARGET)

$(TARGET): mangen.c
	$(CC) $(CFLAGS) -o $(TARGET) mangen.c

clean:
	rm -f $(TARGET)
