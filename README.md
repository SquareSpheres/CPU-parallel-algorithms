# Parallel Algorithms

Some parallel algorithms implemented using the Bulk Synchronous Parallel model, and OpenMP. The algorithms are implemented in C, but might as well have been implemented in C++. Using C++ I would see a significant performance increase especially on the sorting algorithms. The algorithms were tested on a machine with 80 threads. On the performance graphs, you will see some strange behavior were performance drops especially on the 20,40, 60 thread marks. This probably has to do with recruiting new processor cores, and the overhead that comes with it.

## Bucket sort

<details> 
  <summary>Sequential bucket sort </summary>
  <img src="/Results//Bucket_Seq_sort.PNG">
</details>

<details> 
  <summary>Parallel bucket sort </summary>
  <img src="/Results//Bucket_Par_sort.PNG">
  <img src="/Results//Bucket_speedup_p.PNG">
  <img src="/Results//Bucket_speedup_s.PNG">
  <img src="/Results//Bucket_weak_scaling.PNG">
</details>


## Matrix multiplication

<details> 
  <summary>Shifting-B </summary>
  <img src="/Results//Shift_B.PNG">
  <img src="/Results//Shift_B_scale.PNG">
</details>

<details> 
  <summary>Cannon's Algorithm </summary>
  <img src="/Results//Cannon.PNG">
  <img src="/Results//Cannon_speedup.PNG">
</details>


## Odd–Even Transposition sort
