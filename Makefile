# Makefile для сборки утилиты mangen

CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = mangen
GIT_HASH := $(shell git rev-parse --short HEAD)

all: $(TARGET)

$(TARGET): mangen.c
	$(CC) $(CFLAGS) -DGIT_COMMIT_HASH=\"$(GIT_HASH)\" -o $(TARGET) mangen.c

clean:
	rm -f $(TARGET)

version:
	@echo "Current git commit: $(GIT_HASH)"
