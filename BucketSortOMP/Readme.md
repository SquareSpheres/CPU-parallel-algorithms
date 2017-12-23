# BucketSort

OpenMP implementation of a bcuket sort algorithm. There is also a sequential implementation of bucket sort.
The sequential version can be used by the parallel one to inrease running time over the standard soring, std::sort.

## Bucket sort

Bucket sort, or bin sort, is a sorting algorithm that works by distributing the elements of an array into a number of buckets.
Each bucket is then sorted individually, either using a different sorting algorithm, or by recursively applying the bucket sorting
algorithm. It is a distribution sort, a generalization of pigeonhole sort, and is a cousin of radix sort in the most-to-least
significant digit flavor. Bucket sort can be implemented with comparisons and therefore can also be considered a comparison sort algorithm.
The computational complexity estimates involve the number of buckets. 
[<cite>Bucket sort</cite> <a href="https://en.wikipedia.org/wiki/Bucket_sort">Wikipedia</a>.]
