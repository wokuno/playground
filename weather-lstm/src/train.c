#include "../include/lstm.h"
#include "../include/weather_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void print_usage(const char* program_name) {
    printf("Usage: %s --data <csv_file> --epochs <num_epochs> --output <model_file> [options]\n", program_name);
    printf("Options:\n");
    printf("  --data <file>        Path to weather data CSV file\n");
    printf("  --epochs <number>    Number of training epochs (default: 100)\n");
    printf("  --output <file>      Output model file path\n");
    printf("  --hidden <size>      Hidden layer size (default: 64)\n");
    printf("  --sequence <length>  Sequence length (default: 10)\n");
    printf("  --learning-rate <lr> Learning rate (default: 0.001)\n");
    printf("  --help               Show this help message\n");
}

int main(int argc, char* argv[]) {
    // Default parameters
    char* data_file = NULL;
    char* model_file = NULL;
    int epochs = 100;
    int hidden_size = 64;
    int sequence_length = 10;
    double learning_rate = 0.001;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--data") == 0 && i + 1 < argc) {
            data_file = argv[++i];
        } else if (strcmp(argv[i], "--epochs") == 0 && i + 1 < argc) {
            epochs = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            model_file = argv[++i];
        } else if (strcmp(argv[i], "--hidden") == 0 && i + 1 < argc) {
            hidden_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--sequence") == 0 && i + 1 < argc) {
            sequence_length = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--learning-rate") == 0 && i + 1 < argc) {
            learning_rate = atof(argv[++i]);
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
    if (!data_file || !model_file) {
        printf("Error: Missing required arguments\n");
        print_usage(argv[0]);
        return 1;
    }
    
    if (epochs <= 0 || hidden_size <= 0 || sequence_length <= 0 || learning_rate <= 0) {
        printf("Error: Invalid parameter values\n");
        return 1;
    }
    
    printf("Weather LSTM Training\n");
    printf("====================\n");
    printf("Data file: %s\n", data_file);
    printf("Model file: %s\n", model_file);
    printf("Epochs: %d\n", epochs);
    printf("Hidden size: %d\n", hidden_size);
    printf("Sequence length: %d\n", sequence_length);
    printf("Learning rate: %.4f\n", learning_rate);
    printf("\n");
    
    // Initialize random seed
    srand(time(NULL));
    
    // Load weather data
    printf("Loading weather data...\n");
    WeatherDataset* dataset = weather_dataset_create(1000);
    if (!dataset) {
        printf("Error: Could not create dataset\n");
        return 1;
    }
    
    if (weather_load_csv(data_file, dataset) != 0) {
        printf("Error: Could not load weather data from %s\n", data_file);
        weather_dataset_free(dataset);
        return 1;
    }
    
    if (dataset->size <= sequence_length) {
        printf("Error: Dataset too small. Need at least %d data points, got %d\n", 
               sequence_length + 1, dataset->size);
        weather_dataset_free(dataset);
        return 1;
    }
    
    // Calculate and apply normalization
    printf("Normalizing data...\n");
    NormalizationParams* norm_params = calculate_normalization_params(dataset);
    if (!norm_params) {
        printf("Error: Could not calculate normalization parameters\n");
        weather_dataset_free(dataset);
        return 1;
    }
    
    print_normalization_params(norm_params);
    normalize_dataset(dataset, norm_params);
    
    // Create training data
    printf("Creating training sequences...\n");
    TrainingData* training_data = create_training_data(dataset, sequence_length);
    if (!training_data) {
        printf("Error: Could not create training data\n");
        free(norm_params);
        weather_dataset_free(dataset);
        return 1;
    }
    
    printf("Created %d training sequences\n", training_data->num_sequences);
    
    // Create LSTM network
    printf("Creating LSTM network...\n");
    LSTMNetwork* network = lstm_network_create(6, hidden_size, 6); // 6 weather features
    if (!network) {
        printf("Error: Could not create LSTM network\n");
        free_training_data(training_data);
        free(norm_params);
        weather_dataset_free(dataset);
        return 1;
    }
    
    network->learning_rate = learning_rate;
    network->sequence_length = sequence_length;
    network->norm_params = norm_params;
    
    // Train the network
    printf("Starting training...\n");
    clock_t start_time = clock();
    
    lstm_train(network, training_data, epochs);
    
    clock_t end_time = clock();
    double training_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Training completed in %.2f seconds\n", training_time);
    
    // Test the model on the last sequence
    printf("\nTesting model on last sequence...\n");
    Matrix** test_sequence = training_data->inputs[training_data->num_sequences - 1];
    Matrix* predicted = lstm_network_predict(network, test_sequence, sequence_length);
    Matrix* actual = training_data->targets[training_data->num_sequences - 1];
    
    if (predicted && actual) {
        double test_loss = calculate_loss(predicted, actual);
        printf("Test loss: %.6f\n", test_loss);
        
        // Convert back to weather points for readable output
        WeatherPoint pred_weather = matrix_to_weather_point(predicted);
        WeatherPoint actual_weather = matrix_to_weather_point(actual);
        
        // Denormalize for display
        denormalize_point(&pred_weather, norm_params);
        denormalize_point(&actual_weather, norm_params);
        
        printf("\nPredicted weather:\n");
        print_weather_point(&pred_weather);
        printf("Actual weather:\n");
        print_weather_point(&actual_weather);
        
        matrix_free(predicted);
    }
    
    // Save the trained model
    printf("\nSaving model to %s...\n", model_file);
    if (save_lstm_model(network, model_file) == 0) {
        printf("Model saved successfully\n");
    } else {
        printf("Error: Could not save model\n");
    }
    
    // Clean up
    free_training_data(training_data);
    weather_dataset_free(dataset);
    lstm_network_free(network);
    
    printf("\nTraining completed successfully!\n");
    return 0;
}
