#pragma once

namespace cpua
{
	namespace sequential
	{
		/**
		* \brief A sequential implementation of Bucketsort.
		* \param arr array to sort
		* \param size size of array
		* \param numberOfBuckets number of buckets to use
		* \param intLimit highest integer in the array
		*/
		void BucketSort(int *const arr, const int size, const int numberOfBuckets, const int intLimit);
	}
}
