# ==============================================================================
# Core Settings - Change here to switch projects!
# ==============================================================================
# Which module is the project currently in? (c / ds / system_programming)
MODULE   ?= system_programming

# Project name
PROJECT  ?= authenticator

# Additional dependencies (.c files in other modules needed for compilation)
DEP_SRCS = string.c memFuncs.c pam_auth.c

# ==============================================================================
# Paths and Environment Setup
# ==============================================================================
SRC_DIR  = ./$(MODULE)/src
INC_DIR  = ./$(MODULE)/include
TEST_DIR = ./$(MODULE)/test
BIN_DIR  = ./$(MODULE)/bin

# Search for Header files across all modules 
INCLUDES = -I$(INC_DIR) -I./c/include -I./system_programming/include -I./ds/include

# VPATH allows Make to find source files scattered across different directories
VPATH = $(SRC_DIR) $(TEST_DIR) ./system_programming/src ./c/src ./ds/src

# ==============================================================================
# Compiler Settings
# ==============================================================================
CC = gcc
CFLAGS = -ansi -pedantic-errors -Wall -Wextra $(INCLUDES)
DEBUG_FLAGS = -g
RELEASE_FLAGS = -DNDEBUG -O3
DYN_LIBS = -lcrypt -lpam

# ==============================================================================
# Files and Targets
# ==============================================================================
TARGET     = $(PROJECT).out
LIB_TARGET = lib$(PROJECT).a

SRCS       = $(PROJECT).c $(DEP_SRCS) $(PROJECT)_test.c
LIB_SRCS   = $(PROJECT).c $(DEP_SRCS)

OBJS       = $(SRCS:.c=.o)
LIB_OBJS   = $(LIB_SRCS:.c=.o)

# ==============================================================================
# Build Rules
# ==============================================================================
# 'all' calls 'help' first to print instructions, then builds 'debug'
all: help debug

help:
	@echo "======================================================================"
	@echo "                     Makefile Build & Run Options                     "
	@echo "======================================================================"
	@echo "  make              - Prints this help and builds the Debug version"
	@echo "  make debug        - Builds the Debug version"
	@echo "  make release      - Builds the Release version (optimized)"
	@echo "  make run          - Builds and runs the Debug version"
	@echo "  make run-release  - Builds and runs the Release version"
	@echo "  make clean        - Removes all compiled objects and binaries"
	@echo ""
	@echo "  [!] Passing Arguments:"
	@echo "      To pass arguments to your program, use the ARGS variable."
	@echo "      Example: make run ARGS=\"arg1 arg2 ...\""
	@echo "======================================================================"
	@echo ""

debug: CFLAGS += $(DEBUG_FLAGS)
debug: OUT_PATH = $(BIN_DIR)/debug
debug: compile

release: CFLAGS += $(RELEASE_FLAGS)
release: OUT_PATH = $(BIN_DIR)/release
release: compile

# Internal target that centralizes compilation and directs outputs to the appropriate bin directory
compile: $(OBJS)
	@mkdir -p $(OUT_PATH)
	$(CC) $(CFLAGS) $(OBJS) -o $(OUT_PATH)/$(TARGET) $(DYN_LIBS)
	ar rcs $(OUT_PATH)/$(LIB_TARGET) $(LIB_OBJS)
	-rm -f $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ==============================================================================
# Execution Rules
# ==============================================================================
# Allow passing arguments via 'make run ARGS="arg1 arg2"'
ARGS ?= 

run: debug
	@echo "\n--- Running $(TARGET) (Debug) with args: $(ARGS) ---"
	@./$(BIN_DIR)/debug/$(TARGET) $(ARGS)

run-release: release
	@echo "\n--- Running $(TARGET) (Release) with args: $(ARGS) ---"
	@./$(BIN_DIR)/release/$(TARGET) $(ARGS)

clean:
	-rm -f *.o
	-rm -f ./c/bin/debug/* ./c/bin/release/*
	-rm -f ./ds/bin/debug/* ./ds/bin/release/*
	-rm -f ./system_programming/bin/debug/* ./system_programming/bin/release/*

.PHONY: all help debug release compile run run-release clean
