#include "../include/weather_data.h"

// Create weather dataset
WeatherDataset* weather_dataset_create(int initial_capacity) {
    WeatherDataset* dataset = malloc(sizeof(WeatherDataset));
    if (!dataset) return NULL;
    
    dataset->data = malloc(initial_capacity * sizeof(WeatherPoint));
    if (!dataset->data) {
        free(dataset);
        return NULL;
    }
    
    dataset->size = 0;
    dataset->capacity = initial_capacity;
    return dataset;
}

// Free weather dataset
void weather_dataset_free(WeatherDataset* dataset) {
    if (!dataset) return;
    
    free(dataset->data);
    free(dataset);
}

// Add weather point to dataset
int weather_dataset_add(WeatherDataset* dataset, WeatherPoint point) {
    if (!dataset) return -1;
    
    // Resize if needed
    if (dataset->size >= dataset->capacity) {
        int new_capacity = dataset->capacity * 2;
        WeatherPoint* new_data = realloc(dataset->data, new_capacity * sizeof(WeatherPoint));
        if (!new_data) return -1;
        
        dataset->data = new_data;
        dataset->capacity = new_capacity;
    }
    
    dataset->data[dataset->size] = point;
    dataset->size++;
    return 0;
}

// Load weather data from CSV
int weather_load_csv(const char* filename, WeatherDataset* dataset) {
    if (!filename || !dataset) return -1;
    
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open file %s\n", filename);
        return -1;
    }
    
    char line[512];  // Increased buffer size for timestamp columns
    int line_count = 0;
    int has_timestamps = 0;
    
    // Read and analyze header line
    if (fgets(line, sizeof(line), file)) {
        line_count++;
        // Check if header contains timestamp columns
        if (strstr(line, "timestamp") != NULL || strstr(line, "unix_timestamp") != NULL) {
            has_timestamps = 1;
            printf("Detected CSV format with timestamps\n");
        } else {
            printf("Detected legacy CSV format without timestamps\n");
        }
    }
    
    while (fgets(line, sizeof(line), file)) {
        line_count++;
        WeatherPoint point;
        int parsed = 0;
        
        if (has_timestamps) {
            // New format: timestamp,unix_timestamp,temp,pressure,humidity,wind_speed,wind_dir,precipitation
            // We skip the first two columns (timestamp and unix_timestamp)
            char* token = strtok(line, ",");
            if (token) token = strtok(NULL, ",");  // Skip timestamp
            if (token) token = strtok(NULL, ",");  // Skip unix_timestamp (now pointing to temperature)
            
            // Parse weather data (token already points to temperature)
            if (token) {
                point.temperature = atof(token);
                if ((token = strtok(NULL, ","))) {
                    point.pressure = atof(token);
                    if ((token = strtok(NULL, ","))) {
                        point.humidity = atof(token);
                        if ((token = strtok(NULL, ","))) {
                            point.wind_speed = atof(token);
                            if ((token = strtok(NULL, ","))) {
                                point.wind_direction = atof(token);
                                if ((token = strtok(NULL, ",\n"))) {
                                    point.precipitation = atof(token);
                                    parsed = 6;
                                }
                            }
                        }
                    }
                }
            }
        } else {
            // Legacy format: temp,pressure,humidity,wind_speed,wind_dir,precipitation
            parsed = sscanf(line, "%lf,%lf,%lf,%lf,%lf,%lf",
                           &point.temperature,
                           &point.pressure,
                           &point.humidity,
                           &point.wind_speed,
                           &point.wind_direction,
                           &point.precipitation);
        }
        
        if (parsed == 6) {
            if (weather_dataset_add(dataset, point) != 0) {
                printf("Error: Failed to add data point at line %d\n", line_count);
                fclose(file);
                return -1;
            }
        } else {
            printf("Warning: Invalid data format at line %d (parsed %d fields)\n", line_count, parsed);
        }
    }
    
    fclose(file);
    printf("Loaded %d weather data points from %s\n", dataset->size, filename);
    return 0;
}

// Save weather data to CSV
int weather_save_csv(const char* filename, WeatherDataset* dataset) {
    if (!filename || !dataset) return -1;
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Error: Could not create file %s\n", filename);
        return -1;
    }
    
    // Write header
    fprintf(file, "temperature,pressure,humidity,wind_speed,wind_direction,precipitation\n");
    
    // Write data
    for (int i = 0; i < dataset->size; i++) {
        WeatherPoint* point = &dataset->data[i];
        fprintf(file, "%.2f,%.2f,%.2f,%.2f,%.2f,%.4f\n",
                point->temperature,
                point->pressure,
                point->humidity,
                point->wind_speed,
                point->wind_direction,
                point->precipitation);
    }
    
    fclose(file);
    printf("Saved %d weather data points to %s\n", dataset->size, filename);
    return 0;
}

// Calculate normalization parameters
NormalizationParams* calculate_normalization_params(WeatherDataset* dataset) {
    if (!dataset || dataset->size == 0) return NULL;
    
    NormalizationParams* params = malloc(sizeof(NormalizationParams));
    if (!params) return NULL;
    
    // Initialize min/max with first data point
    WeatherPoint* first = &dataset->data[0];
    params->temp_min = params->temp_max = first->temperature;
    params->pressure_min = params->pressure_max = first->pressure;
    params->humidity_min = params->humidity_max = first->humidity;
    params->wind_speed_min = params->wind_speed_max = first->wind_speed;
    params->wind_dir_min = params->wind_dir_max = first->wind_direction;
    params->precip_min = params->precip_max = first->precipitation;
    
    // Find actual min/max values
    for (int i = 1; i < dataset->size; i++) {
        WeatherPoint* point = &dataset->data[i];
        
        if (point->temperature < params->temp_min) params->temp_min = point->temperature;
        if (point->temperature > params->temp_max) params->temp_max = point->temperature;
        
        if (point->pressure < params->pressure_min) params->pressure_min = point->pressure;
        if (point->pressure > params->pressure_max) params->pressure_max = point->pressure;
        
        if (point->humidity < params->humidity_min) params->humidity_min = point->humidity;
        if (point->humidity > params->humidity_max) params->humidity_max = point->humidity;
        
        if (point->wind_speed < params->wind_speed_min) params->wind_speed_min = point->wind_speed;
        if (point->wind_speed > params->wind_speed_max) params->wind_speed_max = point->wind_speed;
        
        if (point->wind_direction < params->wind_dir_min) params->wind_dir_min = point->wind_direction;
        if (point->wind_direction > params->wind_dir_max) params->wind_dir_max = point->wind_direction;
        
        if (point->precipitation < params->precip_min) params->precip_min = point->precipitation;
        if (point->precipitation > params->precip_max) params->precip_max = point->precipitation;
    }
    
    return params;
}

// Normalize dataset
void normalize_dataset(WeatherDataset* dataset, NormalizationParams* params) {
    if (!dataset || !params) return;
    
    for (int i = 0; i < dataset->size; i++) {
        WeatherPoint* point = &dataset->data[i];
        
        // Normalize to [0, 1] range, handle cases where min == max
        double temp_range = params->temp_max - params->temp_min;
        double pressure_range = params->pressure_max - params->pressure_min;
        double humidity_range = params->humidity_max - params->humidity_min;
        double wind_speed_range = params->wind_speed_max - params->wind_speed_min;
        double wind_dir_range = params->wind_dir_max - params->wind_dir_min;
        double precip_range = params->precip_max - params->precip_min;
        
        point->temperature = (temp_range > 0) ? (point->temperature - params->temp_min) / temp_range : 0.5;
        point->pressure = (pressure_range > 0) ? (point->pressure - params->pressure_min) / pressure_range : 0.5;
        point->humidity = (humidity_range > 0) ? (point->humidity - params->humidity_min) / humidity_range : 0.5;
        point->wind_speed = (wind_speed_range > 0) ? (point->wind_speed - params->wind_speed_min) / wind_speed_range : 0.5;
        point->wind_direction = (wind_dir_range > 0) ? (point->wind_direction - params->wind_dir_min) / wind_dir_range : 0.5;
        point->precipitation = (precip_range > 0) ? (point->precipitation - params->precip_min) / precip_range : 0.5;
    }
}

// Denormalize weather point
void denormalize_point(WeatherPoint* point, NormalizationParams* params) {
    if (!point || !params) return;
    
    double temp_range = params->temp_max - params->temp_min;
    double pressure_range = params->pressure_max - params->pressure_min;
    double humidity_range = params->humidity_max - params->humidity_min;
    double wind_speed_range = params->wind_speed_max - params->wind_speed_min;
    double wind_dir_range = params->wind_dir_max - params->wind_dir_min;
    double precip_range = params->precip_max - params->precip_min;
    
    point->temperature = (temp_range > 0) ? point->temperature * temp_range + params->temp_min : params->temp_min;
    point->pressure = (pressure_range > 0) ? point->pressure * pressure_range + params->pressure_min : params->pressure_min;
    point->humidity = (humidity_range > 0) ? point->humidity * humidity_range + params->humidity_min : params->humidity_min;
    point->wind_speed = (wind_speed_range > 0) ? point->wind_speed * wind_speed_range + params->wind_speed_min : params->wind_speed_min;
    point->wind_direction = (wind_dir_range > 0) ? point->wind_direction * wind_dir_range + params->wind_dir_min : params->wind_dir_min;
    point->precipitation = (precip_range > 0) ? point->precipitation * precip_range + params->precip_min : params->precip_min;
}

// Convert weather point to matrix
Matrix* weather_point_to_matrix(WeatherPoint* point) {
    if (!point) return NULL;
    
    Matrix* m = matrix_create(6, 1);
    if (!m) return NULL;
    
    m->data[0][0] = point->temperature;
    m->data[1][0] = point->pressure;
    m->data[2][0] = point->humidity;
    m->data[3][0] = point->wind_speed;
    m->data[4][0] = point->wind_direction;
    m->data[5][0] = point->precipitation;
    
    return m;
}

// Convert matrix to weather point
WeatherPoint matrix_to_weather_point(Matrix* m) {
    WeatherPoint point = {0};
    
    if (!m || m->rows != 6 || m->cols != 1) {
        return point;
    }
    
    point.temperature = m->data[0][0];
    point.pressure = m->data[1][0];
    point.humidity = m->data[2][0];
    point.wind_speed = m->data[3][0];
    point.wind_direction = m->data[4][0];
    point.precipitation = m->data[5][0];
    
    return point;
}

// Split dataset into training and testing
void split_dataset(WeatherDataset* dataset, WeatherDataset* train, WeatherDataset* test, double train_ratio) {
    if (!dataset || !train || !test || train_ratio < 0.0 || train_ratio > 1.0) {
        return;
    }
    
    int train_size = (int)(dataset->size * train_ratio);
    
    // Add training data
    for (int i = 0; i < train_size; i++) {
        weather_dataset_add(train, dataset->data[i]);
    }
    
    // Add test data
    for (int i = train_size; i < dataset->size; i++) {
        weather_dataset_add(test, dataset->data[i]);
    }
}

// Print weather point
void print_weather_point(WeatherPoint* point) {
    if (!point) {
        printf("NULL weather point\n");
        return;
    }
    
    printf("Temperature: %.2f°F, Pressure: %.2f inHg, Humidity: %.2f%%, "
           "Wind: %.2f mph @ %.0f°, Precipitation: %.4f in\n",
           point->temperature, point->pressure, point->humidity,
           point->wind_speed, point->wind_direction, point->precipitation);
}

// Print normalization parameters
void print_normalization_params(NormalizationParams* params) {
    if (!params) {
        printf("NULL normalization parameters\n");
        return;
    }
    
    printf("Normalization Parameters:\n");
    printf("  Temperature: [%.2f, %.2f]\n", params->temp_min, params->temp_max);
    printf("  Pressure: [%.2f, %.2f]\n", params->pressure_min, params->pressure_max);
    printf("  Humidity: [%.2f, %.2f]\n", params->humidity_min, params->humidity_max);
    printf("  Wind Speed: [%.2f, %.2f]\n", params->wind_speed_min, params->wind_speed_max);
    printf("  Wind Direction: [%.2f, %.2f]\n", params->wind_dir_min, params->wind_dir_max);
    printf("  Precipitation: [%.4f, %.4f]\n", params->precip_min, params->precip_max);
}
