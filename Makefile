.POSIX:
SHELL=/bin/bash
TARGET = server
SOURCE = server.c
CC = gcc
CFLAGS = -g
BUILD_DIR = ./build
SRC_DIR = .
#PAGE ?= $(shell bash -c 'read -p "Page: " page; echo $$page')

all: $(SOURCE)
	$(CC) $(CFLAGS) $(SRC_DIR)/$(SOURCE) -o $(BUILD_DIR)/$(TARGET)
	cd $(BUILD_DIR); trap "echo [CLEANUP]; rm -f $(TARGET); exit 1" SIGINT; \
	./$(TARGET)

.PHONY: clean
clean:
	rm -f $(TARGET)
