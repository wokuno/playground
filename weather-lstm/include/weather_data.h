#ifndef WEATHER_DATA_H
#define WEATHER_DATA_H

#include "matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Weather data structure
typedef struct {
    double temperature;     // Temperature in Fahrenheit
    double pressure;        // Atmospheric pressure in inHg
    double humidity;        // Humidity percentage
    double wind_speed;      // Wind speed in mph
    double wind_direction;  // Wind direction in degrees
    double precipitation;   // Precipitation in inches
} WeatherPoint;

// Dataset structure
typedef struct {
    WeatherPoint* data;
    int size;
    int capacity;
} WeatherDataset;

// Data normalization parameters
typedef struct {
    double temp_min, temp_max;
    double pressure_min, pressure_max;
    double humidity_min, humidity_max;
    double wind_speed_min, wind_speed_max;
    double wind_dir_min, wind_dir_max;
    double precip_min, precip_max;
} NormalizationParams;

// Function declarations
WeatherDataset* weather_dataset_create(int initial_capacity);
void weather_dataset_free(WeatherDataset* dataset);
int weather_dataset_add(WeatherDataset* dataset, WeatherPoint point);
int weather_load_csv(const char* filename, WeatherDataset* dataset);
int weather_save_csv(const char* filename, WeatherDataset* dataset);

// Data preprocessing
NormalizationParams* calculate_normalization_params(WeatherDataset* dataset);
void normalize_dataset(WeatherDataset* dataset, NormalizationParams* params);
void denormalize_point(WeatherPoint* point, NormalizationParams* params);

// Convert weather point to matrix
Matrix* weather_point_to_matrix(WeatherPoint* point);
WeatherPoint matrix_to_weather_point(Matrix* m);

// Data splitting
void split_dataset(WeatherDataset* dataset, WeatherDataset* train, WeatherDataset* test, double train_ratio);

// Utility functions
void print_weather_point(WeatherPoint* point);
void print_normalization_params(NormalizationParams* params);

#endif // WEATHER_DATA_H
