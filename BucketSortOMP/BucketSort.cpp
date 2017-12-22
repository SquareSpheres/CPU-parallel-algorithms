#include "BucketSort.h"
#include <algorithm>

void cpua::sequential::BucketSort(int * const arr, const int size, const int numberOfBuckets, const int intLimit)
{

	
	int *buckets = new int[size];
	int *bucketCounter = new int[size] {0};
	int *bucketPrefixSum = new int[size] {0};

	const int bucketRange = intLimit / numberOfBuckets != 0 ? intLimit / numberOfBuckets : 1;

	// Increment buckets
	for (int i = 0; i < size; i++) {
		int index = arr[i] / bucketRange;

		if (index >= numberOfBuckets)
			index = numberOfBuckets - 1;

		bucketCounter[index]++;
		bucketPrefixSum[index]++;
	}

	// Calculate prefix sum
	for (int i = 1; i < numberOfBuckets; i++) {
		bucketPrefixSum[i] += bucketPrefixSum[i - 1];
	}

	// Copy elements to their respective buckets
	for (int i = 0; i < size; i++) {
		int index = arr[i] / bucketRange;

		if (index >= numberOfBuckets)
			index = numberOfBuckets - 1;

		buckets[--bucketPrefixSum[index]] = arr[i];
	}

	// Sort buckets
	int prefixSum = 0;
	for (size_t i = 0; i <= numberOfBuckets; i++) {

		if (i == numberOfBuckets) {
			std::sort(buckets + prefixSum, (buckets + prefixSum) + size - prefixSum);
		}
		else {
			std::sort(buckets + prefixSum, (buckets + prefixSum) + bucketCounter[i]);
		}

		prefixSum += bucketCounter[i];
	}

	// Copy elements back to input array
	for (size_t i = 0; i < size; i++) {
		arr[i] = buckets[i];
	}

	// Clean up
	delete[] buckets;
	delete[] bucketPrefixSum;
	delete[] bucketCounter;

}
