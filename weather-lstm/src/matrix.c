#include "../include/matrix.h"

// Create a new matrix
Matrix* matrix_create(int rows, int cols) {
    Matrix* m = malloc(sizeof(Matrix));
    if (!m) return NULL;
    
    m->rows = rows;
    m->cols = cols;
    m->data = malloc(rows * sizeof(double*));
    if (!m->data) {
        free(m);
        return NULL;
    }
    
    for (int i = 0; i < rows; i++) {
        m->data[i] = calloc(cols, sizeof(double));
        if (!m->data[i]) {
            // Clean up on failure
            for (int j = 0; j < i; j++) {
                free(m->data[j]);
            }
            free(m->data);
            free(m);
            return NULL;
        }
    }
    
    return m;
}

// Free matrix memory
void matrix_free(Matrix* m) {
    if (!m) return;
    
    for (int i = 0; i < m->rows; i++) {
        free(m->data[i]);
    }
    free(m->data);
    free(m);
}

// Set all elements to zero
void matrix_zero(Matrix* m) {
    if (!m) return;
    
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            m->data[i][j] = 0.0;
        }
    }
}

// Fill matrix with random values
void matrix_random(Matrix* m, double min, double max) {
    if (!m) return;
    
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            double random = (double)rand() / RAND_MAX;
            m->data[i][j] = min + random * (max - min);
        }
    }
}

// Copy matrix
void matrix_copy(Matrix* dest, Matrix* src) {
    if (!dest || !src || dest->rows != src->rows || dest->cols != src->cols) {
        return;
    }
    
    for (int i = 0; i < src->rows; i++) {
        for (int j = 0; j < src->cols; j++) {
            dest->data[i][j] = src->data[i][j];
        }
    }
}

// Matrix multiplication
Matrix* matrix_multiply(Matrix* a, Matrix* b) {
    if (!a || !b || a->cols != b->rows) {
        return NULL;
    }
    
    Matrix* result = matrix_create(a->rows, b->cols);
    if (!result) return NULL;
    
    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < b->cols; j++) {
            double sum = 0.0;
            for (int k = 0; k < a->cols; k++) {
                sum += a->data[i][k] * b->data[k][j];
            }
            result->data[i][j] = sum;
        }
    }
    
    return result;
}

// Matrix addition
Matrix* matrix_add(Matrix* a, Matrix* b) {
    if (!a || !b || a->rows != b->rows || a->cols != b->cols) {
        return NULL;
    }
    
    Matrix* result = matrix_create(a->rows, a->cols);
    if (!result) return NULL;
    
    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < a->cols; j++) {
            result->data[i][j] = a->data[i][j] + b->data[i][j];
        }
    }
    
    return result;
}

// Matrix subtraction
Matrix* matrix_subtract(Matrix* a, Matrix* b) {
    if (!a || !b || a->rows != b->rows || a->cols != b->cols) {
        return NULL;
    }
    
    Matrix* result = matrix_create(a->rows, a->cols);
    if (!result) return NULL;
    
    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < a->cols; j++) {
            result->data[i][j] = a->data[i][j] - b->data[i][j];
        }
    }
    
    return result;
}

// Matrix transpose
Matrix* matrix_transpose(Matrix* m) {
    if (!m) return NULL;
    
    Matrix* result = matrix_create(m->cols, m->rows);
    if (!result) return NULL;
    
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            result->data[j][i] = m->data[i][j];
        }
    }
    
    return result;
}

// Scale matrix by scalar
void matrix_scale(Matrix* m, double scalar) {
    if (!m) return;
    
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            m->data[i][j] *= scalar;
        }
    }
}

// Print matrix
void matrix_print(Matrix* m) {
    if (!m) {
        printf("NULL matrix\n");
        return;
    }
    
    printf("Matrix %dx%d:\n", m->rows, m->cols);
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            printf("%8.4f ", m->data[i][j]);
        }
        printf("\n");
    }
}

// Get element
double matrix_get(Matrix* m, int row, int col) {
    if (!m || row < 0 || row >= m->rows || col < 0 || col >= m->cols) {
        return 0.0;
    }
    return m->data[row][col];
}

// Set element
void matrix_set(Matrix* m, int row, int col, double value) {
    if (!m || row < 0 || row >= m->rows || col < 0 || col >= m->cols) {
        return;
    }
    m->data[row][col] = value;
}

// Activation functions
double sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x));
}

double tanh_activation(double x) {
    return tanh(x);
}

double relu(double x) {
    return x > 0.0 ? x : 0.0;
}

// Apply activation functions to entire matrix
void apply_sigmoid(Matrix* m) {
    if (!m) return;
    
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            m->data[i][j] = sigmoid(m->data[i][j]);
        }
    }
}

void apply_tanh(Matrix* m) {
    if (!m) return;
    
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            m->data[i][j] = tanh_activation(m->data[i][j]);
        }
    }
}

void apply_relu(Matrix* m) {
    if (!m) return;
    
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            m->data[i][j] = relu(m->data[i][j]);
        }
    }
}

// Derivative functions
double sigmoid_derivative(double x) {
    double s = sigmoid(x);
    return s * (1.0 - s);
}

double tanh_derivative(double x) {
    double t = tanh_activation(x);
    return 1.0 - t * t;
}

double relu_derivative(double x) {
    return x > 0.0 ? 1.0 : 0.0;
}
