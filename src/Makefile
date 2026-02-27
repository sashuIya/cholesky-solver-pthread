# Compiler and flags
CC = gcc
CFLAGS = -O3 -Wall -Wextra
LDFLAGS = 
LDLIBS = -lm -lpthread -lrt

# Project structure
BUILD_DIR = ../build
EXECUTABLE = cholesky_solver

# Source and object files
SOURCES = main.c array_op.c timer.c array_io.c cholesky_threaded.c
OBJS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)

# Default target
all: $(BUILD_DIR) $(BUILD_DIR)/$(EXECUTABLE)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link the executable
$(BUILD_DIR)/$(EXECUTABLE): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

# Compile source files
$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Format code using project settings
format:
	clang-format -i *.c *.h

.PHONY: all clean format
