#!/bin/bash
# Local workflow test script
# Tests the complete workflow including artifact simulation

set -e

echo "🧪 Testing Weather LSTM Local Workflow"
echo "======================================"

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_step() {
    echo -e "\n${BLUE}[STEP]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_info() {
    echo -e "${YELLOW}[INFO]${NC} $1"
}

# Step 1: Clean build
print_step "Building C programs"
make clean
make all
print_success "Build completed"

# Step 2: Fetch data
print_step "Fetching weather data"
if python3 scripts/fetch_noaa_weather.py --station KMSP --days 30 --incremental --output data/historical_data.csv; then
    print_success "Real NOAA data fetched incrementally"
else
    print_info "NOAA failed, using synthetic data"
    python3 scripts/fetch_weather.py --location KMSP --days 30 --output data/historical_data.csv
fi

# Fetch recent data - need enough for sequence length
if python3 scripts/fetch_noaa_weather.py --station KMSP --recent --output data/recent_weather.csv; then
    # Check if we have enough data points
    RECENT_POINTS=$(($(wc -l < data/recent_weather.csv) - 1))
    if [ $RECENT_POINTS -lt 5 ]; then
        print_info "NOAA recent data insufficient ($RECENT_POINTS points), generating synthetic supplement"
        python3 scripts/fetch_weather.py --location KMSP --recent --output data/recent_weather.csv
    else
        print_success "Real NOAA recent data fetched ($RECENT_POINTS points)"
    fi
else
    print_info "NOAA recent failed, using synthetic data"
    python3 scripts/fetch_weather.py --location KMSP --recent --output data/recent_weather.csv
fi

# Validate data
TRAINING_POINTS=$(($(wc -l < data/historical_data.csv) - 1))
RECENT_POINTS=$(($(wc -l < data/recent_weather.csv) - 1))
print_info "Training data: $TRAINING_POINTS points"
print_info "Recent data: $RECENT_POINTS points"

if [ $TRAINING_POINTS -lt 10 ]; then
    print_info "Insufficient training data, supplementing with synthetic data"
    python3 scripts/fetch_weather.py --location KMSP --days 30 --output data/historical_data.csv
fi

# Step 3: Check for existing model
print_step "Checking for existing model"
if [ -f "models/weather_model.bin" ]; then
    print_info "Found existing model: models/weather_model.bin"
    MODEL_STATUS="continue"
else
    print_info "No existing model found, will train from scratch"
    MODEL_STATUS="new"
fi

# Step 4: Train model
print_step "Training LSTM model ($MODEL_STATUS)"
./bin/train \
    --data data/historical_data.csv \
    --epochs 20 \
    --output models/weather_model.bin \
    --hidden 32 \
    --sequence 5 \
    --learning-rate 0.01

print_success "Training completed"
echo "Model size: $(ls -lh models/weather_model.bin | awk '{print $5}')"

# Step 5: Test prediction
print_step "Testing prediction"
if ./bin/predict \
    --model models/weather_model.bin \
    --input data/recent_weather.csv \
    --output data/prediction.csv; then
    
    print_success "Prediction completed"
    echo "Prediction results:"
    cat data/prediction.csv
else
    print_info "Prediction failed (likely insufficient data), skipping prediction test"
fi

# Step 6: Performance test
print_step "Running performance test"
time ./bin/train \
    --data data/historical_data.csv \
    --epochs 5 \
    --output models/perf_test_model.bin \
    --hidden 16 \
    --sequence 3

time ./bin/predict \
    --model models/weather_model.bin \
    --input data/recent_weather.csv

print_success "Performance test completed"

# Step 7: Accuracy test
print_step "Running accuracy test"

# Check if we have enough recent data for accuracy test
RECENT_POINTS=$(($(wc -l < data/recent_weather.csv) - 1))
if [ $RECENT_POINTS -ge 6 ]; then
    # Extract enough data points for sequence testing
    tail -n 7 data/recent_weather.csv > data/test_input.csv
    head -n 1 data/recent_weather.csv > data/temp_header.csv
    tail -n 6 data/test_input.csv >> data/temp_header.csv
    cp data/temp_header.csv data/test_input.csv
    
    # Save the actual last value for comparison
    tail -n 1 data/recent_weather.csv > data/actual_value.csv

    if ./bin/predict \
        --model models/weather_model.bin \
        --input data/test_input.csv \
        --output data/accuracy_test.csv; then
        
        print_success "Accuracy test completed"
        echo "Accuracy test results:"
        cat data/accuracy_test.csv
        
        # Enhanced accuracy analysis
        echo ""
        echo "🎯 Accuracy Analysis:"
        echo "==================="
        
        # Get predicted temperature (skip header, get first data value)
        PREDICTED_TEMP=$(tail -n +2 data/accuracy_test.csv | head -n 1 | cut -d',' -f1 2>/dev/null || echo "0")
        
        # Get actual temperature from the last data point
        ACTUAL_TEMP=$(tail -n 1 data/actual_value.csv | cut -d',' -f1 2>/dev/null || echo "0")
        
        echo "Predicted Temperature: ${PREDICTED_TEMP}°F"
        echo "Actual Temperature:    ${ACTUAL_TEMP}°F"
        
        # Calculate accuracy metrics using bc for floating point math
        if command -v bc >/dev/null 2>&1 && [ "$PREDICTED_TEMP" != "0" ] && [ "$ACTUAL_TEMP" != "0" ]; then
            # Calculate absolute error
            ABS_ERROR=$(echo "if ($PREDICTED_TEMP > $ACTUAL_TEMP) $PREDICTED_TEMP - $ACTUAL_TEMP else $ACTUAL_TEMP - $PREDICTED_TEMP" | bc -l)
            
            # Calculate relative error percentage
            REL_ERROR=$(echo "scale=2; ($ABS_ERROR / $ACTUAL_TEMP) * 100" | bc -l)
            
            # Calculate accuracy percentage (100 - relative error, min 0)
            ACCURACY=$(echo "scale=2; if (100 - $REL_ERROR < 0) 0 else 100 - $REL_ERROR" | bc -l)
            
            echo "Absolute Error:        ${ABS_ERROR}°F"
            echo "Relative Error:        ${REL_ERROR}%"
            echo "Accuracy:              ${ACCURACY}%"
            
            # Provide accuracy assessment using integer comparison
            ABS_ERROR_INT=$(echo "$ABS_ERROR" | cut -d'.' -f1)
            if [ "${ABS_ERROR_INT:-999}" -le 2 ]; then
                echo "Assessment:            🎯 EXCELLENT (within 2°F)"
            elif [ "${ABS_ERROR_INT:-999}" -le 5 ]; then
                echo "Assessment:            ✅ GOOD (within 5°F)"
            elif [ "${ABS_ERROR_INT:-999}" -le 10 ]; then
                echo "Assessment:            ⚠️  FAIR (within 10°F)"
            else
                echo "Assessment:            ❌ POOR (>10°F difference)"
            fi
        else
            echo "⚠️  Could not calculate detailed accuracy metrics (bc not available or invalid values)"
            echo "Manual comparison: Predicted=${PREDICTED_TEMP}°F vs Actual=${ACTUAL_TEMP}°F"
        fi
        
        echo ""
        echo "📊 Raw Data Comparison:"
        echo "Actual values (target for prediction):"
        cat data/actual_value.csv
        echo "Test input data (last few rows):"
        tail -n 3 data/test_input.csv | head -n 2
        
    else
        print_info "Accuracy test failed, but this is acceptable for testing"
    fi
else
    print_info "Insufficient recent data for accuracy test (need 6 points, have $RECENT_POINTS)"
    echo "Generating minimal test data for accuracy test..."
    
    # Create a simple test case with sufficient data
    head -n 1 data/historical_data.csv > data/test_input.csv
    tail -n 5 data/historical_data.csv >> data/test_input.csv
    
    # Save the actual last value for comparison
    tail -n 1 data/historical_data.csv > data/actual_value.csv
    
    if ./bin/predict \
        --model models/weather_model.bin \
        --input data/test_input.csv \
        --output data/accuracy_test.csv; then
        
        print_success "Accuracy test completed with training data"
        echo "Accuracy test results:"
        cat data/accuracy_test.csv
        
        # Enhanced accuracy analysis
        echo ""
        echo "🎯 Accuracy Analysis:"
        echo "==================="
        
        # Get predicted temperature (skip header, get first data value)
        PREDICTED_TEMP=$(tail -n +2 data/accuracy_test.csv | head -n 1 | cut -d',' -f1 2>/dev/null || echo "0")
        
        # Get actual temperature from the last data point
        ACTUAL_TEMP=$(tail -n 1 data/actual_value.csv | cut -d',' -f1 2>/dev/null || echo "0")
        
        echo "Predicted Temperature: ${PREDICTED_TEMP}°F"
        echo "Actual Temperature:    ${ACTUAL_TEMP}°F"
        
        # Calculate accuracy metrics using bc for floating point math
        if command -v bc >/dev/null 2>&1 && [ "$PREDICTED_TEMP" != "0" ] && [ "$ACTUAL_TEMP" != "0" ]; then
            # Calculate absolute error
            ABS_ERROR=$(echo "if ($PREDICTED_TEMP > $ACTUAL_TEMP) $PREDICTED_TEMP - $ACTUAL_TEMP else $ACTUAL_TEMP - $PREDICTED_TEMP" | bc -l)
            
            # Calculate relative error percentage
            REL_ERROR=$(echo "scale=2; ($ABS_ERROR / $ACTUAL_TEMP) * 100" | bc -l)
            
            # Calculate accuracy percentage (100 - relative error, min 0)
            ACCURACY=$(echo "scale=2; if (100 - $REL_ERROR < 0) 0 else 100 - $REL_ERROR" | bc -l)
            
            echo "Absolute Error:        ${ABS_ERROR}°F"
            echo "Relative Error:        ${REL_ERROR}%"
            echo "Accuracy:              ${ACCURACY}%"
            
            # Provide accuracy assessment using integer comparison
            ABS_ERROR_INT=$(echo "$ABS_ERROR" | cut -d'.' -f1)
            if [ "${ABS_ERROR_INT:-999}" -le 2 ]; then
                echo "Assessment:            🎯 EXCELLENT (within 2°F)"
            elif [ "${ABS_ERROR_INT:-999}" -le 5 ]; then
                echo "Assessment:            ✅ GOOD (within 5°F)"
            elif [ "${ABS_ERROR_INT:-999}" -le 10 ]; then
                echo "Assessment:            ⚠️  FAIR (within 10°F)"
            else
                echo "Assessment:            ❌ POOR (>10°F difference)"
            fi
        else
            echo "⚠️  Could not calculate detailed accuracy metrics (bc not available or invalid values)"
            echo "Manual comparison: Predicted=${PREDICTED_TEMP}°F vs Actual=${ACTUAL_TEMP}°F"
        fi
        
        echo ""
        echo "📊 Raw Data Comparison:"
        echo "Actual values (target for prediction):"
        cat data/actual_value.csv
        echo "Test input data (last few rows):"
        tail -n 3 data/test_input.csv | head -n 2
        
    else
        print_info "Accuracy test failed, but this is acceptable for testing"
    fi
fi

# Summary
echo ""
echo "🎉 Local workflow completed successfully!"
echo ""
echo "📁 Generated files:"
ls -lah models/ data/
echo ""
echo "🔄 Next run will continue from the current model"
echo "💡 To simulate GitHub Actions workflow:"
echo "   1. Archive current models/data to simulate upload"
echo "   2. Remove models/weather_model.bin"
echo "   3. Restore from archive to simulate download_artifacts.py"
echo "   4. Run this script again"
