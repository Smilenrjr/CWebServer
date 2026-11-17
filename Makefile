.POSIX:
SHELL=/bin/bash
TARGET = server
SOURCE = server.c
BUILD_DIR = ./build
SRC_DIR = ./src
INCLUDE_PATH = ./include
CC = gcc
CFLAGS = -g -fsanitize=address -fno-omit-frame-pointer
INCFLAGS = $(addprefix -I,$(INCLUDE_PATH))

C_FILES := $(wildcard src/*.c)
HEADER_FILES := $(wildcard include/*.h)
OBJ_FILES := $(patsubst %.c,%.o,$(C_FILES))

#PAGE ?= $(shell bash -c 'read -p "Page: " page; echo $$page')


all: $(TARGET)
	cd $(BUILD_DIR); trap "echo [CLEANUP]; cd ..; make clean; exit 1" SIGINT; \
	./$(TARGET)

%.o: %.c $(HEADER_FILES)
	$(CC) $(CFLAGS) $(INCFLAGS) -c $< -o $@

$(TARGET): $(OBJ_FILES) 
	$(CC) $(CFLAGS) $(INCFLAGS) -o $(BUILD_DIR)/$@ $(OBJ_FILES)

clean:
	rm $(OBJ_FILES)
	rm $(BUILD_DIR)/$(TARGET)
