#!/bin/bash

# Script to setup and test both Python and C versions of the Missing Item benchmark
# This mimics what the GitHub Actions workflow does

set -e  # Exit on any error

echo "🚀 Setting up Missing Item benchmark testing environment..."

# Function to test Python version
test_python() {
    echo ""
    echo "=== Python Version ==="
    
    # Check if Python is available
    if ! command -v python3 &> /dev/null; then
        echo "❌ Python 3 is not installed. Please install Python 3 first."
        return 1
    fi

    # Create virtual environment
    echo "📦 Creating virtual environment..."
    python3 -m venv venv

    # Activate virtual environment
    echo "🔧 Activating virtual environment..."
    source venv/bin/activate

    # Upgrade pip
    echo "⬆️ Upgrading pip..."
    python -m pip install --upgrade pip

    # Install dependencies
    echo "📥 Installing dependencies..."
    if [ -f "python/requirements.txt" ]; then
        pip install -r python/requirements.txt
        echo "✅ Dependencies installed from python/requirements.txt"
    else
        echo "⚠️ No python/requirements.txt found, installing numpy manually..."
        pip install numpy
    fi

    # Run the missing item script
    echo "🧪 Running Python Missing Item benchmark..."
    cd python
    python missing-item.py
    cd ..
    
    deactivate
    echo "✅ Python test completed successfully!"
}

# Function to test C version
test_c() {
    echo ""
    echo "=== C Version ==="
    
    # Check if gcc is available
    if ! command -v gcc &> /dev/null; then
        echo "❌ GCC is not installed. Please install GCC first."
        echo "   On macOS: xcode-select --install"
        echo "   On Ubuntu/Debian: sudo apt-get install gcc"
        echo "   On CentOS/RHEL: sudo yum install gcc"
        return 1
    fi
    
    # Check if make is available
    if ! command -v make &> /dev/null; then
        echo "❌ Make is not installed. Please install Make first."
        return 1
    fi
    
    echo "🔧 Building C benchmark..."
    cd c
    make clean
    make
    
    echo "🧪 Running C Missing Item benchmark..."
    make test
    
    echo "🔍 Testing debug build..."
    make debug
    echo "Running debug version..."
    ./missing-item_debug
    cd ..
    
    echo "✅ C test completed successfully!"
}


# Check for help flag
if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    echo ""
    echo "Usage instructions:"
    echo ""
    echo "Python version:"
    echo "  source venv/bin/activate"
    echo "  python python/missing-item.py"
    echo "  deactivate"
    echo ""
    echo "C version:"
    echo "  cd c"
    echo "  make           # Build"
    echo "  make run       # Build and run"
    echo "  make time      # Build and run with timing"
    echo "  make clean     # Clean build files"
    exit 0
fi

# Run both tests
test_python
test_c

echo ""
echo "🎉 All tests completed successfully!"

