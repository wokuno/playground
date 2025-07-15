#ifndef LSTM_H
#define LSTM_H

#include "matrix.h"
#include "weather_data.h"

// LSTM cell structure
typedef struct {
    // Input dimensions
    int input_size;
    int hidden_size;
    
    // Weight matrices
    Matrix* W_f;  // Forget gate weights
    Matrix* W_i;  // Input gate weights
    Matrix* W_c;  // Candidate gate weights
    Matrix* W_o;  // Output gate weights
    
    // Recurrent weight matrices
    Matrix* U_f;  // Forget gate recurrent weights
    Matrix* U_i;  // Input gate recurrent weights
    Matrix* U_c;  // Candidate gate recurrent weights
    Matrix* U_o;  // Output gate recurrent weights
    
    // Bias vectors
    Matrix* b_f;  // Forget gate bias
    Matrix* b_i;  // Input gate bias
    Matrix* b_c;  // Candidate gate bias
    Matrix* b_o;  // Output gate bias
    
    // Cell state and hidden state
    Matrix* cell_state;
    Matrix* hidden_state;
    
    // Intermediate computations (for backprop)
    Matrix* forget_gate;
    Matrix* input_gate;
    Matrix* candidate_gate;
    Matrix* output_gate;
    
} LSTMCell;

// LSTM network structure
typedef struct {
    LSTMCell* lstm_layer;
    Matrix* W_output;      // Output layer weights
    Matrix* b_output;      // Output layer bias
    
    int input_size;
    int hidden_size;
    int output_size;
    
    // Training parameters
    double learning_rate;
    int sequence_length;
    
    // Normalization parameters
    NormalizationParams* norm_params;
    
} LSTMNetwork;

// Training data structure
typedef struct {
    Matrix*** inputs;    // Array of input sequences (each sequence is Matrix**)
    Matrix** targets;    // Array of target outputs
    int num_sequences;
    int sequence_length;
} TrainingData;

// Function declarations

// LSTM Cell operations
LSTMCell* lstm_cell_create(int input_size, int hidden_size);
void lstm_cell_free(LSTMCell* cell);
void lstm_cell_reset_state(LSTMCell* cell);
Matrix* lstm_cell_forward(LSTMCell* cell, Matrix* input);

// LSTM Network operations
LSTMNetwork* lstm_network_create(int input_size, int hidden_size, int output_size);
void lstm_network_free(LSTMNetwork* network);
Matrix* lstm_network_predict(LSTMNetwork* network, Matrix** sequence, int seq_length);
void lstm_network_reset(LSTMNetwork* network);

// Training
TrainingData* create_training_data(WeatherDataset* dataset, int sequence_length);
void free_training_data(TrainingData* data);
void lstm_train(LSTMNetwork* network, TrainingData* data, int epochs);
double calculate_loss(Matrix* predicted, Matrix* target);

// Model persistence
int save_lstm_model(LSTMNetwork* network, const char* filename);
LSTMNetwork* load_lstm_model(const char* filename);

// Utility functions
void initialize_weights(Matrix* m, double scale);
Matrix* create_sequence_input(WeatherDataset* dataset, int start_idx, int seq_length);
WeatherPoint lstm_predict_next(LSTMNetwork* network, WeatherDataset* recent_data, int seq_length);

#endif // LSTM_H
