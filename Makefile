export TEMP := ./build
export TMP := ./build

CC = gcc
CFLAGS = -MMD -Wall -Wextra -g -fopenmp
LINKER = -lglfw3dll -lm -lOpenAL32 -lsndfile
LIB_DIR = lib
SRC_DIR = src
OBJ_DIR = build
DEP_DIR = build
ASM_DIR = build
BIN_DIR = bin
NAME = prog
TARGET = $(BIN_DIR)/$(NAME)

LIB_DIRS = $(shell find $(LIB_DIR) -type d -name "*link")
INCLUDE_DIRS = $(shell find $(LIB_DIR) -type d -name "*include")
SRCS = $(shell find $(SRC_DIR) $(LIB_DIR) -name "*.c")
HDRS = $(shell find $(SRC_DIR) $(LIB_DIR) -name "*.h")
LIBS = $(patsubst %, -L./%, $(LIB_DIRS)) $(LINKER)
OBJS = $(SRCS:%.c=$(OBJ_DIR)/%.o)
ASMS = $(SRCS:%.c=$(ASM_DIR)/%.s)
DEPS = $(SRCS:%.c=$(DEP_DIR)/%.d)
DEPSH = $(shell find build -name "*.d" -exec grep -Eoh "[^ ]+.h" {} +)
INCLUDES = $(patsubst %, -I./%, $(INCLUDE_DIRS))

all: $(OBJS)
	@mkdir -p $(BIN_DIR)
	@$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) $(LIBS) -o $(TARGET)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(shell dirname $@)
	@$(CC) $(CFLAGS) $(LIBS) $(INCLUDES) -c -o $@ $<

asm: $(ASMS)

asm-clean:
	@rm -f $(shell find build -name "*.s")

$(ASM_DIR)/%.s: %.c
	@mkdir -p $(shell dirname $@)
	@$(CC) $(LIBS) $(INCLUDES) -O3 -S -o $@ $<

$(DEPSH):

-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR) $(DEP_DIR) $(TARGET)
.PHONY: clean