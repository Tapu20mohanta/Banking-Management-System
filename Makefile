# Compiler and compilation flags
CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic

# Source files and headers
SRCS = main.c banking.c file_handler.c
HEADERS = banking.h file_handler.h

# Executable name
TARGET_UNIX = banking
TARGET_WIN = banking.exe

ifeq ($(OS),Windows_NT)
    TARGET = $(TARGET_WIN)
    RM = del /Q /F
else
    TARGET = $(TARGET_UNIX)
    RM = rm -f
endif

# Default target: compile binary
all: $(TARGET)

$(TARGET): $(SRCS) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Run the program
run: $(TARGET)
ifeq ($(OS),Windows_NT)
	.\$(TARGET)
else
	./$(TARGET)
endif

# Clean generated binaries and build artifacts
clean:
ifeq ($(OS),Windows_NT)
	-if exist $(TARGET_WIN) $(RM) $(TARGET_WIN)
else
	$(RM) $(TARGET_UNIX) *.o
endif

.PHONY: all run clean
