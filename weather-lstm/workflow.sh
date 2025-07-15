#!/bin/bash
# End-to-end workflow for Weather LSTM Prediction
# This script demonstrates the complete workflow from data collection to prediction

set -e

echo "=== Weather LSTM Prediction Workflow ==="
echo "========================================"

# Step 1: Prepare the environment
echo "Step 1: Preparing environment..."
make clean
make all
mkdir -p models

# Step 2: Fetch training data (30 days)
echo "Step 2: Fetching training data from NOAA..."

# Try NOAA first, fallback to synthetic if it fails
if command -v pip3 &> /dev/null; then
    echo "Installing requests library for NOAA API..."
    pip3 install requests --quiet || echo "Failed to install requests"
fi

if python3 scripts/fetch_noaa_weather.py \
    --station KMSP \
    --days 30 \
    --output data/weather_training.csv 2>/dev/null; then
    echo "✅ Successfully fetched real NOAA weather data"
else
    echo "⚠️  NOAA fetch failed, using synthetic data..."
    python3 scripts/fetch_weather.py \
        --location KMSP \
        --days 30 \
        --output data/weather_training.csv
fi

# Step 3: Fetch recent data for testing (2 data points)
echo "Step 3: Fetching recent data from NOAA for testing..."

if python3 scripts/fetch_noaa_weather.py \
    --station KMSP \
    --recent \
    --output data/recent_weather.csv 2>/dev/null; then
    echo "✅ Successfully fetched real NOAA recent data"
else
    echo "⚠️  NOAA recent fetch failed, using synthetic data..."
    python3 scripts/fetch_weather.py \
        --location KMSP \
        --recent \
        --output data/recent_weather.csv
fi

# Step 4: Train the model
echo "Step 4: Training LSTM model..."
./bin/train \
    --data data/weather_training.csv \
    --epochs 100 \
    --output models/weather_lstm.bin \
    --hidden 64 \
    --sequence 10 \
    --learning-rate 0.001

# Step 5: Use older of the 2 recent data points to predict the newer one
echo "Step 5: Testing prediction accuracy..."

# Create input with just the first (older) data point
head -n 1 data/recent_weather.csv > data/prediction_input.csv
head -n 2 data/recent_weather.csv | tail -n 1 >> data/prediction_input.csv

# Make prediction
./bin/predict \
    --model models/weather_lstm.bin \
    --input data/prediction_input.csv \
    --output data/prediction_result.csv

echo "Step 6: Comparing prediction with actual newer data point..."

# Extract the actual newer data point for comparison
tail -n 1 data/recent_weather.csv > data/actual_result.csv

echo "Predicted weather:"
cat data/prediction_result.csv

echo ""
echo "Actual weather (from recent data):"
echo "temperature,pressure,humidity,wind_speed,wind_direction,precipitation"
cat data/actual_result.csv

echo ""
echo "=== Workflow Complete ==="
echo "The model has been trained on 30 days of data and tested by:"
echo "1. Using the older of 2 recent data points as input"
echo "2. Predicting the next time step"
echo "3. Comparing with the actual newer data point"
echo ""
echo "Files created:"
echo "- models/weather_lstm.bin (trained model)"
echo "- data/prediction_result.csv (prediction)"
echo "- data/actual_result.csv (actual weather for comparison)"
