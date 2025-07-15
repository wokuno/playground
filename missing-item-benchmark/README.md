# Missing Item Benchmark

This project benchmarks different methods for finding a missing element in a list using various approaches including XOR operations, loops, sets, and NumPy. It includes **Python**, **C**, and **Go** implementations for performance comparison.

## Project Structure

```
missing-item-benchmark/
├── python/                    # Python implementation
│   ├── missing-item.py        # Main Python benchmark script
│   └── requirements.txt       # Python dependencies
├── c/                         # C implementation  
│   ├── missing-item.c         # Main C benchmark program
│   └── Makefile              # Build configuration
├── go/                        # Go implementation
│   ├── missing-item.go        # Main Go benchmark program
│   ├── go.mod                # Go module file
│   └── Makefile              # Build configuration
├── run.sh         # Setup script for all versions
└── README.md                 # This file
```

## Implementations

### Python Version (`python/missing-item.py`)
- 8 different algorithms including NumPy optimizations
- Comprehensive benchmarking across different list sizes
- Virtual environment support

### C Version (`c/missing-item.c`)
- 4 core algorithms: XOR (original & optimized), sum difference, and linear search
- High-performance native implementation
- Makefile for easy building and testing

### Go Version (`go/missing-item.go`)
- 5 algorithms: XOR (original & optimized), sum difference, linear search, and set-based
- Native Go performance with garbage collection
- Makefile for easy building and testing

## Testing

### GitHub Actions
The project includes a GitHub Actions workflow that automatically tests Python, C, and Go versions:
- Python: Tests across multiple versions (3.8-3.12) with virtual environments
- C: Compiles and tests with GCC on Ubuntu
- Go: Compiles and tests with the latest Go version

### Local Testing

#### Option 1: Use the setup script (tests all versions)
```bash
./run.sh
```

#### Option 2: Manual setup

**Python version:**
```bash
# Create virtual environment
python3 -m venv venv

# Activate virtual environment
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install dependencies
pip install -r python/requirements.txt

# Run the benchmark
python python/missing-item.py

# Deactivate when done
deactivate
```

**C version:**
```bash
# Build and run
cd c
make run

# Or step by step
make           # Build
./missing-item # Run

# Other useful targets
make time      # Run with timing
make debug     # Build debug version
make clean     # Clean build files
```

**Go version:**
```bash
# Build and run
cd go
make run

# Or step by step
make           # Build
./missing-item # Run

# Other useful targets
make time      # Run with timing
make release   # Build optimized version
make clean     # Clean build files
make check     # Format and vet code
```

## Requirements

### Python Version
- Python 3.8+
- NumPy (optional, but recommended for full functionality)

### C Version  
- GCC compiler
- Make utility
- No external dependencies

### Go Version
- Go 1.21+
- Make utility
- No external dependencies

## Algorithms Implemented

### Python Version
1. XOR method (original)
2. XOR method (optimized)
3. XOR method (using functools.reduce)
4. XOR method (NumPy vectorized)
5. XOR method (NumPy combined arrays)
6. Loop method with Counter
7. Set difference method (using Counter)
8. Sum difference method

### C Version
1. XOR method (original)
2. XOR method (optimized)
3. Sum difference method
4. Linear search method

### Go Version
1. XOR method (original)
2. XOR method (optimized)
3. Sum difference method
4. Linear search method
5. Set-based method

## Performance Notes

The C version typically provides significant performance improvements over Python for large datasets, while Go offers a good balance between performance and ease of development. The Python version offers more algorithmic variety and NumPy optimizations for scientific computing workflows.

All implementations will automatically skip unavailable methods (e.g., NumPy methods when NumPy isn't installed).