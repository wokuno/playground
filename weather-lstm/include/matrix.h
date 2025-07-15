#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Matrix structure
typedef struct {
    double **data;
    int rows;
    int cols;
} Matrix;

// Matrix operations
Matrix* matrix_create(int rows, int cols);
void matrix_free(Matrix* m);
void matrix_zero(Matrix* m);
void matrix_random(Matrix* m, double min, double max);
void matrix_copy(Matrix* dest, Matrix* src);
Matrix* matrix_multiply(Matrix* a, Matrix* b);
Matrix* matrix_add(Matrix* a, Matrix* b);
Matrix* matrix_subtract(Matrix* a, Matrix* b);
Matrix* matrix_transpose(Matrix* m);
void matrix_scale(Matrix* m, double scalar);
void matrix_print(Matrix* m);

// Element access
double matrix_get(Matrix* m, int row, int col);
void matrix_set(Matrix* m, int row, int col, double value);

// Activation functions
double sigmoid(double x);
double tanh_activation(double x);
double relu(double x);
void apply_sigmoid(Matrix* m);
void apply_tanh(Matrix* m);
void apply_relu(Matrix* m);

// Derivatives
double sigmoid_derivative(double x);
double tanh_derivative(double x);
double relu_derivative(double x);

#endif // MATRIX_H
