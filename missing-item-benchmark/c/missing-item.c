#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Function to measure execution time
double time_function(int (*func)(int*, int, int*, int), int* a, int size_a, int* b, int size_b) {
    clock_t start = clock();
    func(a, size_a, b, size_b);  // We don't need the result for timing
    clock_t end = clock();
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    return elapsed;
}

// XOR method - original approach
int find_missing_xor_original(int* a, int size_a, int* b, int size_b) {
    int xor_a = 0;
    int xor_b = 0;
    
    for (int i = 0; i < size_a; i++) {
        xor_a ^= a[i];
    }
    
    for (int i = 0; i < size_b; i++) {
        xor_b ^= b[i];
    }
    
    return xor_a ^ xor_b;
}

// XOR method - optimized single loop
int find_missing_xor_optimized(int* a, int size_a, int* b, int size_b) {
    int result = 0;
    
    for (int i = 0; i < size_a; i++) {
        result ^= a[i];
    }
    
    for (int i = 0; i < size_b; i++) {
        result ^= b[i];
    }
    
    return result;
}

// Sum difference method
int find_missing_sum(int* a, int size_a, int* b, int size_b) {
    long long sum_a = 0;
    long long sum_b = 0;
    
    for (int i = 0; i < size_a; i++) {
        sum_a += a[i];
    }
    
    for (int i = 0; i < size_b; i++) {
        sum_b += b[i];
    }
    
    return (int)(sum_a - sum_b);
}

// Linear search method (naive approach)
int find_missing_linear(int* a, int size_a, int* b, int size_b) {
    for (int i = 0; i < size_a; i++) {
        int count_a = 0, count_b = 0;
        
        // Count occurrences in array a
        for (int j = 0; j < size_a; j++) {
            if (a[j] == a[i]) count_a++;
        }
        
        // Count occurrences in array b
        for (int j = 0; j < size_b; j++) {
            if (b[j] == a[i]) count_b++;
        }
        
        if (count_a != count_b) {
            return a[i];
        }
    }
    return -1; // Not found
}

// Utility function to copy array
void copy_array(int* src, int* dest, int size) {
    for (int i = 0; i < size; i++) {
        dest[i] = src[i];
    }
}

// Utility function to remove random element
int remove_random_element(int* array, int size) {
    int index = rand() % size;
    int removed = array[index];
    
    // Shift elements to remove the element at index
    for (int i = index; i < size - 1; i++) {
        array[i] = array[i + 1];
    }
    
    return removed;
}

int main() {
    srand(time(NULL));
    
    printf("XOR Benchmark - C Implementation\n");
    printf("=================================\n\n");
    
    // Test different sizes (powers of 2)
    int sizes[] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    // Function pointers for different algorithms
    int (*algorithms[])(int*, int, int*, int) = {
        find_missing_xor_original,
        find_missing_xor_optimized,
        find_missing_sum,
        find_missing_linear
    };
    
    const char* algorithm_names[] = {
        "XOR (original)",
        "XOR (optimized)", 
        "Sum difference",
        "Linear search"
    };
    
    int num_algorithms = sizeof(algorithms) / sizeof(algorithms[0]);
    
    for (int s = 0; s < num_sizes; s++) {
        int size = sizes[s];
        printf("=== Testing with array size: %d ===\n", size);
        
        // Create array with random integers
        int* a = malloc(size * sizeof(int));
        int* b = malloc((size - 1) * sizeof(int));
        
        for (int i = 0; i < size; i++) {
            a[i] = rand() % size;
        }
        
        // Copy array a to b, then remove one element
        copy_array(a, b, size);
        remove_random_element(b, size);
        int size_b = size - 1;
        
        // Run each algorithm multiple times and calculate average
        const int iterations = 10;
        double avg_times[num_algorithms];
        int results[num_algorithms];
        
        for (int alg = 0; alg < num_algorithms; alg++) {
            double total_time = 0.0;
            
            for (int iter = 0; iter < iterations; iter++) {
                double elapsed = time_function(algorithms[alg], a, size, b, size_b);
                total_time += elapsed;
            }
            
            avg_times[alg] = total_time / iterations;
            results[alg] = algorithms[alg](a, size, b, size_b);
            
            printf("Average time using %s: %.8f seconds\n", 
                   algorithm_names[alg], avg_times[alg]);
        }
        
        // Find fastest method
        int fastest_idx = 0;
        for (int i = 1; i < num_algorithms; i++) {
            if (avg_times[i] < avg_times[fastest_idx]) {
                fastest_idx = i;
            }
        }
        
        printf("Fastest method: %s (%.8f seconds)\n", 
               algorithm_names[fastest_idx], avg_times[fastest_idx]);
        
        // Check for result consistency
        int consistent = 1;
        for (int i = 1; i < num_algorithms; i++) {
            if (results[i] != results[0]) {
                consistent = 0;
                break;
            }
        }
        
        if (consistent) {
            printf("All methods returned the same result: %d\n", results[0]);
        } else {
            printf("Discrepancy found in results:\n");
            for (int i = 0; i < num_algorithms; i++) {
                printf("  %s: %d\n", algorithm_names[i], results[i]);
            }
        }
        
        printf("\n");
        
        free(a);
        free(b);
    }
    
    return 0;
}
