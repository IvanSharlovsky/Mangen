# Makefile — modular build for mangen

CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -g
SRCDIR  = src
OBJDIR  = obj
BINDIR  = bin
OBJS    = $(OBJDIR)/main.o $(OBJDIR)/walker.o $(OBJDIR)/hash.o $(OBJDIR)/verify.o
TARGET  = $(BINDIR)/mangen

# Compute the hash immediately and pass it as a string
GIT_HASH := $(shell git rev-parse --short HEAD)
DEFS     = -DGIT_COMMIT_HASH='"$(GIT_HASH)"'

all: $(TARGET)

# Final linking
$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(DEFS) -o $@ $^

# -------- Object files --------
$(OBJDIR)/main.o: $(SRCDIR)/main.c $(SRCDIR)/walker.h $(SRCDIR)/hash.h $(SRCDIR)/verify.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@

$(OBJDIR)/walker.o: $(SRCDIR)/walker.c $(SRCDIR)/walker.h $(SRCDIR)/hash.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/hash.o: $(SRCDIR)/hash.c $(SRCDIR)/hash.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/verify.o: $(SRCDIR)/verify.c $(SRCDIR)/verify.h $(SRCDIR)/hash.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# -------- Auxiliary targets --------
clean
	rm -rf $(OBJDIR) $(BINDIR)

test: all        # Always rebuild to ensure the hash is up-to-date
	chmod +x test.sh
	BIN=$(TARGET) ./test.sh

version:
	@echo Commit hash: $(GIT_HASH)
