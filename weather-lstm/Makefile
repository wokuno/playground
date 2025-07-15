# Weather LSTM Prediction Makefile

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2 -g
LDFLAGS = -lm

SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Exclude main files from common objects
COMMON_SOURCES = $(filter-out $(SRCDIR)/train.c $(SRCDIR)/predict.c, $(SOURCES))
COMMON_OBJECTS = $(COMMON_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Targets
TRAIN_TARGET = $(BINDIR)/train
PREDICT_TARGET = $(BINDIR)/predict
TEST_TARGET = $(BINDIR)/test_lstm

.PHONY: all clean test install run-tests

all: $(TRAIN_TARGET) $(PREDICT_TARGET) $(TEST_TARGET)

# Create directories
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

# Compile object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

# Compile test files
$(OBJDIR)/test_lstm.o: tests/test_lstm.c | $(OBJDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

# Link training program
$(TRAIN_TARGET): $(COMMON_OBJECTS) $(OBJDIR)/train.o | $(BINDIR)
	$(CC) $(COMMON_OBJECTS) $(OBJDIR)/train.o -o $@ $(LDFLAGS)

# Link prediction program
$(PREDICT_TARGET): $(COMMON_OBJECTS) $(OBJDIR)/predict.o | $(BINDIR)
	$(CC) $(COMMON_OBJECTS) $(OBJDIR)/predict.o -o $@ $(LDFLAGS)

# Link test program
$(TEST_TARGET): $(COMMON_OBJECTS) $(OBJDIR)/test_lstm.o | $(BINDIR)
	$(CC) $(COMMON_OBJECTS) $(OBJDIR)/test_lstm.o -o $@ $(LDFLAGS)

# Clean build files
clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Test build (compile without linking)
test:
	$(CC) $(CFLAGS) -I$(INCDIR) -c $(SOURCES)
	rm -f *.o

# Run unit tests
run-tests: $(TEST_TARGET)
	@echo "Running unit tests..."
	./$(TEST_TARGET)

# Install (copy to system bin - optional)
install: all
	@echo "Install not implemented. Binaries are in $(BINDIR)/"

# Help
help:
	@echo "Available targets:"
	@echo "  all        - Build all programs"
	@echo "  clean      - Remove build files"
	@echo "  test       - Test compilation"
	@echo "  run-tests  - Run unit tests"
	@echo "  help       - Show this help"

# Debug information
debug:
	@echo "Sources: $(SOURCES)"
	@echo "Objects: $(OBJECTS)"
	@echo "Common Objects: $(COMMON_OBJECTS)"
	@echo "Targets: $(TRAIN_TARGET) $(PREDICT_TARGET)"
