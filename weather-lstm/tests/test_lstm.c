#include "../include/lstm.h"
#include "../include/weather_data.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

// Test matrix operations
void test_matrix_operations() {
    printf("Testing matrix operations...\n");
    
    // Test matrix creation
    Matrix* m1 = matrix_create(2, 3);
    Matrix* m2 = matrix_create(3, 2);
    assert(m1 != NULL);
    assert(m2 != NULL);
    assert(m1->rows == 2 && m1->cols == 3);
    assert(m2->rows == 3 && m2->cols == 2);
    
    // Test matrix initialization
    matrix_set(m1, 0, 0, 1.0);
    matrix_set(m1, 0, 1, 2.0);
    matrix_set(m1, 0, 2, 3.0);
    matrix_set(m1, 1, 0, 4.0);
    matrix_set(m1, 1, 1, 5.0);
    matrix_set(m1, 1, 2, 6.0);
    
    assert(matrix_get(m1, 0, 0) == 1.0);
    assert(matrix_get(m1, 1, 2) == 6.0);
    
    // Test matrix multiplication
    matrix_set(m2, 0, 0, 1.0);
    matrix_set(m2, 1, 0, 2.0);
    matrix_set(m2, 2, 0, 3.0);
    matrix_set(m2, 0, 1, 4.0);
    matrix_set(m2, 1, 1, 5.0);
    matrix_set(m2, 2, 1, 6.0);
    
    Matrix* result = matrix_multiply(m1, m2);
    assert(result != NULL);
    assert(result->rows == 2 && result->cols == 2);
    
    // Expected result: [1*1+2*2+3*3, 1*4+2*5+3*6] = [14, 32]
    //                  [4*1+5*2+6*3, 4*4+5*5+6*6] = [32, 77]
    assert(fabs(matrix_get(result, 0, 0) - 14.0) < 1e-10);
    assert(fabs(matrix_get(result, 0, 1) - 32.0) < 1e-10);
    assert(fabs(matrix_get(result, 1, 0) - 32.0) < 1e-10);
    assert(fabs(matrix_get(result, 1, 1) - 77.0) < 1e-10);
    
    matrix_free(m1);
    matrix_free(m2);
    matrix_free(result);
    
    printf("Matrix operations tests passed!\n");
}

// Test weather data operations
void test_weather_data() {
    printf("Testing weather data operations...\n");
    
    // Test dataset creation
    WeatherDataset* dataset = weather_dataset_create(10);
    assert(dataset != NULL);
    assert(dataset->size == 0);
    assert(dataset->capacity == 10);
    
    // Test adding weather points
    WeatherPoint point1 = {45.0, 30.0, 60.0, 8.0, 180.0, 0.0};
    WeatherPoint point2 = {47.0, 29.9, 65.0, 10.0, 175.0, 0.1};
    
    assert(weather_dataset_add(dataset, point1) == 0);
    assert(weather_dataset_add(dataset, point2) == 0);
    assert(dataset->size == 2);
    
    // Test weather point to matrix conversion
    Matrix* m = weather_point_to_matrix(&point1);
    assert(m != NULL);
    assert(m->rows == 6 && m->cols == 1);
    assert(matrix_get(m, 0, 0) == 45.0);
    assert(matrix_get(m, 1, 0) == 30.0);
    assert(matrix_get(m, 5, 0) == 0.0);
    
    // Test matrix to weather point conversion
    WeatherPoint converted = matrix_to_weather_point(m);
    assert(fabs(converted.temperature - 45.0) < 1e-10);
    assert(fabs(converted.pressure - 30.0) < 1e-10);
    assert(fabs(converted.precipitation - 0.0) < 1e-10);
    
    matrix_free(m);
    weather_dataset_free(dataset);
    
    printf("Weather data operations tests passed!\n");
}

// Test LSTM cell creation and basic operations
void test_lstm_cell() {
    printf("Testing LSTM cell operations...\n");
    
    // Test LSTM cell creation
    LSTMCell* cell = lstm_cell_create(6, 32);
    assert(cell != NULL);
    assert(cell->input_size == 6);
    assert(cell->hidden_size == 32);
    
    // Test that all matrices are properly allocated
    assert(cell->W_f != NULL);
    assert(cell->W_i != NULL);
    assert(cell->W_c != NULL);
    assert(cell->W_o != NULL);
    assert(cell->U_f != NULL);
    assert(cell->U_i != NULL);
    assert(cell->U_c != NULL);
    assert(cell->U_o != NULL);
    assert(cell->b_f != NULL);
    assert(cell->b_i != NULL);
    assert(cell->b_c != NULL);
    assert(cell->b_o != NULL);
    assert(cell->cell_state != NULL);
    assert(cell->hidden_state != NULL);
    
    // Test forward pass with dummy input
    Matrix* input = matrix_create(6, 1);
    for (int i = 0; i < 6; i++) {
        matrix_set(input, i, 0, 0.5); // Set all inputs to 0.5
    }
    
    Matrix* output = lstm_cell_forward(cell, input);
    assert(output != NULL);
    assert(output->rows == 32 && output->cols == 1);
    
    // Check that output values are reasonable (between -1 and 1 due to tanh)
    for (int i = 0; i < 32; i++) {
        double val = matrix_get(output, i, 0);
        assert(val >= -1.1 && val <= 1.1); // Allow small margin for numerical precision
    }
    
    matrix_free(input);
    matrix_free(output);
    lstm_cell_free(cell);
    
    printf("LSTM cell operations tests passed!\n");
}

// Test LSTM network
void test_lstm_network() {
    printf("Testing LSTM network operations...\n");
    
    // Test network creation
    LSTMNetwork* network = lstm_network_create(6, 16, 6);
    assert(network != NULL);
    assert(network->input_size == 6);
    assert(network->hidden_size == 16);
    assert(network->output_size == 6);
    
    // Test prediction with dummy sequence
    int seq_length = 3;
    Matrix** sequence = malloc(seq_length * sizeof(Matrix*));
    
    for (int t = 0; t < seq_length; t++) {
        sequence[t] = matrix_create(6, 1);
        for (int i = 0; i < 6; i++) {
            matrix_set(sequence[t], i, 0, 0.5 + 0.1 * t); // Slightly different values
        }
    }
    
    Matrix* prediction = lstm_network_predict(network, sequence, seq_length);
    assert(prediction != NULL);
    assert(prediction->rows == 6 && prediction->cols == 1);
    
    // Clean up
    for (int t = 0; t < seq_length; t++) {
        matrix_free(sequence[t]);
    }
    free(sequence);
    matrix_free(prediction);
    lstm_network_free(network);
    
    printf("LSTM network operations tests passed!\n");
}

// Test training data creation
void test_training_data() {
    printf("Testing training data creation...\n");
    
    // Create a small dataset
    WeatherDataset* dataset = weather_dataset_create(10);
    
    // Add some test data points
    for (int i = 0; i < 8; i++) {
        WeatherPoint point = {
            45.0 + i, // temperature
            30.0,     // pressure
            60.0,     // humidity
            8.0,      // wind_speed
            180.0,    // wind_direction
            0.0       // precipitation
        };
        weather_dataset_add(dataset, point);
    }
    
    // Create training data with sequence length 3
    TrainingData* training_data = create_training_data(dataset, 3);
    assert(training_data != NULL);
    assert(training_data->sequence_length == 3);
    assert(training_data->num_sequences == 5); // 8 - 3 = 5 sequences
    
    // Check first sequence
    assert(training_data->inputs[0] != NULL);
    assert(training_data->targets[0] != NULL);
    
    // Check that input sequence has correct values
    Matrix* first_input = training_data->inputs[0][0];
    assert(matrix_get(first_input, 0, 0) == 45.0); // First temperature
    
    Matrix* first_target = training_data->targets[0];
    assert(matrix_get(first_target, 0, 0) == 48.0); // Fourth temperature (45 + 3)
    
    free_training_data(training_data);
    weather_dataset_free(dataset);
    
    printf("Training data creation tests passed!\n");
}

int main() {
    printf("Running Weather LSTM Tests\n");
    printf("==========================\n\n");
    
    test_matrix_operations();
    test_weather_data();
    test_lstm_cell();
    test_lstm_network();
    test_training_data();
    
    printf("\n==========================\n");
    printf("All tests passed! ✅\n");
    
    return 0;
}
