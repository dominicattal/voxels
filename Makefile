export TEMP := ./build
export TMP := ./build

CC = gcc
CFLAGS = -Wall -Wextra -O3 -g
LINKER = -lglfw3dll -lm -lOpenAL32 -lsndfile
LIB_DIR = lib
SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin
TARGET = $(BIN_DIR)/gui

LIB_DIRS = $(shell find $(LIB_DIR) -type d -name "*link")
INCLUDE_DIRS = $(shell find $(LIB_DIR) -type d -name "*include")
SRCS = $(shell find $(SRC_DIR) $(LIB_DIR) -name "*.c")
LIBS = $(patsubst %, -L./%, $(LIB_DIRS)) $(LINKER)
OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRCS))
DEPS = $(patsubst %.c, %.d, $(SRCS))
INCLUDES = $(patsubst %, -I./%, $(INCLUDE_DIRS))

all: $(OBJS)
	@$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) $(LIBS) -o $(TARGET)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(shell dirname $@)
	@$(CC) $(CFLAGS) $(LIBS) $(INCLUDES) $^ -c -o $@

-include $(DEPS)

clean:
	rm -r $(OBJ_DIR) $(TARGET)

.PHONY: clean