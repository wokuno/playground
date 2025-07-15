import random
import time
from collections import Counter
try:
    import numpy as np
    HAS_NUMPY = True
except ImportError:
    HAS_NUMPY = False

# This script generates two identical lists of integers, 
# then removes one random element from the second list.
# It then finds the missing element using using different methods
# Current Methods:
# 1. XOR method (original)
# 2. XOR method (optimized)
# 3. XOR method (using functools.reduce)
# 4. XOR method (NumPy vectorized)
# 5. XOR method (NumPy combined arrays)
# 6. Loop method with Counter
# 7. Set difference method (using Counter)
# 8. Sum difference method

def time_function(func, *args):
    start_time = time.time()
    result = func(*args)
    end_time = time.time()
    return result, end_time - start_time

# function to find the missing element using xor
def find_missing_xor_element(a, b):
    xor_a = 0
    xor_b = 0
    
    for num in a:
        xor_a ^= num
        
    for num in b:
        xor_b ^= num
        
    return xor_a ^ xor_b

# Optimized XOR method using single loop and built-in functions
def find_missing_xor_element_optimized(a, b):
    # XOR all elements in a, then XOR all elements in b
    # The result will be the missing element
    result = 0
    for num in a:
        result ^= num
    for num in b:
        result ^= num
    return result

# Even more optimized XOR using functools.reduce
def find_missing_xor_element_reduce(a, b):
    from functools import reduce
    import operator
    return reduce(operator.xor, a, 0) ^ reduce(operator.xor, b, 0)

# Vectorized XOR using NumPy
def find_missing_xor_element_numpy(a, b):
    # Convert to numpy arrays and use vectorized XOR operations
    arr_a = np.array(a, dtype=np.int32)
    arr_b = np.array(b, dtype=np.int32)
    
    # XOR all elements in each array, then XOR the results
    return np.bitwise_xor.reduce(arr_a) ^ np.bitwise_xor.reduce(arr_b)

# Ultra-fast vectorized XOR using NumPy with combined arrays
def find_missing_xor_element_numpy_combined(a, b):
    # Combine both arrays and XOR all elements
    # Since elements in both arrays cancel out, only the missing element remains
    combined = np.concatenate([np.array(a, dtype=np.int32), np.array(b, dtype=np.int32)])
    return np.bitwise_xor.reduce(combined)

# func to find the missing element with loops
def find_missing_element(a, b):
    count_a = Counter(a)
    count_b = Counter(b)
    for num in count_a:
        if count_a[num] != count_b.get(num, 0):
            return num
    return None

# function to find the missing element using set difference
def find_missing_set_element(a, b):
    # Handle case where elements might be duplicated
    # Convert to multisets using Counter, then find difference
    from collections import Counter
    count_a = Counter(a)
    count_b = Counter(b)
    
    for element, count in count_a.items():
        if count_b[element] < count:
            return element
    return None

# Alternative faster set-based method for when we know there's exactly one missing element
def find_missing_sum_element(a, b):
    return sum(a) - sum(b)

if __name__ == "__main__":

    # Test with different list sizes
    sizes = [2**x for x in range(1, 16)]  # Sizes: 2 -> 4 -> 8 -> ... -> 2^31
    test_functions = {
        "xor": find_missing_xor_element,
        "xor_opt": find_missing_xor_element_optimized,
        "xor_reduce": find_missing_xor_element_reduce,
        "xor_numpy": find_missing_xor_element_numpy,
        "xor_numpy_combined": find_missing_xor_element_numpy_combined,
        "loop": find_missing_element,
        "set": find_missing_set_element,
        "sum": find_missing_sum_element
    }

    if not HAS_NUMPY:
        print("NumPy is not available. Skipping NumPy-based tests.")
        del test_functions["xor_numpy"]
        del test_functions["xor_numpy_combined"]
    
    for size in sizes:
        print(f"\n=== Testing with list size: {size} ===")
        
        # random list of integers
        a = [random.randint(0, size) for _ in range(size)]

        # random same as list a, but with 1 random element removed
        b = a.copy()
        b.remove(random.choice(b))

        # Run multiple times and average
        time_results = {
            'xor': [],
            'xor_opt': [],
            'xor_reduce': [],
            'xor_numpy': [],
            'xor_numpy_combined': [],
            'loop': [],
            'set': [],
            'sum': []
        }

        results = []
        
        for _ in range(10):  # Run 10 times for better average
            for key, func in test_functions.items():
                result, elapsed = time_function(func, a, b)
                time_results[key].append(elapsed)
                results.append(result)

        # Calculate averages only for available functions
        avg_times = {}
        for key in test_functions.keys():
            if time_results[key]:  # Only calculate if we have results
                avg_times[key] = sum(time_results[key]) / len(time_results[key])

        print(f"Average time using XOR: {avg_times['xor']:.8f} seconds")
        print(f"Average time using XOR (optimized): {avg_times['xor_opt']:.8f} seconds")
        print(f"Average time using XOR (reduce): {avg_times['xor_reduce']:.8f} seconds")
        if 'xor_numpy' in avg_times:
            print(f"Average time using XOR (NumPy): {avg_times['xor_numpy']:.8f} seconds")
        if 'xor_numpy_combined' in avg_times:
            print(f"Average time using XOR (NumPy combined): {avg_times['xor_numpy_combined']:.8f} seconds")
        print(f"Average time using Loop: {avg_times['loop']:.8f} seconds")
        print(f"Average time using Set: {avg_times['set']:.8f} seconds")
        print(f"Average time using Sum: {avg_times['sum']:.8f} seconds")

        # Find the fastest method
        times = {
            'XOR': avg_times['xor'],
            'XOR (optimized)': avg_times['xor_opt'],
            'XOR (reduce)': avg_times['xor_reduce'],
            'Loop': avg_times['loop'],
            'Set': avg_times['set'],
            'Sum': avg_times['sum']
        }
        
        # Add NumPy methods only if they were tested
        if 'xor_numpy' in avg_times:
            times['XOR (NumPy)'] = avg_times['xor_numpy']
        if 'xor_numpy_combined' in avg_times:
            times['XOR (NumPy combined)'] = avg_times['xor_numpy_combined']

        fastest_method = min(times, key=times.get)
        print(f"Fastest method: {fastest_method} ({times[fastest_method]:.8f} seconds)")

        # Check for discrepancies
        if len(set(results)) > 1:
            print("Discrepancy found in results:")
            for func, result in zip(test_functions, results):
                print(f"{func[1]}: {result}")
        else:
            print("All methods returned the same result:", results[0])

