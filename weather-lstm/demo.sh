#!/bin/bash
# Demo script showing the complete weather prediction workflow

echo "🌤️  Weather LSTM Prediction - Complete Demo"
echo "============================================"

# Build the system
echo "📦 Building the system..."
make clean && make all

# Get real weather data for training (15 days to ensure enough data)
echo "🌐 Fetching real weather data from NOAA..."
python3 scripts/fetch_noaa_weather.py --station KMSP --days 15 --output data/demo_training.csv

# Train a model with shorter sequence for demo
echo "🧠 Training LSTM model..."
./bin/train \
  --data data/demo_training.csv \
  --epochs 30 \
  --output models/demo_model.bin \
  --hidden 32 \
  --sequence 5 \
  --learning-rate 0.01

# Get recent data for prediction
echo "📊 Fetching recent weather data..."
python3 scripts/fetch_noaa_weather.py --station KMSP --days 7 --output data/demo_recent.csv

# Make prediction
echo "🔮 Making weather prediction..."
./bin/predict --model models/demo_model.bin --input data/demo_recent.csv

echo ""
echo "✅ Demo completed!"
echo ""
echo "What happened:"
echo "1. ✅ Built C programs for LSTM neural network"
echo "2. ✅ Fetched real weather data from NOAA API (Minneapolis station KMSP)"
echo "3. ✅ Trained LSTM model on historical weather patterns"
echo "4. ✅ Used recent weather data to predict next conditions"
echo ""
echo "The model predicts 6 weather parameters:"
echo "  • Temperature (°F)"
echo "  • Atmospheric pressure (inHg)"
echo "  • Humidity (%)"
echo "  • Wind speed (mph)"
echo "  • Wind direction (degrees)"
echo "  • Precipitation (inches)"
echo ""
echo "🚀 Ready for production use!"
