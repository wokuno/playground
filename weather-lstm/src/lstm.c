#include "../include/lstm.h"
#include <time.h>

// Initialize weights with Xavier initialization
void initialize_weights(Matrix* m, double scale) {
    if (!m) return;
    
    double limit = sqrt(6.0 / (m->rows + m->cols)) * scale;
    matrix_random(m, -limit, limit);
}

// Create LSTM cell
LSTMCell* lstm_cell_create(int input_size, int hidden_size) {
    LSTMCell* cell = malloc(sizeof(LSTMCell));
    if (!cell) return NULL;
    
    cell->input_size = input_size;
    cell->hidden_size = hidden_size;
    
    // Initialize weight matrices
    cell->W_f = matrix_create(hidden_size, input_size);
    cell->W_i = matrix_create(hidden_size, input_size);
    cell->W_c = matrix_create(hidden_size, input_size);
    cell->W_o = matrix_create(hidden_size, input_size);
    
    cell->U_f = matrix_create(hidden_size, hidden_size);
    cell->U_i = matrix_create(hidden_size, hidden_size);
    cell->U_c = matrix_create(hidden_size, hidden_size);
    cell->U_o = matrix_create(hidden_size, hidden_size);
    
    // Initialize bias vectors
    cell->b_f = matrix_create(hidden_size, 1);
    cell->b_i = matrix_create(hidden_size, 1);
    cell->b_c = matrix_create(hidden_size, 1);
    cell->b_o = matrix_create(hidden_size, 1);
    
    // Initialize states
    cell->cell_state = matrix_create(hidden_size, 1);
    cell->hidden_state = matrix_create(hidden_size, 1);
    
    // Initialize intermediate matrices
    cell->forget_gate = matrix_create(hidden_size, 1);
    cell->input_gate = matrix_create(hidden_size, 1);
    cell->candidate_gate = matrix_create(hidden_size, 1);
    cell->output_gate = matrix_create(hidden_size, 1);
    
    // Check if all allocations succeeded
    if (!cell->W_f || !cell->W_i || !cell->W_c || !cell->W_o ||
        !cell->U_f || !cell->U_i || !cell->U_c || !cell->U_o ||
        !cell->b_f || !cell->b_i || !cell->b_c || !cell->b_o ||
        !cell->cell_state || !cell->hidden_state ||
        !cell->forget_gate || !cell->input_gate || !cell->candidate_gate || !cell->output_gate) {
        lstm_cell_free(cell);
        return NULL;
    }
    
    // Initialize weights
    initialize_weights(cell->W_f, 1.0);
    initialize_weights(cell->W_i, 1.0);
    initialize_weights(cell->W_c, 1.0);
    initialize_weights(cell->W_o, 1.0);
    
    initialize_weights(cell->U_f, 1.0);
    initialize_weights(cell->U_i, 1.0);
    initialize_weights(cell->U_c, 1.0);
    initialize_weights(cell->U_o, 1.0);
    
    // Initialize biases (forget gate bias to 1.0 for better gradient flow)
    matrix_zero(cell->b_f);
    for (int i = 0; i < hidden_size; i++) {
        cell->b_f->data[i][0] = 1.0;
    }
    matrix_zero(cell->b_i);
    matrix_zero(cell->b_c);
    matrix_zero(cell->b_o);
    
    // Initialize states to zero
    matrix_zero(cell->cell_state);
    matrix_zero(cell->hidden_state);
    
    return cell;
}

// Free LSTM cell
void lstm_cell_free(LSTMCell* cell) {
    if (!cell) return;
    
    matrix_free(cell->W_f);
    matrix_free(cell->W_i);
    matrix_free(cell->W_c);
    matrix_free(cell->W_o);
    
    matrix_free(cell->U_f);
    matrix_free(cell->U_i);
    matrix_free(cell->U_c);
    matrix_free(cell->U_o);
    
    matrix_free(cell->b_f);
    matrix_free(cell->b_i);
    matrix_free(cell->b_c);
    matrix_free(cell->b_o);
    
    matrix_free(cell->cell_state);
    matrix_free(cell->hidden_state);
    
    matrix_free(cell->forget_gate);
    matrix_free(cell->input_gate);
    matrix_free(cell->candidate_gate);
    matrix_free(cell->output_gate);
    
    free(cell);
}

// Reset LSTM cell state
void lstm_cell_reset_state(LSTMCell* cell) {
    if (!cell) return;
    
    matrix_zero(cell->cell_state);
    matrix_zero(cell->hidden_state);
}

// LSTM cell forward pass
Matrix* lstm_cell_forward(LSTMCell* cell, Matrix* input) {
    if (!cell || !input) return NULL;
    
    // Temporary matrices for computations
    Matrix* W_x_f = matrix_multiply(cell->W_f, input);
    Matrix* U_h_f = matrix_multiply(cell->U_f, cell->hidden_state);
    Matrix* f_temp = matrix_add(W_x_f, U_h_f);
    Matrix* f_temp2 = matrix_add(f_temp, cell->b_f);
    matrix_copy(cell->forget_gate, f_temp2);
    apply_sigmoid(cell->forget_gate);
    
    Matrix* W_x_i = matrix_multiply(cell->W_i, input);
    Matrix* U_h_i = matrix_multiply(cell->U_i, cell->hidden_state);
    Matrix* i_temp = matrix_add(W_x_i, U_h_i);
    Matrix* i_temp2 = matrix_add(i_temp, cell->b_i);
    matrix_copy(cell->input_gate, i_temp2);
    apply_sigmoid(cell->input_gate);
    
    Matrix* W_x_c = matrix_multiply(cell->W_c, input);
    Matrix* U_h_c = matrix_multiply(cell->U_c, cell->hidden_state);
    Matrix* c_temp = matrix_add(W_x_c, U_h_c);
    Matrix* c_temp2 = matrix_add(c_temp, cell->b_c);
    matrix_copy(cell->candidate_gate, c_temp2);
    apply_tanh(cell->candidate_gate);
    
    Matrix* W_x_o = matrix_multiply(cell->W_o, input);
    Matrix* U_h_o = matrix_multiply(cell->U_o, cell->hidden_state);
    Matrix* o_temp = matrix_add(W_x_o, U_h_o);
    Matrix* o_temp2 = matrix_add(o_temp, cell->b_o);
    matrix_copy(cell->output_gate, o_temp2);
    apply_sigmoid(cell->output_gate);
    
    // Update cell state: C_t = f_t * C_{t-1} + i_t * tilde{C_t}
    Matrix* forget_cell = matrix_create(cell->hidden_size, 1);
    Matrix* input_candidate = matrix_create(cell->hidden_size, 1);
    
    for (int i = 0; i < cell->hidden_size; i++) {
        forget_cell->data[i][0] = cell->forget_gate->data[i][0] * cell->cell_state->data[i][0];
        input_candidate->data[i][0] = cell->input_gate->data[i][0] * cell->candidate_gate->data[i][0];
        cell->cell_state->data[i][0] = forget_cell->data[i][0] + input_candidate->data[i][0];
    }
    
    // Update hidden state: h_t = o_t * tanh(C_t)
    Matrix* cell_tanh = matrix_create(cell->hidden_size, 1);
    matrix_copy(cell_tanh, cell->cell_state);
    apply_tanh(cell_tanh);
    
    for (int i = 0; i < cell->hidden_size; i++) {
        cell->hidden_state->data[i][0] = cell->output_gate->data[i][0] * cell_tanh->data[i][0];
    }
    
    // Clean up temporary matrices
    matrix_free(W_x_f); matrix_free(U_h_f); matrix_free(f_temp); matrix_free(f_temp2);
    matrix_free(W_x_i); matrix_free(U_h_i); matrix_free(i_temp); matrix_free(i_temp2);
    matrix_free(W_x_c); matrix_free(U_h_c); matrix_free(c_temp); matrix_free(c_temp2);
    matrix_free(W_x_o); matrix_free(U_h_o); matrix_free(o_temp); matrix_free(o_temp2);
    matrix_free(forget_cell); matrix_free(input_candidate); matrix_free(cell_tanh);
    
    // Return copy of hidden state
    Matrix* output = matrix_create(cell->hidden_size, 1);
    matrix_copy(output, cell->hidden_state);
    return output;
}

// Create LSTM network
LSTMNetwork* lstm_network_create(int input_size, int hidden_size, int output_size) {
    LSTMNetwork* network = malloc(sizeof(LSTMNetwork));
    if (!network) return NULL;
    
    network->lstm_layer = lstm_cell_create(input_size, hidden_size);
    if (!network->lstm_layer) {
        free(network);
        return NULL;
    }
    
    network->W_output = matrix_create(output_size, hidden_size);
    network->b_output = matrix_create(output_size, 1);
    
    if (!network->W_output || !network->b_output) {
        lstm_cell_free(network->lstm_layer);
        matrix_free(network->W_output);
        matrix_free(network->b_output);
        free(network);
        return NULL;
    }
    
    network->input_size = input_size;
    network->hidden_size = hidden_size;
    network->output_size = output_size;
    network->learning_rate = 0.001;
    network->sequence_length = 10;
    network->norm_params = NULL;
    
    // Initialize output weights
    initialize_weights(network->W_output, 1.0);
    matrix_zero(network->b_output);
    
    return network;
}

// Free LSTM network
void lstm_network_free(LSTMNetwork* network) {
    if (!network) return;
    
    lstm_cell_free(network->lstm_layer);
    matrix_free(network->W_output);
    matrix_free(network->b_output);
    
    if (network->norm_params) {
        free(network->norm_params);
    }
    
    free(network);
}

// Reset network state
void lstm_network_reset(LSTMNetwork* network) {
    if (!network) return;
    
    lstm_cell_reset_state(network->lstm_layer);
}

// Network prediction
Matrix* lstm_network_predict(LSTMNetwork* network, Matrix** sequence, int seq_length) {
    if (!network || !sequence) return NULL;
    
    lstm_network_reset(network);
    
    // Process sequence
    Matrix* hidden_output = NULL;
    for (int t = 0; t < seq_length; t++) {
        if (hidden_output) {
            matrix_free(hidden_output);
        }
        hidden_output = lstm_cell_forward(network->lstm_layer, sequence[t]);
    }
    
    if (!hidden_output) return NULL;
    
    // Generate output
    Matrix* output_temp = matrix_multiply(network->W_output, hidden_output);
    Matrix* output = matrix_add(output_temp, network->b_output);
    
    matrix_free(hidden_output);
    matrix_free(output_temp);
    
    return output;
}

// Create training data from weather dataset
TrainingData* create_training_data(WeatherDataset* dataset, int sequence_length) {
    if (!dataset || sequence_length <= 0 || dataset->size <= sequence_length) {
        return NULL;
    }
    
    TrainingData* data = malloc(sizeof(TrainingData));
    if (!data) return NULL;
    
    data->num_sequences = dataset->size - sequence_length;
    data->sequence_length = sequence_length;
    
    data->inputs = malloc(data->num_sequences * sizeof(Matrix**));
    data->targets = malloc(data->num_sequences * sizeof(Matrix*));
    
    if (!data->inputs || !data->targets) {
        free(data->inputs);
        free(data->targets);
        free(data);
        return NULL;
    }
    
    // Create sequences
    for (int i = 0; i < data->num_sequences; i++) {
        data->inputs[i] = malloc(sequence_length * sizeof(Matrix*));
        if (!data->inputs[i]) {
            // Clean up on failure
            for (int j = 0; j < i; j++) {
                for (int k = 0; k < sequence_length; k++) {
                    matrix_free(data->inputs[j][k]);
                }
                free(data->inputs[j]);
            }
            free(data->inputs);
            free(data->targets);
            free(data);
            return NULL;
        }
        
        // Create input sequence
        for (int t = 0; t < sequence_length; t++) {
            data->inputs[i][t] = weather_point_to_matrix(&dataset->data[i + t]);
        }
        
        // Create target (next weather point)
        data->targets[i] = weather_point_to_matrix(&dataset->data[i + sequence_length]);
    }
    
    return data;
}

// Free training data
void free_training_data(TrainingData* data) {
    if (!data) return;
    
    for (int i = 0; i < data->num_sequences; i++) {
        for (int t = 0; t < data->sequence_length; t++) {
            matrix_free(data->inputs[i][t]);
        }
        free(data->inputs[i]);
        matrix_free(data->targets[i]);
    }
    
    free(data->inputs);
    free(data->targets);
    free(data);
}

// Calculate mean squared error loss
double calculate_loss(Matrix* predicted, Matrix* target) {
    if (!predicted || !target || predicted->rows != target->rows || predicted->cols != target->cols) {
        return -1.0;
    }
    
    double loss = 0.0;
    int total_elements = predicted->rows * predicted->cols;
    
    for (int i = 0; i < predicted->rows; i++) {
        for (int j = 0; j < predicted->cols; j++) {
            double diff = predicted->data[i][j] - target->data[i][j];
            loss += diff * diff;
        }
    }
    
    return loss / total_elements;
}

// Simple training function (simplified backpropagation)
void lstm_train(LSTMNetwork* network, TrainingData* data, int epochs) {
    if (!network || !data) return;
    
    printf("Starting training for %d epochs...\n", epochs);
    
    for (int epoch = 0; epoch < epochs; epoch++) {
        double total_loss = 0.0;
        
        for (int seq = 0; seq < data->num_sequences; seq++) {
            // Forward pass
            Matrix* prediction = lstm_network_predict(network, data->inputs[seq], data->sequence_length);
            if (!prediction) continue;
            
            // Calculate loss
            double loss = calculate_loss(prediction, data->targets[seq]);
            total_loss += loss;
            
            // Simple gradient update (simplified for demonstration)
            // In a full implementation, you would compute gradients through backpropagation
            Matrix* error = matrix_subtract(data->targets[seq], prediction);
            matrix_scale(error, network->learning_rate);
            
            // Update output weights (simplified)
            Matrix* hidden_transpose = matrix_transpose(network->lstm_layer->hidden_state);
            Matrix* weight_update = matrix_multiply(error, hidden_transpose);
            Matrix* new_weights = matrix_add(network->W_output, weight_update);
            matrix_copy(network->W_output, new_weights);
            
            matrix_free(prediction);
            matrix_free(error);
            matrix_free(hidden_transpose);
            matrix_free(weight_update);
            matrix_free(new_weights);
        }
        
        double avg_loss = total_loss / data->num_sequences;
        if (epoch % 10 == 0) {
            printf("Epoch %d: Average Loss = %.6f\n", epoch + 1, avg_loss);
        }
    }
    
    printf("Training completed.\n");
}

// Save model to file
int save_lstm_model(LSTMNetwork* network, const char* filename) {
    if (!network || !filename) return -1;
    
    FILE* file = fopen(filename, "wb");
    if (!file) return -1;
    
    // Write network dimensions
    fwrite(&network->input_size, sizeof(int), 1, file);
    fwrite(&network->hidden_size, sizeof(int), 1, file);
    fwrite(&network->output_size, sizeof(int), 1, file);
    fwrite(&network->learning_rate, sizeof(double), 1, file);
    fwrite(&network->sequence_length, sizeof(int), 1, file);
    
    // Write LSTM weights (simplified - would need full implementation)
    // For demonstration, just write output layer
    for (int i = 0; i < network->output_size; i++) {
        fwrite(network->W_output->data[i], sizeof(double), network->hidden_size, file);
    }
    
    for (int i = 0; i < network->output_size; i++) {
        fwrite(&network->b_output->data[i][0], sizeof(double), 1, file);
    }
    
    // Write normalization parameters if available
    int has_norm = (network->norm_params != NULL);
    fwrite(&has_norm, sizeof(int), 1, file);
    if (has_norm) {
        fwrite(network->norm_params, sizeof(NormalizationParams), 1, file);
    }
    
    fclose(file);
    return 0;
}

// Load model from file
LSTMNetwork* load_lstm_model(const char* filename) {
    if (!filename) return NULL;
    
    FILE* file = fopen(filename, "rb");
    if (!file) return NULL;
    
    int input_size, hidden_size, output_size, sequence_length;
    double learning_rate;
    
    // Read network dimensions
    if (fread(&input_size, sizeof(int), 1, file) != 1 ||
        fread(&hidden_size, sizeof(int), 1, file) != 1 ||
        fread(&output_size, sizeof(int), 1, file) != 1 ||
        fread(&learning_rate, sizeof(double), 1, file) != 1 ||
        fread(&sequence_length, sizeof(int), 1, file) != 1) {
        fclose(file);
        return NULL;
    }
    
    LSTMNetwork* network = lstm_network_create(input_size, hidden_size, output_size);
    if (!network) {
        fclose(file);
        return NULL;
    }
    
    network->learning_rate = learning_rate;
    network->sequence_length = sequence_length;
    
    // Read weights (simplified)
    for (int i = 0; i < output_size; i++) {
        if (fread(network->W_output->data[i], sizeof(double), (size_t)hidden_size, file) != (size_t)hidden_size) {
            printf("Error reading output weights\n");
            lstm_network_free(network);
            fclose(file);
            return NULL;
        }
    }
    
    for (int i = 0; i < output_size; i++) {
        if (fread(&network->b_output->data[i][0], sizeof(double), 1, file) != 1) {
            printf("Error reading output bias\n");
            lstm_network_free(network);
            fclose(file);
            return NULL;
        }
    }
    
    // Read normalization parameters
    int has_norm;
    if (fread(&has_norm, sizeof(int), 1, file) == 1 && has_norm) {
        network->norm_params = malloc(sizeof(NormalizationParams));
        if (fread(network->norm_params, sizeof(NormalizationParams), 1, file) != 1) {
            printf("Error reading normalization parameters\n");
            free(network->norm_params);
            network->norm_params = NULL;
            lstm_network_free(network);
            fclose(file);
            return NULL;
        }
    }
    
    fclose(file);
    return network;
}

// Predict next weather point
WeatherPoint lstm_predict_next(LSTMNetwork* network, WeatherDataset* recent_data, int seq_length) {
    WeatherPoint result = {0};
    
    if (!network || !recent_data || recent_data->size < seq_length) {
        return result;
    }
    
    // Create input sequence from most recent data
    Matrix** sequence = malloc(seq_length * sizeof(Matrix*));
    if (!sequence) return result;
    
    int start_idx = recent_data->size - seq_length;
    for (int i = 0; i < seq_length; i++) {
        sequence[i] = weather_point_to_matrix(&recent_data->data[start_idx + i]);
    }
    
    // Make prediction
    Matrix* prediction = lstm_network_predict(network, sequence, seq_length);
    if (prediction) {
        result = matrix_to_weather_point(prediction);
        matrix_free(prediction);
    }
    
    // Clean up
    for (int i = 0; i < seq_length; i++) {
        matrix_free(sequence[i]);
    }
    free(sequence);
    
    return result;
}
