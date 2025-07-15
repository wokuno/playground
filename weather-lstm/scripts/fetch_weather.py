#!/usr/bin/env python3
"""
Synthetic Weather Data Generator
Generates synthetic weather data as a fallback when NOAA API is unavailable
"""

import argparse
import csv
import random
import math
from datetime import datetime, timedelta, timezone

def generate_synthetic_weather(location, days, include_recent=False, include_timestamp=False):
    """Generate synthetic weather data for a given location and time period"""
    
    # Base values for different locations (rough approximations)
    location_params = {
        'KMSP': {  # Minneapolis
            'temp_base': 50,
            'temp_range': 40,
            'humidity_base': 60,
            'pressure_base': 30.0,
            'wind_base': 8
        },
        'KJFK': {  # New York JFK
            'temp_base': 55,
            'temp_range': 35,
            'humidity_base': 65,
            'pressure_base': 30.1,
            'wind_base': 10
        },
        'default': {
            'temp_base': 60,
            'temp_range': 30,
            'humidity_base': 50,
            'pressure_base': 30.0,
            'wind_base': 7
        }
    }
    
    params = location_params.get(location, location_params['default'])
    
    # Start from days ago
    start_date = datetime.now() - timedelta(days=days)
    
    data = []
    
    for i in range(days):
        current_date = start_date + timedelta(days=i)
        
        # Create some seasonal variation
        day_of_year = current_date.timetuple().tm_yday
        seasonal_factor = math.sin((day_of_year / 365.0) * 2 * math.pi)
        
        # Temperature with seasonal variation and daily randomness
        temp = (params['temp_base'] + 
                seasonal_factor * params['temp_range'] * 0.5 +
                random.uniform(-10, 10))
        
        # Humidity (inversely related to temperature)
        humidity = max(20, min(100, 
                      params['humidity_base'] - (temp - params['temp_base']) * 0.3 +
                      random.uniform(-15, 15)))
        
        # Pressure (slight random variation)
        pressure = params['pressure_base'] + random.uniform(-0.5, 0.5)
        
        # Wind speed (correlated with pressure changes)
        wind_speed = max(0, params['wind_base'] + 
                        (pressure - params['pressure_base']) * 10 +
                        random.uniform(-5, 5))
        
        # Wind direction (random but somewhat persistent)
        if i == 0:
            wind_direction = random.uniform(0, 360)
        else:
            # Drift from previous direction
            wind_direction = (data[-1]['wind_direction'] + 
                            random.uniform(-30, 30)) % 360
        
        data_point = {
            'temperature': round(temp, 1),
            'pressure': round(pressure, 2),
            'humidity': round(humidity, 1),
            'wind_speed': round(wind_speed, 1),
            'wind_direction': round(wind_direction, 1),
            'precipitation': 0.0  # No precipitation for synthetic data
        }
        
        # Add timestamp if requested
        if include_timestamp:
            data_point['timestamp'] = current_date.replace(tzinfo=timezone.utc).isoformat()
            data_point['unix_timestamp'] = int(current_date.timestamp())
            
        data.append(data_point)
    
    # Add recent data if requested
    if include_recent:
        recent_start = datetime.now() - timedelta(hours=24)
        for i in range(24):  # Last 24 hours
            current_time = recent_start + timedelta(hours=i)
            
            # Use last day's data as base
            base_data = data[-1] if data else {
                'temperature': params['temp_base'],
                'humidity': params['humidity_base'],
                'pressure': params['pressure_base'],
                'wind_speed': params['wind_base'],
                'wind_direction': 180
            }
            
            # Small variations for recent data
            recent_data = {
                'temperature': round(base_data['temperature'] + random.uniform(-3, 3), 1),
                'pressure': round(base_data['pressure'] + random.uniform(-0.2, 0.2), 2),
                'humidity': round(max(20, min(100, base_data['humidity'] + random.uniform(-10, 10))), 1),
                'wind_speed': round(max(0, base_data['wind_speed'] + random.uniform(-2, 2)), 1),
                'wind_direction': round((base_data['wind_direction'] + random.uniform(-20, 20)) % 360, 1),
                'precipitation': 0.0
            }
            
            # Add timestamp if requested
            if include_timestamp:
                recent_data['timestamp'] = current_time.replace(tzinfo=timezone.utc).isoformat()
                recent_data['unix_timestamp'] = int(current_time.timestamp())
            
            data.append(recent_data)
    
    return data

def write_csv(data, output_file, include_timestamp=False):
    """Write weather data to CSV file in C-compatible format"""
    # Base C-compatible fields
    if include_timestamp:
        fieldnames = ['timestamp', 'unix_timestamp', 'temperature', 'pressure', 'humidity', 'wind_speed', 'wind_direction', 'precipitation']
    else:
        # C expects: temp,pressure,humidity,wind_speed,wind_dir,precipitation
        fieldnames = ['temperature', 'pressure', 'humidity', 'wind_speed', 'wind_direction', 'precipitation']
    
    with open(output_file, 'w', newline='') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        for row in data:
            # Only write fields that are in fieldnames
            filtered_row = {field: row.get(field, 0) for field in fieldnames}
            writer.writerow(filtered_row)

def main():
    parser = argparse.ArgumentParser(description='Generate synthetic weather data')
    parser.add_argument('--location', default='KMSP', 
                       help='Location code (KMSP, KJFK, etc.) - default: KMSP')
    parser.add_argument('--days', type=int, default=30,
                       help='Number of days of historical data - default: 30')
    parser.add_argument('--output', required=True,
                       help='Output CSV file path')
    parser.add_argument('--recent', action='store_true',
                       help='Include recent hourly data for testing')
    parser.add_argument('--include-timestamp', action='store_true',
                       help='Include timestamp and unix_timestamp columns in CSV output')
    
    args = parser.parse_args()
    
    print(f"Generating synthetic weather data for {args.location}")
    print(f"Days: {args.days}")
    print(f"Include recent data: {args.recent}")
    
    # Generate data
    data = generate_synthetic_weather(args.location, args.days, args.recent, args.include_timestamp)
    
    # Write to file
    write_csv(data, args.output, args.include_timestamp)
    
    print(f"✅ Generated {len(data)} data points")
    print(f"📄 Saved to: {args.output}")
    
    # Show sample data
    if len(data) > 0:
        print("\nSample data:")
        print(f"First: {data[0]}")
        print(f"Last:  {data[-1]}")

if __name__ == '__main__':
    main()
