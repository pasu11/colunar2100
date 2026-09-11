# Makefile for colunar (C99)

CC ?= gcc
PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin

TARGET := colunar
SRC := colunar.c
OBJ := $(SRC:.c=.o)

CFLAGS ?= -O2
CFLAGS += -std=c99 -Wall -Wextra -Wshadow -Wconversion -pedantic
LDFLAGS ?=

.PHONY: all clean install uninstall run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

%.o: %.c tables.h
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

install: $(TARGET)
	sudo install -d "$(BINDIR)"
	sudo install -m 0755 "$(TARGET)" "$(BINDIR)/$(TARGET)"

uninstall:
	rm -f "$(BINDIR)/$(TARGET)"

clean:
	rm -f $(OBJ) $(TARGET)
