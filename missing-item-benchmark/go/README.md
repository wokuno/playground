# Go Missing Item Benchmark

This directory contains the Go implementation of the missing item benchmark.

## Features

- 5 different algorithms for finding the missing element
- Comprehensive benchmarking across different list sizes (2 to 32,768 elements)
- Native Go performance with garbage collection
- Easy build and run with Makefile

## Algorithms Implemented

1. **XOR (original)** - Uses separate XOR operations for each array
2. **XOR (optimized)** - Uses a single result variable for XOR operations
3. **Sum difference** - Calculates the difference between array sums
4. **Linear search** - Uses map-based counting to find the missing element
5. **Set-based** - Uses map lookup to identify the missing element

## Building and Running

### Using Makefile (recommended)
```bash
# Build and run
make run

# Just build
make

# Run with timing information
make time

# Clean build artifacts
make clean

# Format and vet code
make check

# Build optimized release version
make release
```

### Using Go directly
```bash
# Run directly without building binary
go run missing-item.go

# Build manually
go build -o missing-item missing-item.go

# Run the binary
./missing-item
```

## Requirements

- Go 1.21 or later
- Make utility (for using the Makefile)

## Output

The program will test each algorithm across different list sizes and display:
- Average execution time for each algorithm
- The fastest algorithm for each test size
- Verification that all algorithms return the same result