#!/bin/bash

# Script to setup and test the XOR script locally with virtual environment
# This mimics what the GitHub Actions workflow does

set -e  # Exit on any error

echo "🚀 Setting up Python virtual environment for XOR script testing..."

# Check if Python is available
if ! command -v python3 &> /dev/null; then
    echo "❌ Python 3 is not installed. Please install Python 3 first."
    exit 1
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
if [ -f "requirements.txt" ]; then
    pip install -r requirements.txt
    echo "✅ Dependencies installed from requirements.txt"
else
    echo "⚠️ No requirements.txt found, installing numpy manually..."
    pip install numpy
fi

# Run the XOR script
echo "🧪 Running XOR script..."
python xor.py

echo "✅ Test completed successfully!"
echo ""
echo "To use this environment again:"
echo "  source venv/bin/activate"
echo "  python xor.py"
echo ""
echo "To deactivate the environment:"
echo "  deactivate"
