#!/bin/bash
# Simple test script to validate the weather LSTM workflow steps locally
# This helps debug issues before running the full CI/CD pipeline

set -e

echo "🧪 Weather LSTM Workflow Test"
echo "=============================="

# Test 1: Check if required directories exist
echo "📁 Checking directory structure..."
cd "$(dirname "$0")/.."
pwd

required_dirs=("data" "models" "bin" "scripts" "src")
for dir in "${required_dirs[@]}"; do
    if [ -d "$dir" ]; then
        echo "✅ $dir/ exists"
    else
        echo "⚠️  $dir/ missing - will be created"
        mkdir -p "$dir"
    fi
done

# Test 2: Check if Python script is executable and works
echo ""
echo "🐍 Testing NOAA fetch script..."
if [ -f "scripts/fetch_noaa_weather.py" ]; then
    echo "✅ fetch_noaa_weather.py exists"
    chmod +x scripts/fetch_noaa_weather.py
    
    # Test station info (quick API test)
    if python3 scripts/fetch_noaa_weather.py --station KMSP --info; then
        echo "✅ NOAA API connectivity test passed"
    else
        echo "❌ NOAA API connectivity test failed"
    fi
else
    echo "❌ fetch_noaa_weather.py not found"
fi

# Test 3: Check if Makefile exists and build system works
echo ""
echo "🔨 Testing build system..."
if [ -f "Makefile" ]; then
    echo "✅ Makefile exists"
    
    # Test clean build
    make clean 2>/dev/null || echo "Clean step skipped (no previous build)"
    
    if make all; then
        echo "✅ Build successful"
        
        # Check if binaries were created
        if [ -f "bin/train" ] && [ -f "bin/predict" ]; then
            echo "✅ Both train and predict binaries created"
        else
            echo "⚠️  Some binaries missing:"
            ls -la bin/ 2>/dev/null || echo "  bin/ directory empty"
        fi
    else
        echo "❌ Build failed"
    fi
else
    echo "❌ Makefile not found"
fi

# Test 4: Test data fetching (limited test)
echo ""
echo "📡 Testing data fetch (limited)..."
if command -v python3 >/dev/null && [ -f "scripts/fetch_noaa_weather.py" ]; then
    # Try to fetch just 1 day of data as a test
    if python3 scripts/fetch_noaa_weather.py --station KMSP --days 1 --output data/test_data.csv; then
        echo "✅ Data fetch test successful"
        
        if [ -f "data/test_data.csv" ]; then
            LINES=$(wc -l < data/test_data.csv)
            echo "📊 Test data file created with $LINES lines"
            
            # Show first few lines
            echo "Sample data:"
            head -n 3 data/test_data.csv
        fi
    else
        echo "⚠️  Data fetch test failed (API may be temporarily unavailable)"
    fi
fi

# Test 5: Check artifact preparation
echo ""
echo "📦 Testing artifact preparation..."
mkdir -p artifacts/models artifacts/data artifacts/bin

# Simulate the artifact preparation logic from the workflow
FILES_FOUND=0

# Check for model files (may not exist in test)
for model_file in models/weather_model.bin models/perf_test_model.bin; do
  if [ -f "$model_file" ]; then
    cp "$model_file" "artifacts/$model_file"
    echo "✅ Added: $model_file"
    FILES_FOUND=$((FILES_FOUND + 1))
  else
    echo "⚠️  Missing: $model_file (expected in test)"
  fi
done

# Check for data files
for data_file in data/test_data.csv; do
  if [ -f "$data_file" ]; then
    cp "$data_file" "artifacts/data/"
    echo "✅ Added: $data_file"
    FILES_FOUND=$((FILES_FOUND + 1))
  else
    echo "⚠️  Missing: $data_file"
  fi
done

# Check for binaries
if [ -d "bin" ] && [ "$(ls -A bin 2>/dev/null)" ]; then
  cp bin/* artifacts/bin/ 2>/dev/null || true
  BIN_COUNT=$(ls bin/ 2>/dev/null | wc -l)
  echo "✅ Added: bin/ ($BIN_COUNT executables)"
  FILES_FOUND=$((FILES_FOUND + 1))
fi

echo "📊 Artifact test summary: $FILES_FOUND file groups found"

# Test 6: Cleanup
echo ""
echo "🧹 Cleaning up test files..."
rm -f data/test_data.csv
rm -rf artifacts/
echo "✅ Test cleanup completed"

echo ""
echo "🎉 Workflow test completed!"
echo "This provides a basic validation of the workflow components."
echo "For full testing, run the actual GitHub Actions workflow."
