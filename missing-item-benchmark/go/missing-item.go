package main

import (
	"fmt"
	"math/rand"
	"time"
)

// timeFunction measures the execution time of a function
func timeFunction(fn func([]int, []int) int, a, b []int) (int, time.Duration) {
	start := time.Now()
	result := fn(a, b)
	elapsed := time.Since(start)
	return result, elapsed
}

// findMissingXOROriginal - XOR method using separate loops
func findMissingXOROriginal(a, b []int) int {
	xorA := 0
	xorB := 0

	for _, num := range a {
		xorA ^= num
	}

	for _, num := range b {
		xorB ^= num
	}

	return xorA ^ xorB
}

// findMissingXOROptimized - XOR method using single result variable
func findMissingXOROptimized(a, b []int) int {
	result := 0

	for _, num := range a {
		result ^= num
	}

	for _, num := range b {
		result ^= num
	}

	return result
}

// findMissingSum - Sum difference method
func findMissingSum(a, b []int) int {
	sumA := 0
	sumB := 0

	for _, num := range a {
		sumA += num
	}

	for _, num := range b {
		sumB += num
	}

	return sumA - sumB
}

// findMissingLinear - Linear search method using map
func findMissingLinear(a, b []int) int {
	// Count occurrences in both slices
	countA := make(map[int]int)
	countB := make(map[int]int)

	for _, num := range a {
		countA[num]++
	}

	for _, num := range b {
		countB[num]++
	}

	// Find the element with different counts
	for num, countInA := range countA {
		if countB[num] != countInA {
			return num
		}
	}

	return 0 // Should never reach here if input is valid
}

// findMissingSet - Set difference method using map lookup
func findMissingSet(a, b []int) int {
	// Create a set from b
	setB := make(map[int]int)
	for _, num := range b {
		setB[num]++
	}

	// Check each element in a
	countA := make(map[int]int)
	for _, num := range a {
		countA[num]++
	}

	for num, countInA := range countA {
		if setB[num] < countInA {
			return num
		}
	}

	return 0 // Should never reach here if input is valid
}

// generateTestData creates test arrays with one missing element
func generateTestData(size int) ([]int, []int) {
	// Create array a with random integers
	a := make([]int, size)
	for i := 0; i < size; i++ {
		a[i] = rand.Intn(size)
	}

	// Create array b as copy of a, then remove one random element
	b := make([]int, len(a))
	copy(b, a)

	// Remove a random element
	removeIndex := rand.Intn(len(b))
	b = append(b[:removeIndex], b[removeIndex+1:]...)

	return a, b
}

func main() {
	// Test with different list sizes
	sizes := []int{2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768}

	// Define test functions
	testFunctions := map[string]func([]int, []int) int{
		"xor_original":  findMissingXOROriginal,
		"xor_optimized": findMissingXOROptimized,
		"sum":           findMissingSum,
		"linear":        findMissingLinear,
		"set":           findMissingSet,
	}

	// Seed random number generator
	rand.Seed(time.Now().UnixNano())

	for _, size := range sizes {
		fmt.Printf("\n=== Testing with list size: %d ===\n", size)

		// Generate test data
		a, b := generateTestData(size)

		// Run multiple times and average
		timeResults := make(map[string][]time.Duration)
		for key := range testFunctions {
			timeResults[key] = make([]time.Duration, 0, 10)
		}

		var results []int

		// Run 10 times for better average
		for i := 0; i < 10; i++ {
			for key, fn := range testFunctions {
				result, elapsed := timeFunction(fn, a, b)
				timeResults[key] = append(timeResults[key], elapsed)
				if i == 0 { // Only store results from first run for comparison
					results = append(results, result)
				}
			}
		}

		// Calculate averages
		avgTimes := make(map[string]time.Duration)
		for key, times := range timeResults {
			var total time.Duration
			for _, t := range times {
				total += t
			}
			avgTimes[key] = total / time.Duration(len(times))
		}

		// Print results
		fmt.Printf("Average time using XOR (original): %v\n", avgTimes["xor_original"])
		fmt.Printf("Average time using XOR (optimized): %v\n", avgTimes["xor_optimized"])
		fmt.Printf("Average time using Sum: %v\n", avgTimes["sum"])
		fmt.Printf("Average time using Linear: %v\n", avgTimes["linear"])
		fmt.Printf("Average time using Set: %v\n", avgTimes["set"])

		// Find the fastest method
		var fastestMethod string
		var fastestTime time.Duration = time.Hour // Start with a very large time

		times := map[string]time.Duration{
			"XOR (original)":  avgTimes["xor_original"],
			"XOR (optimized)": avgTimes["xor_optimized"],
			"Sum":             avgTimes["sum"],
			"Linear":          avgTimes["linear"],
			"Set":             avgTimes["set"],
		}

		for method, t := range times {
			if t < fastestTime {
				fastestTime = t
				fastestMethod = method
			}
		}

		fmt.Printf("Fastest method: %s (%v)\n", fastestMethod, fastestTime)

		// Check for discrepancies - all results should be the same
		firstResult := results[0]
		allSame := true
		for _, result := range results[1:] {
			if result != firstResult {
				allSame = false
				break
			}
		}

		if !allSame {
			fmt.Println("Discrepancy found in results:")
			i := 0
			for key := range testFunctions {
				fmt.Printf("%s: %d\n", key, results[i])
				i++
			}
		} else {
			fmt.Printf("All methods returned the same result: %d\n", firstResult)
		}
	}
}
