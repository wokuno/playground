#!/usr/bin/env python3
"""
Continue Training Script
Downloads latest artifacts and continues training the LSTM model
"""

import os
import sys
import subprocess
import argparse

def run_command(cmd, description):
    """Run a command and print its output"""
    print(f"\n=== {description} ===")
    print(f"Running: {cmd}")
    
    try:
        result = subprocess.run(cmd, shell=True, check=True, 
                              capture_output=True, text=True)
        if result.stdout:
            print(result.stdout)
        return True
    except subprocess.CalledProcessError as e:
        print(f"Error: {e}")
        if e.stdout:
            print(f"stdout: {e.stdout}")
        if e.stderr:
            print(f"stderr: {e.stderr}")
        return False

def check_file_exists(filepath, description):
    """Check if a file exists and print status"""
    if os.path.exists(filepath):
        size = os.path.getsize(filepath)
        print(f"✅ Found {description}: {filepath} ({size} bytes)")
        return True
    else:
        print(f"❌ Missing {description}: {filepath}")
        return False

def main():
    parser = argparse.ArgumentParser(description='Continue training LSTM model with latest artifacts')
    parser.add_argument('--owner', help='GitHub repository owner')
    parser.add_argument('--repo', help='GitHub repository name')
    parser.add_argument('--token', help='GitHub token (or set GITHUB_TOKEN env var)')
    parser.add_argument('--epochs', type=int, default=50, help='Additional epochs to train')
    parser.add_argument('--use-noaa', action='store_true', help='Fetch fresh data from NOAA API')
    parser.add_argument('--station', default='KMSP', help='NOAA station ID (default: KMSP)')
    parser.add_argument('--days', type=int, default=30, help='Days of training data to fetch')
    
    args = parser.parse_args()
    
    print("Weather LSTM - Continue Training")
    print("================================")
    
    # Step 1: Build programs
    if not run_command("make clean && make all", "Building C programs"):
        print("Failed to build programs")
        return 1
    
    # Step 2: Download latest artifacts (if repo info provided)
    if args.owner and args.repo:
        print(f"\nDownloading latest artifacts from {args.owner}/{args.repo}...")
        
        download_cmd = f"python3 scripts/download_artifacts.py --owner {args.owner} --repo {args.repo}"
        if args.token:
            download_cmd += f" --token {args.token}"
        
        if run_command(download_cmd, "Downloading GitHub artifacts"):
            print("✅ Artifacts downloaded successfully")
        else:
            print("⚠️  Failed to download artifacts, will train from scratch")
    else:
        print("No repository info provided, skipping artifact download")
    
    # Step 3: Fetch fresh weather data
    if args.use_noaa:
        print(f"\nFetching fresh data from NOAA API...")
        
        # Install requests if needed
        try:
            import requests
        except ImportError:
            print("Installing requests library...")
            if not run_command("pip3 install requests", "Installing requests"):
                print("Failed to install requests, falling back to synthetic data")
                args.use_noaa = False
        
        if args.use_noaa:
            # Fetch training data
            noaa_cmd = f"python3 scripts/fetch_noaa_weather.py --station {args.station} --days {args.days} --output data/noaa_training.csv"
            if run_command(noaa_cmd, "Fetching NOAA training data"):
                training_data = "data/noaa_training.csv"
            else:
                print("Failed to fetch NOAA data, falling back to synthetic data")
                args.use_noaa = False
            
            # Fetch recent data
            if args.use_noaa:
                recent_cmd = f"python3 scripts/fetch_noaa_weather.py --station {args.station} --recent --output data/noaa_recent.csv"
                if run_command(recent_cmd, "Fetching NOAA recent data"):
                    recent_data = "data/noaa_recent.csv"
                else:
                    recent_data = None
    
    # Fallback to synthetic data if NOAA failed
    if not args.use_noaa:
        print("\nUsing synthetic weather data...")
        
        synthetic_cmd = f"python3 scripts/fetch_weather.py --days {args.days} --output data/synthetic_training.csv"
        if run_command(synthetic_cmd, "Generating synthetic training data"):
            training_data = "data/synthetic_training.csv"
        else:
            print("Failed to generate synthetic data")
            return 1
        
        recent_cmd = "python3 scripts/fetch_weather.py --recent --output data/synthetic_recent.csv"
        if run_command(recent_cmd, "Generating synthetic recent data"):
            recent_data = "data/synthetic_recent.csv"
        else:
            recent_data = None
    
    # Step 4: Check for existing model
    existing_model = None
    model_files = [
        "models/weather_model.bin",
        "models/weather_lstm.bin", 
        "models/test_model.bin"
    ]
    
    for model_file in model_files:
        if check_file_exists(model_file, "existing model"):
            existing_model = model_file
            break
    
    # Step 5: Train or continue training
    if existing_model:
        print(f"\n🔄 Continuing training from existing model: {existing_model}")
        
        # For now, we'll train a new model since our C implementation 
        # doesn't support incremental training yet
        # In a full implementation, you'd modify the train program to load existing weights
        print("Note: Current implementation trains new model (incremental training not yet implemented)")
        
        output_model = "models/continued_model.bin"
    else:
        print("\n🆕 Training new model from scratch")
        output_model = "models/weather_model.bin"
    
    # Ensure models directory exists
    os.makedirs("models", exist_ok=True)
    
    # Training command
    train_cmd = (f"./bin/train "
                f"--data {training_data} "
                f"--epochs {args.epochs} "
                f"--output {output_model} "
                f"--hidden 64 "
                f"--sequence 10 "
                f"--learning-rate 0.001")
    
    if not run_command(train_cmd, f"Training LSTM model for {args.epochs} epochs"):
        print("Training failed")
        return 1
    
    # Step 6: Test the trained model
    if recent_data and check_file_exists(recent_data, "recent test data"):
        print(f"\n🧪 Testing trained model...")
        
        test_cmd = f"./bin/predict --model {output_model} --input {recent_data}"
        if run_command(test_cmd, "Testing model prediction"):
            print("✅ Model testing completed")
        else:
            print("⚠️  Model testing failed")
    
    # Step 7: Summary
    print("\n" + "="*50)
    print("TRAINING COMPLETED")
    print("="*50)
    
    print(f"✅ Model saved to: {output_model}")
    print(f"✅ Training data: {training_data}")
    if recent_data:
        print(f"✅ Test data: {recent_data}")
    
    # Show available files
    print("\nGenerated files:")
    for root, dirs, files in os.walk("."):
        for file in files:
            if file.endswith(('.bin', '.csv')) and not file.startswith('.'):
                filepath = os.path.join(root, file)
                size = os.path.getsize(filepath)
                print(f"  {filepath} ({size} bytes)")
    
    print(f"\n🎉 Continue training completed successfully!")
    print(f"\nNext steps:")
    print(f"  1. Test predictions: ./bin/predict --model {output_model} --input {recent_data if recent_data else 'data/recent_weather.csv'}")
    print(f"  2. Run more epochs: python3 scripts/continue_training.py --epochs 100")
    print(f"  3. Try different parameters: ./bin/train --help")
    
    return 0

if __name__ == '__main__':
    sys.exit(main())
