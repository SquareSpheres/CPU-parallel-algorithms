#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <limits.h>
#include <stdlib.h>


void printArray(int * const arr, const int size);
int isSorted(int *const arr, const int size);
void bucketSortP(int * const arr, const int size);
void bucketSortS(int * const arr, int * const auxiliaryArr, const int size, const int numberOfBuckets);


int cmpfunc(const void * a, const void * b)
{
	return (*(int*)a - *(int*)b);
}


#define NUM_ELEMENTS 30000000
#define LIMIT 100000

int main(int argc, char *argv[]) {

	int * A = malloc(NUM_ELEMENTS * sizeof(int));

	srand(time(NULL));

	for (size_t i = 0; i < NUM_ELEMENTS; i++)
	{
		A[i] = rand() % LIMIT;
	}


	// Time start
	double timeStart = omp_get_wtime();
	bucketSortP(A, NUM_ELEMENTS);
	double endTime = omp_get_wtime() - timeStart;
	// Time end


	printf("size = %d	time = %f\n", NUM_ELEMENTS, endTime);

	if (!isSorted(A, NUM_ELEMENTS)) {
		printf("Array is not sorted!\n");
	}

	if (NUM_ELEMENTS < 40) {
		printArray(A, NUM_ELEMENTS);
	}

	free(A);


#ifdef _WIN32
	//Pause execution
	int n;
	printf("done... ");
	scanf("%d", &n);
#endif

}





/**
Prints an array
**/
void printArray(int * const arr, const int size)
{
	for (size_t i = 0; i < size; i++) {
		printf("%d ", arr[i]);
	}
	printf("\n");
}

/**
Check if a list is sorted
**/
int isSorted(int * const arr, const int size)
{
	for (size_t i = 1; i < size; i++)
	{
		if (arr[i - 1] > arr[i])
			return 0;
	}
	return 1;
}



// Parallel bucket sort
void bucketSortP(int * const arr, const int size)
{

	int numThreads;
	int numElementsPerThread;

	// The array that should be partinoned into buckets
	int * buckets = malloc(size * sizeof(int));

	// Prefix sum for global buckets. I.e. prefix sum for |numThreads| buckets
	int * globalBucketPrefix;

	// Prefix sum for local buckets. Each thread has |numThreads| local buckets.
	// Remains unchanged throughout process lifetime.
	int ** bucketPrefixConst;

	// Prefix sum for local buckets.Each thread has | numThreads | local buckets
	int ** bucketPrefix;




#pragma omp parallel // START OF PARALLEL
	{

		//Memory allocation
#pragma omp master
		{
			numThreads = omp_get_num_threads();
			numElementsPerThread = size / numThreads;


			globalBucketPrefix = calloc(numThreads, sizeof(int));

			bucketPrefixConst = malloc(numThreads * sizeof(int*));
			bucketPrefix = malloc(numThreads * sizeof(int*));

			for (size_t i = 0; i < numThreads; i++)
			{
				bucketPrefixConst[i] = calloc(numThreads, sizeof(int));
				bucketPrefix[i] = calloc(numThreads, sizeof(int));
			}
		}

#pragma omp barrier

		int threadNum = omp_get_thread_num();

		// Split up array in equal parts
		int arrStart = threadNum * numElementsPerThread;
		int arrEnd = (threadNum + 1) * numElementsPerThread;

		// Last thread takes the leftovers, if any
		if (threadNum == numThreads - 1) {
			arrEnd = size;
		}


		int numberOfLocalBuckets = numThreads;


		// To avoid division by zero, bucketRange is set to 1 if numberOfBuckets is larger then the range of numbers.
		int bucketRange = LIMIT / numberOfLocalBuckets != 0 ? LIMIT / numberOfLocalBuckets : 1;


		// Count numbers and increment buckets
		for (size_t i = arrStart; i < arrEnd; i++)
		{
			int index = arr[i] / bucketRange;

			if (index >= numberOfLocalBuckets)
				index = numberOfLocalBuckets - 1;


			bucketPrefixConst[threadNum][index]++;
			bucketPrefix[threadNum][index]++;

		}


		// Calculate prefix sums for local buckets
		for (size_t i = 1; i < numberOfLocalBuckets; i++)
		{
			bucketPrefix[threadNum][i] += bucketPrefix[threadNum][i - 1];
			bucketPrefixConst[threadNum][i] += bucketPrefixConst[threadNum][i - 1];
		}


		// Put numbers in respective local buckets
		for (size_t i = arrStart; i < arrEnd; i++)
		{
			int index = arr[i] / bucketRange;

			if (index >= numberOfLocalBuckets)
				index = numberOfLocalBuckets - 1;

			//place in local bucket
			buckets[arrStart + (--bucketPrefix[threadNum][index])] = arr[i];

		}

#pragma omp barrier

		//Count number of elements in global bucket
		for (size_t i = 0; i < numThreads; i++)
		{
			int threadArrStart = i * numElementsPerThread;

			int prefixSumForLocalBucket;
			if (threadNum == 0) {
				prefixSumForLocalBucket = 0;
			}
			else {
				prefixSumForLocalBucket = bucketPrefixConst[i][threadNum - 1];
			}

			int localBucketStart = threadArrStart + prefixSumForLocalBucket;

			int numberOfElementsInLocalBucket = bucketPrefixConst[i][threadNum] - prefixSumForLocalBucket;

			globalBucketPrefix[threadNum] += numberOfElementsInLocalBucket;

		}
#pragma omp barrier
#pragma omp master
		{
			// Calculate prefix sums for global buckets
			for (size_t i = 1; i < numThreads; i++)
			{
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
		for (size_t i = 0; i < numThreads; i++)
		{
			int threadArrStart = i * numElementsPerThread;

			int prefixSumForLocalBucket;
			if (threadNum == 0) {
				prefixSumForLocalBucket = 0;
			}
			else {
				prefixSumForLocalBucket = bucketPrefixConst[i][threadNum - 1];
			}


			int localBucketStart = threadArrStart + prefixSumForLocalBucket;

			int numberOfElementsInLocalBucket = bucketPrefixConst[i][threadNum] - prefixSumForLocalBucket;

			int localBucketEnd = localBucketStart + numberOfElementsInLocalBucket;

			// Put numbers in respective global buckets
			for (size_t i = localBucketStart; i < localBucketEnd; i++)
			{
				arr[globalBucketIndex] = buckets[i];
				globalBucketIndex++;
			}
		}



		int sortStart = 0;
		int sortEnd = globalBucketPrefix[threadNum];

		if (threadNum != 0) {
			sortStart = globalBucketPrefix[threadNum - 1];
		}


		int * bucketSortStart = &(arr[sortStart]);
		int * auxSortStart = &(buckets[sortStart]);
		int sizeSort = sortEnd - sortStart;

		//barrier only necessary if using bucketSort. 
#pragma omp barrier

		// Sort global arrays using sequential bucket sort. Avoid using extra memory by passing the unused array buckets as argument.
		// Have to make sure every thread is finished working on the array before preceding, hence the barrier above.
		bucketSortS(bucketSortStart, auxSortStart, sizeSort, LIMIT / numThreads);

		//Alternative sorting.
		//qsort(&(arr[sortStart]), (sortEnd - sortStart), sizeof(int), cmpfunc);

	} // END OF PARALLEL


	  //free up memory
	free(buckets);
	free(globalBucketPrefix);

	for (size_t i = 0; i < numThreads; i++)
	{
		free(bucketPrefix[i]);
		free(bucketPrefixConst[i]);
	}

	free(bucketPrefixConst);
	free(bucketPrefix);

}

/**
Sequential bucket sort.
**/
void bucketSortS(int * const arr, int * const auxiliaryArr, const int size, const int numberOfBuckets)
{

	int * bucketCounter = calloc(numberOfBuckets, sizeof(int));
	int * bucketPrefixSum = calloc(numberOfBuckets, sizeof(int));

	int bucketRange = LIMIT / numberOfBuckets != 0 ? LIMIT / numberOfBuckets : 1;


	for (size_t i = 0; i < size; i++)
	{
		int index = arr[i] / bucketRange;

		if (index >= numberOfBuckets)
			index = numberOfBuckets - 1;


		bucketCounter[index]++;
		bucketPrefixSum[index]++;
	}



	for (size_t i = 1; i < numberOfBuckets; i++)
	{
		bucketPrefixSum[i] += bucketPrefixSum[i - 1];
	}


	for (size_t i = 0; i < size; i++)
	{
		int index = arr[i] / bucketRange;

		if (index >= numberOfBuckets)
			index = numberOfBuckets - 1;

		auxiliaryArr[--bucketPrefixSum[index]] = arr[i];
	}




	int prefixSum = 0;
	for (size_t i = 0; i <= numberOfBuckets; i++)
	{

		if (i == numberOfBuckets) {
			qsort(&(auxiliaryArr[prefixSum]), size - prefixSum, sizeof(int), cmpfunc);
		}
		else {
			qsort(&(auxiliaryArr[prefixSum]), bucketCounter[i], sizeof(int), cmpfunc);
		}

		prefixSum += bucketCounter[i];

	}


	for (size_t i = 0; i < size; i++)
	{
		arr[i] = auxiliaryArr[i];
	}

	// clean up
	free(bucketPrefixSum);
	free(bucketCounter);

}
