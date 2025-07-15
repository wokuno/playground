#!/usr/bin/env python3
"""
NOAA Weather Data Fetcher
Fetches real weather data from the National Weather Service API
https://api.weather.gov/
"""

import argparse
import csv
import json
import os
import sys
import time
from datetime import datetime, timedelta, timezone
from urllib.request import urlopen, Request
from urllib.parse import urlencode
from urllib.error import URLError, HTTPError

class NOAAWeatherFetcher:
    def __init__(self):
        self.base_url = "https://api.weather.gov"
        self.headers = {
            'User-Agent': 'Weather-LSTM-Prediction/1.0 (https://github.com/user/weather-lstm)',
            'Accept': 'application/geo+json'
        }
    
    def _make_request(self, url):
        """Make a request to the NOAA API"""
        try:
            request = Request(url, headers=self.headers)
            response = urlopen(request, timeout=30)
            data = response.read().decode('utf-8')
            return json.loads(data)
        except (URLError, HTTPError) as e:
            print(f"Error fetching from {url}: {e}")
            return None
        except json.JSONDecodeError as e:
            print(f"Error parsing JSON from {url}: {e}")
            return None
    
    def get_station_info(self, station_id):
        """Get information about a weather station"""
        url = f"{self.base_url}/stations/{station_id}"
        return self._make_request(url)
    
    def get_observations(self, station_id, start_time=None, end_time=None, limit=None):
        """Get weather observations for a station"""
        url = f"{self.base_url}/stations/{station_id}/observations"
        
        params = {}
        if start_time:
            params['start'] = start_time.replace(tzinfo=None).isoformat() + 'Z'
        if end_time:
            params['end'] = end_time.replace(tzinfo=None).isoformat() + 'Z'
        if limit:
            params['limit'] = str(limit)
        
        if params:
            url += '?' + urlencode(params)
        
        return self._make_request(url)
    
    def convert_to_imperial(self, value, unit):
        """Convert metric values to imperial units"""
        if value is None:
            return 0.0
        
        # Temperature: Celsius to Fahrenheit
        if unit == 'wmoUnit:degC':
            return (value * 9/5) + 32
        
        # Pressure: Pascal to inHg
        elif unit == 'wmoUnit:Pa':
            return value * 0.0002953
        
        # Wind speed: m/s to mph
        elif unit == 'wmoUnit:m_s-1':
            return value * 2.237
        
        # Wind direction: degrees (no conversion needed)
        elif unit == 'wmoUnit:degree_(angle)':
            return value
        
        # Precipitation: mm to inches
        elif unit == 'wmoUnit:mm':
            return value * 0.0394
        
        # Default: return as-is
        return value
    
    def extract_weather_data(self, observations):
        """Extract and convert weather data from NOAA observations"""
        weather_data = []
        
        for obs in observations.get('features', []):
            props = obs.get('properties', {})
            
            # Extract timestamp
            timestamp = props.get('timestamp')
            if not timestamp:
                continue
            
            # Extract temperature
            temp_data = props.get('temperature', {})
            temperature = self.convert_to_imperial(
                temp_data.get('value'), 
                temp_data.get('unitCode')
            ) if temp_data.get('value') is not None else 50.0  # Default fallback
            
            # Extract pressure
            pressure_data = props.get('barometricPressure', {})
            pressure = self.convert_to_imperial(
                pressure_data.get('value'), 
                pressure_data.get('unitCode')
            ) if pressure_data.get('value') is not None else 30.0  # Default fallback
            
            # Extract humidity
            humidity_data = props.get('relativeHumidity', {})
            humidity = humidity_data.get('value', 60.0) if humidity_data.get('value') is not None else 60.0
            
            # Extract wind speed
            wind_speed_data = props.get('windSpeed', {})
            wind_speed = self.convert_to_imperial(
                wind_speed_data.get('value'), 
                wind_speed_data.get('unitCode')
            ) if wind_speed_data.get('value') is not None else 8.0  # Default fallback
            
            # Extract wind direction
            wind_dir_data = props.get('windDirection', {})
            wind_direction = wind_dir_data.get('value', 180.0) if wind_dir_data.get('value') is not None else 180.0
            
            # Extract precipitation (if available)
            # NOAA doesn't always provide precipitation in observations
            precipitation = 0.0  # Default to no precipitation
            
            weather_point = {
                'timestamp': timestamp,
                'temperature': round(temperature, 2),
                'pressure': round(pressure, 2),
                'humidity': round(humidity, 2),
                'wind_speed': round(wind_speed, 2),
                'wind_direction': round(wind_direction, 0),
                'precipitation': round(precipitation, 4)
            }
            
            weather_data.append(weather_point)
        
        return weather_data
    
    def fetch_historical_data(self, station_id, days=30):
        """Fetch historical weather data for the specified number of days"""
        print(f"Fetching {days} days of historical data from NOAA station {station_id}...")
        
        end_time = datetime.now(timezone.utc)
        start_time = end_time - timedelta(days=days)
        
        # NOAA API has limits, so we'll fetch in smaller chunks if needed
        all_weather_data = []
        current_start = start_time
        
        while current_start < end_time:
            current_end = min(current_start + timedelta(days=7), end_time)
            
            print(f"Fetching data from {current_start.date()} to {current_end.date()}...")
            
            observations = self.get_observations(station_id, current_start, current_end)
            
            if observations:
                weather_data = self.extract_weather_data(observations)
                all_weather_data.extend(weather_data)
                print(f"Fetched {len(weather_data)} observations")
            else:
                print("No observations returned")
            
            current_start = current_end
            time.sleep(1)  # Be respectful to the API
        
        # Sort by timestamp (oldest first)
        all_weather_data.sort(key=lambda x: x['timestamp'])
        
        # Remove duplicates and filter to reasonable intervals (every 3-6 hours instead of daily)
        filtered_data = []
        last_timestamp = None
        min_interval_hours = 1  # Minimum 1 hour between observations

        for data in all_weather_data:
            current_timestamp = datetime.fromisoformat(data['timestamp'].replace('Z', '+00:00'))
            
            if last_timestamp is None or (current_timestamp - last_timestamp).total_seconds() >= min_interval_hours * 3600:
                filtered_data.append(data)
                last_timestamp = current_timestamp
        
        print(f"Filtered {len(all_weather_data)} observations down to {len(filtered_data)} data points (min {min_interval_hours}h intervals)")
        return filtered_data
    
    def fetch_recent_data(self, station_id, hours=48):
        """Fetch recent weather data for prediction testing"""
        print(f"Fetching last {hours} hours of data from NOAA station {station_id}...")
        
        end_time = datetime.now(timezone.utc)
        start_time = end_time - timedelta(hours=hours)
        
        observations = self.get_observations(station_id, start_time, end_time, limit=50)
        
        if observations:
            weather_data = self.extract_weather_data(observations)
            # Take the last 2 observations for testing
            return weather_data[-2:] if len(weather_data) >= 2 else weather_data
        
        return []

def get_latest_timestamp_from_csv(filename):
    """Get the latest timestamp from existing CSV file"""
    if not os.path.exists(filename):
        return None
    
    try:
        latest_timestamp = None
        with open(filename, 'r') as csvfile:
            # Read all lines to find the one with the latest timestamp
            reader = csv.DictReader(csvfile)
            for row in reader:
                # Note: we don't save timestamps in CSV for model training,
                # so we'll need to track this differently
                pass
        return latest_timestamp
    except Exception as e:
        print(f"Error reading existing file {filename}: {e}")
        return None

def load_existing_weather_data(filename):
    """Load existing weather data from CSV file"""
    if not os.path.exists(filename):
        return []
    
    existing_data = []
    # Core weather fields that the model expects
    core_fieldnames = ['temperature', 'pressure', 'humidity', 'wind_speed', 'wind_direction', 'precipitation']
    
    try:
        with open(filename, 'r') as csvfile:
            reader = csv.DictReader(csvfile)
            for row in reader:
                # Convert string values back to float for core weather fields
                data_point = {}
                for field in core_fieldnames:
                    if field in row:
                        try:
                            data_point[field] = float(row[field])
                        except ValueError:
                            data_point[field] = 0.0
                
                # Preserve timestamp fields if they exist
                if 'timestamp' in row:
                    data_point['timestamp'] = row['timestamp']
                if 'unix_timestamp' in row:
                    data_point['unix_timestamp'] = row['unix_timestamp']
                    
                existing_data.append(data_point)
    except Exception as e:
        print(f"Error loading existing data from {filename}: {e}")
        return []
    
    return existing_data

def filter_new_data(new_data, existing_data, tolerance=0.1):
    """Filter out data points that are already in the existing dataset"""
    if not existing_data:
        return new_data
    
    filtered_data = []
    
    for new_point in new_data:
        is_duplicate = False
        
        # Check if this data point already exists (with some tolerance for floating point comparison)
        for existing_point in existing_data:
            temp_diff = abs(new_point.get('temperature', 0) - existing_point.get('temperature', 0))
            pressure_diff = abs(new_point.get('pressure', 0) - existing_point.get('pressure', 0))
            humidity_diff = abs(new_point.get('humidity', 0) - existing_point.get('humidity', 0))
            
            # If temperature, pressure, and humidity are very close, consider it a duplicate
            if temp_diff < tolerance and pressure_diff < tolerance and humidity_diff < tolerance:
                is_duplicate = True
                break
        
        if not is_duplicate:
            filtered_data.append(new_point)
    
    return filtered_data

def save_to_csv_incremental(weather_data, filename, mode='incremental', include_timestamp=False):
    """Save weather data to CSV file with incremental updates"""
    if not weather_data:
        print("No weather data to save")
        return
    
    if include_timestamp:
        fieldnames = ['timestamp', 'unix_timestamp', 'temperature', 'pressure', 'humidity', 'wind_speed', 'wind_direction', 'precipitation']
    else:
        fieldnames = ['temperature', 'pressure', 'humidity', 'wind_speed', 'wind_direction', 'precipitation']
    
    if mode == 'incremental' and os.path.exists(filename):
        # Load existing data
        existing_data = load_existing_weather_data(filename)
        print(f"Found {len(existing_data)} existing data points in {filename}")
        
        # Filter out duplicates
        new_data = filter_new_data(weather_data, existing_data)
        
        if new_data:
            print(f"Adding {len(new_data)} new data points")
            
            # Append new data to existing file
            with open(filename, 'a', newline='') as csvfile:
                writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
                
                for data_point in new_data:
                    # Prepare row data
                    if include_timestamp:
                        # Convert ISO timestamp to Unix timestamp
                        iso_timestamp = data_point.get('timestamp', '')
                        unix_timestamp = 0
                        if iso_timestamp:
                            try:
                                dt = datetime.fromisoformat(iso_timestamp.replace('Z', '+00:00'))
                                unix_timestamp = int(dt.timestamp())
                            except ValueError:
                                unix_timestamp = 0
                        
                        row = {
                            'timestamp': iso_timestamp,
                            'unix_timestamp': unix_timestamp,
                            'temperature': data_point.get('temperature', 0),
                            'pressure': data_point.get('pressure', 0),
                            'humidity': data_point.get('humidity', 0),
                            'wind_speed': data_point.get('wind_speed', 0),
                            'wind_direction': data_point.get('wind_direction', 0),
                            'precipitation': data_point.get('precipitation', 0)
                        }
                    else:
                        # Write only the weather features (exclude timestamp for model training)
                        row = {key: data_point[key] for key in fieldnames if key in data_point}
                    
                    writer.writerow(row)
            
            print(f"Successfully appended {len(new_data)} new data points to {filename}")
            print(f"Total data points now: {len(existing_data) + len(new_data)}")
        else:
            print("No new data points to add (all data already exists)")
    else:
        # Write new file or overwrite mode
        with open(filename, 'w', newline='') as csvfile:
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()
            
            for data_point in weather_data:
                # Prepare row data
                if include_timestamp:
                    # Convert ISO timestamp to Unix timestamp
                    iso_timestamp = data_point.get('timestamp', '')
                    unix_timestamp = 0
                    if iso_timestamp:
                        try:
                            dt = datetime.fromisoformat(iso_timestamp.replace('Z', '+00:00'))
                            unix_timestamp = int(dt.timestamp())
                        except ValueError:
                            unix_timestamp = 0
                    
                    row = {
                        'timestamp': iso_timestamp,
                        'unix_timestamp': unix_timestamp,
                        'temperature': data_point.get('temperature', 0),
                        'pressure': data_point.get('pressure', 0),
                        'humidity': data_point.get('humidity', 0),
                        'wind_speed': data_point.get('wind_speed', 0),
                        'wind_direction': data_point.get('wind_direction', 0),
                        'precipitation': data_point.get('precipitation', 0)
                    }
                else:
                    # Write only the weather features (exclude timestamp for model training)
                    row = {key: data_point[key] for key in fieldnames if key in data_point}
                
                writer.writerow(row)
        
        print(f"Saved {len(weather_data)} weather data points to {filename}")

def save_to_csv(weather_data, filename, include_timestamp=False):
    """Save weather data to CSV file (legacy function for compatibility)"""
    save_to_csv_incremental(weather_data, filename, mode='overwrite', include_timestamp=include_timestamp)

def main():
    parser = argparse.ArgumentParser(description='Fetch weather data from NOAA API')
    parser.add_argument('--station', default='KMSP', 
                       help='NOAA weather station ID (default: KMSP for Minneapolis)')
    parser.add_argument('--days', type=int, default=30,
                       help='Number of days of historical data to fetch (default: 30)')
    parser.add_argument('--output', default='data/historical_data.csv',
                       help='Output CSV file path (default: data/historical_data.csv)')
    parser.add_argument('--recent', action='store_true',
                       help='Fetch only recent data for prediction testing')
    parser.add_argument('--hours', type=int, default=48,
                       help='Hours of recent data to fetch when using --recent (default: 48)')
    parser.add_argument('--incremental', action='store_true',
                       help='Only add new data points since last fetch (append to existing file)')
    parser.add_argument('--include-timestamp', action='store_true',
                       help='Include timestamp and unix_timestamp columns in CSV output')
    parser.add_argument('--info', action='store_true',
                       help='Show station information and exit')
    
    args = parser.parse_args()
    
    try:
        fetcher = NOAAWeatherFetcher()
        
        if args.info:
            # Show station information
            station_info = fetcher.get_station_info(args.station)
            if station_info:
                props = station_info.get('properties', {})
                print(f"Station ID: {args.station}")
                print(f"Name: {props.get('name', 'Unknown')}")
                print(f"State: {props.get('state', 'Unknown')}")
                print(f"Time Zone: {props.get('timeZone', 'Unknown')}")
                if 'elevation' in props:
                    elev = props['elevation']
                    print(f"Elevation: {elev.get('value', 'Unknown')} {elev.get('unitCode', '')}")
            else:
                print(f"Could not fetch information for station {args.station}")
            return 0
        
        if args.recent:
            # Fetch recent data for prediction testing
            weather_data = fetcher.fetch_recent_data(args.station, args.hours)
            if not args.output.endswith('recent_weather.csv'):
                args.output = args.output.replace('.csv', '_recent.csv')
        else:
            # Fetch historical data for training
            weather_data = fetcher.fetch_historical_data(args.station, args.days)
        
        if not weather_data:
            print("No weather data retrieved")
            return 1
        
        # Save to CSV with incremental mode if requested
        if args.incremental:
            save_to_csv_incremental(weather_data, args.output, mode='incremental', include_timestamp=args.include_timestamp)
        else:
            save_to_csv(weather_data, args.output, include_timestamp=args.include_timestamp)
        
        print(f"NOAA weather data fetch completed successfully!")
        print(f"Station: {args.station}")
        print(f"Data points fetched: {len(weather_data)}")
        print(f"Output file: {args.output}")
        
        # Show sample data
        if weather_data:
            print(f"\nSample data (first observation):")
            first_data = weather_data[0]
            print(f"  Temperature: {first_data['temperature']}°F")
            print(f"  Pressure: {first_data['pressure']} inHg")
            print(f"  Humidity: {first_data['humidity']}%")
            print(f"  Wind: {first_data['wind_speed']} mph @ {first_data['wind_direction']}°")
            print(f"  Precipitation: {first_data['precipitation']} in")
        
    except Exception as e:
        print(f"Error fetching NOAA weather data: {e}", file=sys.stderr)
        return 1
    
    return 0

if __name__ == '__main__':
    sys.exit(main())
