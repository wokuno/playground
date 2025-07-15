BINARY_NAME=missing-item
MAIN_PACKAGE=.

.PHONY: all build run clean test benchmark time help

# Default target
all: build

# Build the binary
build:
	@echo "Building Go binary..."
	@go build -o $(BINARY_NAME) $(MAIN_PACKAGE)

# Run the program directly
run: build
	@echo "Running missing item benchmark..."
	@./$(BINARY_NAME)

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -f $(BINARY_NAME)
	@go clean

# Run tests (if any test files exist)
test:
	@echo "Running tests..."
	@go test ./...

# Run benchmark with time command for additional profiling
time: build
	@echo "Running with time profiling..."
	@time ./$(BINARY_NAME)

# Run with go run (no binary creation)
direct:
	@echo "Running directly with go run..."
	@go run $(MAIN_PACKAGE)

# Build optimized release version
release:
	@echo "Building optimized release binary..."
	@go build -ldflags="-s -w" -o $(BINARY_NAME) $(MAIN_PACKAGE)

# Format code
fmt:
	@echo "Formatting Go code..."
	@go fmt ./...

# Vet code for potential issues
vet:
	@echo "Vetting Go code..."
	@go vet ./...

# Run all code quality checks
check: fmt vet
	@echo "All code quality checks passed!"

# Help target
help:
	@echo "Available targets:"
	@echo "  all      - Build the binary (default)"
	@echo "  build    - Build the binary"
	@echo "  run      - Build and run the program"
	@echo "  direct   - Run directly with 'go run'"
	@echo "  clean    - Clean build artifacts"
	@echo "  test     - Run tests"
	@echo "  time     - Run with timing information"
	@echo "  release  - Build optimized release binary"
	@echo "  fmt      - Format Go code"
	@echo "  vet      - Vet Go code for issues"
	@echo "  check    - Run fmt and vet"
	@echo "  help     - Show this help message"
