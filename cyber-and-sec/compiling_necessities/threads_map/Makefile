# ==============================================================================
# Core Settings - Change here to switch projects!
# ==============================================================================
# Which module is the project currently in? (c / ds / system_programming)
MODULE   ?= system_programming

# Project name (the name of your .c file without the extension)
PROJECT  ?= threads_map

# 1 = Standalone file with main() inside it (No test file, no library)
STANDALONE ?= 1

# 1 = Build test & library, 0 = Build static library only (Ignored if STANDALONE=1)
HAS_TEST ?= 0

# Additional dependencies (.c files in other modules needed for compilation)
DEP_SRCS = string.c 

# ==============================================================================
# Paths and Environment Setup
# ==============================================================================
SRC_DIR  = ./$(MODULE)/src
INC_DIR  = ./$(MODULE)/include
TEST_DIR = ./$(MODULE)/test
BIN_DIR  = ./$(MODULE)/bin

INCLUDES = -I$(INC_DIR) -I./c/include -I./system_programming/include -I./ds/include

VPATH = $(SRC_DIR) $(TEST_DIR) ./system_programming/src ./c/src ./ds/src

# ==============================================================================
# Compiler Settings
# ==============================================================================
CC = gcc
# Note: Removed -pedantic-errors for standalone if you want less strictness, but left it here.
CFLAGS = -ansi -pedantic-errors -Wall -Wextra $(INCLUDES) -pthread
DEBUG_FLAGS = -g
RELEASE_FLAGS = -DNDEBUG -O3
DYN_LIBS = -pthread

# ==============================================================================
# Files and Targets
# ==============================================================================
TARGET     = $(PROJECT).out
LIB_TARGET = lib$(PROJECT).a

# Determine exactly what files to compile based on the chosen mode
ifeq ($(STANDALONE), 1)
    SRCS = $(PROJECT).c $(DEP_SRCS)
    LIB_SRCS = 
else ifeq ($(HAS_TEST), 1)
    SRCS = $(PROJECT).c $(DEP_SRCS) $(PROJECT)_test.c
    LIB_SRCS = $(PROJECT).c $(DEP_SRCS)
else
    SRCS = $(PROJECT).c $(DEP_SRCS)
    LIB_SRCS = $(PROJECT).c $(DEP_SRCS)
endif

OBJS       = $(SRCS:.c=.o)
LIB_OBJS   = $(LIB_SRCS:.c=.o)

# ==============================================================================
# Build Rules
# ==============================================================================
all: help debug

help:
	@echo "======================================================================"
	@echo "                     Makefile Build & Run Options                     "
	@echo "======================================================================"
	@echo "  make              - Prints this help and builds the Debug version"
	@echo "  make run          - Builds and runs the Debug version"
	@echo "  make clean        - Removes all compiled objects and binaries"
	@echo "======================================================================"

debug: CFLAGS += $(DEBUG_FLAGS)
debug: OUT_PATH = $(BIN_DIR)/debug
debug: compile

release: CFLAGS += $(RELEASE_FLAGS)
release: OUT_PATH = $(BIN_DIR)/release
release: compile

# Internal target that centralizes compilation
compile: $(OBJS)
	@mkdir -p $(OUT_PATH)
ifeq ($(STANDALONE), 1)
	$(CC) $(CFLAGS) $(OBJS) -o $(OUT_PATH)/$(TARGET) $(DYN_LIBS)
else ifeq ($(HAS_TEST), 1)
	$(CC) $(CFLAGS) $(OBJS) -o $(OUT_PATH)/$(TARGET) $(DYN_LIBS)
	ar rcs $(OUT_PATH)/$(LIB_TARGET) $(LIB_OBJS)
else
	ar rcs $(OUT_PATH)/$(LIB_TARGET) $(LIB_OBJS)
endif
	-rm -f $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ==============================================================================
# Execution Rules
# ==============================================================================
ARGS ?= 

run: debug
ifeq ($(STANDALONE), 1)
	@echo "\n--- Running Standalone $(TARGET) (Debug) ---"
	@./$(BIN_DIR)/debug/$(TARGET) $(ARGS)
else ifeq ($(HAS_TEST), 1)
	@echo "\n--- Running $(TARGET) (Debug) ---"
	@./$(BIN_DIR)/debug/$(TARGET) $(ARGS)
else
	@echo "\n--- [!] Project is set to library-only (HAS_TEST=0). Nothing to run. ---"
endif

run-release: release
ifeq ($(STANDALONE), 1)
	@echo "\n--- Running Standalone $(TARGET) (Release) ---"
	@./$(BIN_DIR)/release/$(TARGET) $(ARGS)
else ifeq ($(HAS_TEST), 1)
	@echo "\n--- Running $(TARGET) (Release) ---"
	@./$(BIN_DIR)/release/$(TARGET) $(ARGS)
else
	@echo "\n--- [!] Project is set to library-only (HAS_TEST=0). Nothing to run. ---"
endif

clean:
	-rm -f *.o
	-rm -f ./c/bin/debug/* ./c/bin/release/*
	-rm -f ./ds/bin/debug/* ./ds/bin/release/*
	-rm -f ./system_programming/bin/debug/* ./system_programming/bin/release/*

.PHONY: all help debug release compile run run-release clean
