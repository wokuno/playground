#!/bin/bash
# Data preparation script for weather LSTM prediction

set -e  # Exit on any error

# Default values
LOCATION="KMSP"
DAYS=30
DATA_DIR="data"
FORCE_FETCH=false

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to show usage
usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -l, --location LOCATION    Weather station code (default: KMSP)"
    echo "  -d, --days DAYS           Number of days to fetch (default: 30)"
    echo "  -o, --output-dir DIR      Output directory (default: data)"
    echo "  -f, --force               Force re-fetch even if data exists"
    echo "  -h, --help               Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                        # Fetch 30 days of KMSP data"
    echo "  $0 -l KORD -d 60         # Fetch 60 days of Chicago O'Hare data"
    echo "  $0 -f                    # Force re-fetch data"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -l|--location)
            LOCATION="$2"
            shift 2
            ;;
        -d|--days)
            DAYS="$2"
            shift 2
            ;;
        -o|--output-dir)
            DATA_DIR="$2"
            shift 2
            ;;
        -f|--force)
            FORCE_FETCH=true
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# Validate inputs
if ! [[ "$DAYS" =~ ^[0-9]+$ ]] || [ "$DAYS" -lt 1 ]; then
    print_error "Days must be a positive integer"
    exit 1
fi

# Create data directory if it doesn't exist
mkdir -p "$DATA_DIR"

# File paths
WEATHER_DATA="$DATA_DIR/historical_data.csv"
RECENT_DATA="$DATA_DIR/recent_weather.csv"

print_status "Weather LSTM Data Preparation"
print_status "============================="
print_status "Location: $LOCATION"
print_status "Days: $DAYS"
print_status "Data directory: $DATA_DIR"
print_status "Force fetch: $FORCE_FETCH"
echo ""

# Check if Python is available
if ! command -v python3 &> /dev/null; then
    print_error "Python 3 is required but not installed"
    exit 1
fi

# Check if weather data already exists
if [ -f "$WEATHER_DATA" ] && [ "$FORCE_FETCH" = false ]; then
    print_warning "Weather data already exists at $WEATHER_DATA"
    print_warning "Use -f/--force to re-fetch data"
    
    # Count lines (subtract 1 for header)
    DATA_POINTS=$(($(wc -l < "$WEATHER_DATA") - 1))
    print_status "Existing data contains $DATA_POINTS data points"
else
    # Fetch weather data
    print_status "Fetching weather data for $LOCATION..."
    
    if python3 scripts/fetch_weather.py \
        --location "$LOCATION" \
        --days "$DAYS" \
        --output "$WEATHER_DATA"; then
        print_status "Weather data fetched successfully"
    else
        print_error "Failed to fetch weather data"
        exit 1
    fi
fi

# Fetch recent weather data for testing
print_status "Fetching recent weather data for testing..."
if python3 scripts/fetch_weather.py \
    --location "$LOCATION" \
    --recent \
    --output "$RECENT_DATA"; then
    print_status "Recent weather data fetched successfully"
else
    print_error "Failed to fetch recent weather data"
    exit 1
fi

# Validate data files
print_status "Validating data files..."

# Check if files exist and are not empty
for file in "$WEATHER_DATA" "$RECENT_DATA"; do
    if [ ! -f "$file" ]; then
        print_error "Data file not found: $file"
        exit 1
    fi
    
    if [ ! -s "$file" ]; then
        print_error "Data file is empty: $file"
        exit 1
    fi
    
    # Count data points (subtract 1 for header)
    DATA_POINTS=$(($(wc -l < "$file") - 1))
    print_status "$(basename "$file"): $DATA_POINTS data points"
done

# Check data format
print_status "Checking data format..."

# Check header format
EXPECTED_HEADER="temperature,pressure,humidity,wind_speed,wind_direction,precipitation"
ACTUAL_HEADER=$(head -n 1 "$WEATHER_DATA")

if [ "$ACTUAL_HEADER" != "$EXPECTED_HEADER" ]; then
    print_error "Invalid header format in $WEATHER_DATA"
    print_error "Expected: $EXPECTED_HEADER"
    print_error "Actual: $ACTUAL_HEADER"
    exit 1
fi

# Check if we have enough data for training
MIN_DATA_POINTS=20
TRAINING_DATA_POINTS=$(($(wc -l < "$WEATHER_DATA") - 1))

if [ "$TRAINING_DATA_POINTS" -lt "$MIN_DATA_POINTS" ]; then
    print_error "Insufficient training data: $TRAINING_DATA_POINTS data points"
    print_error "Minimum required: $MIN_DATA_POINTS data points"
    exit 1
fi

# Show data statistics
print_status "Data preparation completed successfully!"
print_status "Summary:"
print_status "  Training data: $TRAINING_DATA_POINTS data points"
print_status "  Recent data: $(($(wc -l < "$RECENT_DATA") - 1)) data points"
print_status "  Data files ready for training"

# Show sample data
print_status ""
print_status "Sample data from $WEATHER_DATA:"
echo "$(head -n 3 "$WEATHER_DATA")"

print_status ""
print_status "Recent data from $RECENT_DATA:"
echo "$(head -n 3 "$RECENT_DATA")"

print_status ""
print_status "Data preparation completed! You can now run:"
print_status "  make all                    # Build the C programs"
print_status "  ./bin/train --data $WEATHER_DATA --epochs 100 --output models/weather_model.bin"
print_status "  ./bin/predict --model models/weather_model.bin --input $RECENT_DATA"
