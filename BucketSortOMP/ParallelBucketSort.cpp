#include "ParallelBucketSort.h"
#include <omp.h>
#include <algorithm>

void cpua::parallel::BucketSort(int * const arr, const int size, const int numberOfBuckets, const int intLimit)
{

	int numThreads;
	int numElementsPerThread;

	// The array that should be partinoned into buckets
	int *buckets = new int[size];

	// Prefix sum for global buckets. I.e. prefix sum for |numThreads| buckets
	int *globalBucketPrefix;

	// Prefix sum for local buckets. Each thread has |numThreads| local buckets.
	// Remains unchanged throughout process lifetime.
	int **bucketPrefixConst;

	// Prefix sum for local buckets.Each thread has | numThreads | local buckets
	int **bucketPrefix;

	// PARALLEL START
#pragma omp parallel
	{

		// Memory allocation
#pragma omp master
		{
			numThreads = omp_get_num_threads();
			numElementsPerThread = size / numThreads;

			globalBucketPrefix = new int[numThreads] {0};

			bucketPrefixConst = new int*[numThreads];
			bucketPrefix = new int*[numThreads];

			for (int i = 0; i < numThreads; ++i) {
				bucketPrefixConst[i] = new int[numThreads] {0};
				bucketPrefix[i] = new int[numThreads] {0};
			}
		}
#pragma omp barrier

		const int threadNum = omp_get_thread_num();

		// Split up array in equal parts
		int arrStart = threadNum * numElementsPerThread;
		int arrEnd = (threadNum + 1) * numElementsPerThread;

		// Last thread takes the leftovers, if any
		if (threadNum == numThreads - 1) {
			arrEnd = size;
		}

		const int numberOfLocalBuckets = numThreads;

		// To avoid division by zero, bucketRange is set to 1 if numberOfBuckets is
		// larger then the range of numbers.
		const int bucketRange =
			intLimit / numberOfLocalBuckets != 0 ? intLimit / numberOfLocalBuckets : 1;

		// Count numbers and increment buckets
		for (int i = arrStart; i < arrEnd; ++i) {
			int index = arr[i] / bucketRange;

			if (index >= numberOfLocalBuckets)
				index = numberOfLocalBuckets - 1;

			bucketPrefixConst[threadNum][index]++;
			bucketPrefix[threadNum][index]++;
		}

		// Calculate prefix sums for local buckets
		for (int i = 1; i < numberOfLocalBuckets; ++i) {
			bucketPrefix[threadNum][i] += bucketPrefix[threadNum][i - 1];
			bucketPrefixConst[threadNum][i] += bucketPrefixConst[threadNum][i - 1];
		}

		// Put numbers in respective local buckets
		for (int i = arrStart; i < arrEnd; ++i) {
			int index = arr[i] / bucketRange;

			if (index >= numberOfLocalBuckets)
				index = numberOfLocalBuckets - 1;

			// place in local bucket
			buckets[arrStart + (--bucketPrefix[threadNum][index])] = arr[i];
		}

#pragma omp barrier

		// Count number of elements in global bucket
		for (int i = 0; i < numThreads; ++i) {

			int prefixSumForLocalBucket;
			if (threadNum == 0) {
				prefixSumForLocalBucket = 0;
			}
			else {
				prefixSumForLocalBucket = bucketPrefixConst[i][threadNum - 1];
			}

			const int numberOfElementsInLocalBucket =
				bucketPrefixConst[i][threadNum] - prefixSumForLocalBucket;

			globalBucketPrefix[threadNum] += numberOfElementsInLocalBucket;
		}

#pragma omp barrier
#pragma omp master
		{
			// Calculate prefix sums for global buckets
			// TODO this could be done in parallel
			for (int i = 1; i < numThreads; ++i) {
				globalBucketPrefix[i] += globalBucketPrefix[i - 1];
			}
		}

#pragma omp barrier

		// Find where global buckets starts
		int globalBucketIndex;
		if (threadNum == 0) {
			globalBucketIndex = 0;
		}
		else {
			globalBucketIndex = globalBucketPrefix[threadNum - 1];
		}

		// Iterate through each thread and get elements from local bucket
		for (int i = 0; i < numThreads; ++i) {
			const int threadArrStart = i * numElementsPerThread;

			int prefixSumForLocalBucket;
			if (threadNum == 0) {
				prefixSumForLocalBucket = 0;
			}
			else {
				prefixSumForLocalBucket = bucketPrefixConst[i][threadNum - 1];
			}

			const int localBucketStart = threadArrStart + prefixSumForLocalBucket;

			const int numberOfElementsInLocalBucket =
				bucketPrefixConst[i][threadNum] - prefixSumForLocalBucket;

			const int localBucketEnd = localBucketStart + numberOfElementsInLocalBucket;

			// Put numbers in respective global buckets
			for (int j = localBucketStart; j < localBucketEnd; j++) {
				arr[globalBucketIndex] = buckets[j];
				globalBucketIndex++;
			}
		}

		int sortStart = 0;
		int sortEnd = globalBucketPrefix[threadNum];

		if (threadNum != 0) {
			sortStart = globalBucketPrefix[threadNum - 1];
		}

		int *bucketSortStart = &(arr[sortStart]);
		int *auxSortStart = &(buckets[sortStart]);
		int sizeSort = sortEnd - sortStart;

		// barrier only necessary if using bucketSort.
#pragma omp barrier

		// TODO include sequential bucketsort and uncomment code

		// Sort global arrays using sequential bucket sort. Avoid using extra memory
		// by passing the unused array buckets as argument. Have to make sure every
		// thread is finished working on the array before preceding, hence the
		// barrier above.
		//bucketSortS(bucketSortStart, auxSortStart, sizeSort, LIMIT / numThreads);

		// Alternative sorting.
		// qsort(&(arr[sortStart]), (sortEnd - sortStart), sizeof(int), cmpfunc);
		std::sort(arr + sortStart, arr + sortStart + (sortEnd - sortStart));

	} // PARALLEL END


	// free up memory
	delete[] buckets;
	delete[] globalBucketPrefix;

	for (int i = 0; i < numThreads; ++i)
	{
		delete[] bucketPrefixConst[i];
		delete[] bucketPrefix[i];
	}

	delete[] bucketPrefixConst;
	delete[] bucketPrefix;

}
