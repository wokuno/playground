# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O3 -std=c99
DEBUG_CFLAGS = -Wall -Wextra -g -std=c99 -DDEBUG

# Target executable
TARGET = missing-item
SOURCE = missing-item.c

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

# Debug build
debug: $(SOURCE)
	$(CC) $(DEBUG_CFLAGS) -o $(TARGET)_debug $(SOURCE)

# Run the benchmark
run: $(TARGET)
	./$(TARGET)

# Run with time measurement
time: $(TARGET)
	time ./$(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET) $(TARGET)_debug

# Install dependencies (for CI)
install-deps:
	@echo "No dependencies needed for C version"

# Test target (alias for run)
test: run

# Help target
help:
	@echo "Available targets:"
	@echo "  all       - Build the benchmark (default)"
	@echo "  debug     - Build debug version"
	@echo "  run       - Build and run the benchmark"
	@echo "  time      - Build and run with time measurement"
	@echo "  clean     - Remove build artifacts"
	@echo "  test      - Run the benchmark (alias for run)"
	@echo "  help      - Show this help message"

.PHONY: all debug run time clean install-deps test help
