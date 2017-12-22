#include "BucketSort.h"
#include "ParallelBucketSort.h"
#include <vector>
#include <random>
#include <omp.h>

#define NUM_BUCKETS 100000
#define NUM_ELEMENTS 30000000
#define LIMIT 100000

#define NUM_TESTS 100

int main()
{

	for (int k = 0; k < NUM_TESTS; ++k)
	{
		// Generate random data
		std::random_device randomDevice;
		std::mt19937 prng(randomDevice());
		const std::uniform_int_distribution<> uniformIntDistribution(0, LIMIT);

		std::vector<int> numbers(NUM_ELEMENTS);

		for (int i = 0; i < NUM_ELEMENTS; ++i)
		{
			numbers[i] = uniformIntDistribution(prng);
		}

		// sort the data and time it
		const double timeStart = omp_get_wtime();
		//cpua::sequential::BucketSort(&numbers[0], numbers.size(), NUM_BUCKETS, LIMIT);
		cpua::parallel::BucketSort(&numbers[0], numbers.size(), NUM_BUCKETS, LIMIT);
		//std::sort(numbers.begin(),numbers.end());
		const double endTime = omp_get_wtime() - timeStart;


		// verify that the data is sorted
		bool isSorted = true;
		for (int j = 1; j < NUM_ELEMENTS; ++j)
		{
			if(numbers[j-1]>numbers[j])
			{
				isSorted = false;
				break;
			}
		}

		if(!isSorted)
		{
			// sorting failed
			std::cout << "Numbers are not sorted... exiting" << std::endl;
			exit(0);
		}

		std::cout << "Sorting " << NUM_ELEMENTS << " elements with " << NUM_BUCKETS << " buckets took : " << endTime << " seconds" << std::endl;
	}
	
    return 0;

}

