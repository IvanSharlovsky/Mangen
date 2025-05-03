CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g
SRCDIR = src
OBJDIR = obj
BINDIR = bin
OBJS = $(OBJDIR)/main.o $(OBJDIR)/walker.o $(OBJDIR)/hash.o $(OBJDIR)/verify.o
TARGET = $(BINDIR)/mangen
GIT_HASH := $(shell git rev-parse --short HEAD)

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/main.o: $(SRCDIR)/main.c $(SRCDIR)/walker.h $(SRCDIR)/hash.h $(SRCDIR)/verify.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -DGIT_COMMIT_HASH=\"$(GIT_HASH)\" -c $< -o $@

$(OBJDIR)/walker.o: $(SRCDIR)/walker.c $(SRCDIR)/walker.h $(SRCDIR)/hash.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/hash.o: $(SRCDIR)/hash.c $(SRCDIR)/hash.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/verify.o: $(SRCDIR)/verify.c $(SRCDIR)/verify.h $(SRCDIR)/hash.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)

test:
	chmod +x test.sh
# Pass binary path to test script via BIN variable (default used in test.sh if unset)
	BIN=$(TARGET) ./test.sh

version:
	@echo Commit hash: $(GIT_HASH)


