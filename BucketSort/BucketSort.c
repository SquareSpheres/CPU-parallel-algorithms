#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>


void printArray(int * const arr, const int size);
int isSorted(int *const arr, const int size);
void bucketSortS(int * const arr, const int size, const int numberOfBuckets);
int main(int argc, char * argv[]);
void randomizeArray(int * const arr, const int size);

int cmpfunc(const void * a, const void * b)
{
	return (*(int*)a - *(int*)b);
}



#define NUM_BUCKETS 1000
#define NUM_ELEMENTS 30000000
#define LIMIT 100000

int main(int argc, char *argv[]) {

	int * A = malloc(NUM_ELEMENTS * sizeof(int));

	srand(time(NULL));

	randomizeArray(A, NUM_ELEMENTS);


	if (NUM_ELEMENTS < 31) {
		printArray(A, NUM_ELEMENTS);
	}

	//// Time start
	double timeStart = omp_get_wtime();
	bucketSortS(A, NUM_ELEMENTS, NUM_BUCKETS);
	double endTime = omp_get_wtime() - timeStart;
	//// Time end

	printf("size = %d	buckets = %d	time = %f\n", NUM_ELEMENTS, NUM_BUCKETS, endTime);

	if (!isSorted(A, NUM_ELEMENTS)) {
		printf("Array is not sorted!");
	}

	if (NUM_ELEMENTS < 31) {
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
Insert random number into array
*/
void randomizeArray(int * const arr, const int size)
{
	for (size_t i = 0; i < NUM_ELEMENTS; i++)
	{
		arr[i] = rand() % LIMIT;
	}
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

//Assumes an even distrubution of numbers in the range (0..100 000)
//Extra allocated memory is equal to the size of the input array + two arrays the size of number of buckets.
void bucketSortS(int * const arr, const int size, const int numberOfBuckets)
{

	int * buckets = malloc(size * sizeof(int));
	int * bucketCounter = calloc(numberOfBuckets, sizeof(int));
	int * bucketPrefixSum = calloc(numberOfBuckets, sizeof(int));



	int bucketRange = LIMIT / numberOfBuckets != 0 ? LIMIT / numberOfBuckets : 1;


	//Increment buckets
	for (size_t i = 0; i < size; i++)
	{
		int index = arr[i] / bucketRange;

		if (index >= numberOfBuckets)
			index = numberOfBuckets - 1;

		bucketCounter[index]++;
		bucketPrefixSum[index]++;
	}



	//Calculate prefix sum
	for (size_t i = 1; i < numberOfBuckets; i++)
	{
		bucketPrefixSum[i] += bucketPrefixSum[i - 1];
	}


	//Copy elements to their respective buckets
	for (size_t i = 0; i < size; i++)
	{
		int index = arr[i] / bucketRange;

		if (index >= numberOfBuckets)
			index = numberOfBuckets - 1;

		buckets[--bucketPrefixSum[index]] = arr[i];
	}


	// Sort buckets using qsort
	int prefixSum = 0;
	for (size_t i = 0; i <= numberOfBuckets; i++)
	{

		if (i == numberOfBuckets) {
			qsort(&(buckets[prefixSum]), size - prefixSum, sizeof(int), cmpfunc);
		}
		else {
			qsort(&(buckets[prefixSum]), bucketCounter[i], sizeof(int), cmpfunc);
		}

		prefixSum += bucketCounter[i];

	}

	// Copy elements back to input array
	for (size_t i = 0; i < size; i++)
	{
		arr[i] = buckets[i];
	}

	// Clean up
	free(buckets);
	free(bucketPrefixSum);
	free(bucketCounter);

}




