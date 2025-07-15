#include "../include/lstm.h"
#include "../include/weather_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_usage(const char* program_name) {
    printf("Usage: %s --model <model_file> --input <csv_file> [options]\n", program_name);
    printf("Options:\n");
    printf("  --model <file>       Path to trained model file\n");
    printf("  --input <file>       Path to input weather data CSV file\n");
    printf("  --output <file>      Output predictions to CSV file (optional)\n");
    printf("  --help               Show this help message\n");
}

int main(int argc, char* argv[]) {
    char* model_file = NULL;
    char* input_file = NULL;
    char* output_file = NULL;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--model") == 0 && i + 1 < argc) {
            model_file = argv[++i];
        } else if (strcmp(argv[i], "--input") == 0 && i + 1 < argc) {
            input_file = argv[++i];
        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            printf("Unknown argument: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Validate required arguments
    if (!model_file || !input_file) {
        printf("Error: Missing required arguments\n");
        print_usage(argv[0]);
        return 1;
    }
    
    printf("Weather LSTM Prediction\n");
    printf("======================\n");
    printf("Model file: %s\n", model_file);
    printf("Input file: %s\n", input_file);
    if (output_file) {
        printf("Output file: %s\n", output_file);
    }
    printf("\n");
    
    // Load the trained model
    printf("Loading trained model...\n");
    LSTMNetwork* network = load_lstm_model(model_file);
    if (!network) {
        printf("Error: Could not load model from %s\n", model_file);
        return 1;
    }
    
    printf("Model loaded successfully\n");
    printf("Input size: %d, Hidden size: %d, Output size: %d\n",
           network->input_size, network->hidden_size, network->output_size);
    printf("Sequence length: %d\n", network->sequence_length);
    
    // Load input weather data
    printf("\nLoading input weather data...\n");
    WeatherDataset* input_data = weather_dataset_create(1000);
    if (!input_data) {
        printf("Error: Could not create input dataset\n");
        lstm_network_free(network);
        return 1;
    }
    
    if (weather_load_csv(input_file, input_data) != 0) {
        printf("Error: Could not load input data from %s\n", input_file);
        weather_dataset_free(input_data);
        lstm_network_free(network);
        return 1;
    }
    
    if (input_data->size < network->sequence_length) {
        printf("Error: Input data too small. Need at least %d data points, got %d\n",
               network->sequence_length, input_data->size);
        weather_dataset_free(input_data);
        lstm_network_free(network);
        return 1;
    }
    
    // Normalize input data if normalization parameters are available
    if (network->norm_params) {
        printf("Normalizing input data...\n");
        normalize_dataset(input_data, network->norm_params);
    } else {
        printf("Warning: No normalization parameters found in model\n");
    }
    
    // Make prediction using the most recent sequence
    printf("\nMaking prediction...\n");
    WeatherPoint prediction = lstm_predict_next(network, input_data, network->sequence_length);
    
    // Denormalize prediction if parameters are available
    if (network->norm_params) {
        denormalize_point(&prediction, network->norm_params);
    }
    
    // Display prediction
    printf("\nPredicted next weather conditions:\n");
    printf("==================================\n");
    print_weather_point(&prediction);
    
    // If we have at least one more data point, compare with actual
    int actual_idx = input_data->size - 1;
    if (actual_idx >= network->sequence_length) {
        printf("\nComparison with most recent actual data:\n");
        printf("=======================================\n");
        
        WeatherPoint actual = input_data->data[actual_idx];
        if (network->norm_params) {
            denormalize_point(&actual, network->norm_params);
        }
        
        printf("Actual: ");
        print_weather_point(&actual);
        
        // Calculate error metrics
        double temp_error = fabs(prediction.temperature - actual.temperature);
        double pressure_error = fabs(prediction.pressure - actual.pressure);
        double humidity_error = fabs(prediction.humidity - actual.humidity);
        double wind_speed_error = fabs(prediction.wind_speed - actual.wind_speed);
        double wind_dir_error = fabs(prediction.wind_direction - actual.wind_direction);
        double precip_error = fabs(prediction.precipitation - actual.precipitation);
        
        printf("\nPrediction Errors:\n");
        printf("Temperature: %.2f°F\n", temp_error);
        printf("Pressure: %.2f inHg\n", pressure_error);
        printf("Humidity: %.2f%%\n", humidity_error);
        printf("Wind Speed: %.2f mph\n", wind_speed_error);
        printf("Wind Direction: %.0f°\n", wind_dir_error);
        printf("Precipitation: %.4f in\n", precip_error);
        
        // Calculate overall accuracy (inverse of normalized RMSE)
        double total_error = temp_error + pressure_error + humidity_error + 
                           wind_speed_error + wind_dir_error + precip_error;
        printf("Total Absolute Error: %.4f\n", total_error);
    }
    
    // Save predictions to output file if specified
    if (output_file) {
        printf("\nSaving prediction to %s...\n", output_file);
        
        WeatherDataset* output_data = weather_dataset_create(1);
        if (output_data) {
            weather_dataset_add(output_data, prediction);
            
            if (weather_save_csv(output_file, output_data) == 0) {
                printf("Prediction saved successfully\n");
            } else {
                printf("Error: Could not save prediction\n");
            }
            
            weather_dataset_free(output_data);
        }
    }
    
    // Show input sequence used for prediction
    printf("\nInput sequence used for prediction:\n");
    printf("==================================\n");
    int start_idx = input_data->size - network->sequence_length;
    for (int i = 0; i < network->sequence_length; i++) {
        WeatherPoint point = input_data->data[start_idx + i];
        if (network->norm_params) {
            denormalize_point(&point, network->norm_params);
        }
        printf("Step %d: ", i + 1);
        print_weather_point(&point);
    }
    
    // Clean up
    weather_dataset_free(input_data);
    lstm_network_free(network);
    
    printf("\nPrediction completed successfully!\n");
    return 0;
}
