#pragma once

namespace cpua
{
	namespace parallel
	{
		/**
		* \brief A parallel implementation of Bucketsort using openMP.
		* \note because number of threads corresponds to number of local buckets, having a large number of threads will increase performance
		* even though you might physically only have 4 cores and 4 threads. This implementation is somewhat confusing, and can probably have been
		* implemented easier and more efficiently.
		* \param arr array to sort
		* \param size size of array
		* \param numberOfBuckets number of buckets to use
		* \param intLimit highest integer in the array
		*/
		void BucketSort(int *const arr, const int size, const int numberOfBuckets, const int intLimit);
	}
}

