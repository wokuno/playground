# Playground Repository

This repository contains various coding experiments and benchmarks.

## Projects

### Missing Item Benchmark (`missing-item-benchmark/`)
A comprehensive benchmark project that tests different methods for finding a missing element in a list using various approaches including XOR operations, loops, sets, and NumPy.

**Features:**
- Tests different algorithms for finding missing elements
- Benchmarks performance across different array sizes  
- Includes both implementations in multiple languages (python, c, go)

[See the Missing Item Benchmark README](missing-item-benchmark/README.md) for detailed setup and usage instructions.

### Weather LSTM Prediction (`weather-lstm-prediction/`)
A neural network project that uses LSTM to predict weather patterns based on historical data from Weather Underground, implemented entirely in C.

**Features:**
- Pure C implementation of LSTM neural network for training and inference
- Weather data collection from Weather Underground (KMSP - Minneapolis)
- Multi-variable prediction: temperature, pressure, humidity, wind speed/direction, precipitation
- GitHub Actions workflow for automated training and accuracy testing
- Comprehensive testing and benchmarking

[See the Weather LSTM Prediction README](weather-lstm-prediction/README.md) for detailed setup and usage instructions.

## GitHub Actions./
The repository includes automated testing workflows that run on push/PR to the main branch. Each project has its own testing configuration optimized for its specific requirements.