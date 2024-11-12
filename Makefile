export TEMP := ./build
export TMP := ./build

CC = gcc
CFLAGS =
LIB_DIR = lib
SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin
TARGET = $(BIN_DIR)/gui

LIB_DIRS = $(shell find $(LIB_DIR) -type d -name "*link")
LIBS = $(patsubst %, -L./%, $(LIB_DIRS)) -lglfw3dll -lm
INCLUDE_DIRS = $(shell find $(LIB_DIR) -type d -name "*include")
INCLUDES = $(patsubst %, -I./%, $(INCLUDE_DIRS))
SRCS = $(shell find $(SRC_DIR) $(LIB_DIR) -name "*.c")
OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRCS))

all: $(OBJS)
	@$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) $(LIBS) -o $(TARGET)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(shell dirname $@)
	@$(CC) $(CFLAGS) $(LIBS) $(INCLUDES) -c $^ -o $@

clean:
	rm -r $(OBJ_DIR) a.exe

.PHONY: clean