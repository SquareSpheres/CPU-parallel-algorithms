#include "bspedupack.h"
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define N 2

void oddEvenSort();
void getLarger(int *const arr, int *const otherArr);
void getSmaller(int *const arr, int *const otherArr);
void printArray(int *const arr, const int pid, const int n);
int cmpfunc(const void *a, const void *b) { return (*(int *)a - *(int *)b); }

int nprocs = 0;

int main(int argc, char **argv) {
  nprocs = bsp_nprocs();
  bsp_init(oddEvenSort, 0, 0);
  oddEvenSort();
}

void oddEvenSort() {
  bsp_begin(nprocs);

  // Create fake data. Assuming data is already in threads
  int *arr = malloc(N * sizeof(int));

  // Shared array
  int *otherArr = malloc(N * sizeof(int));

  // Share array
  bsp_push_reg(otherArr, N * sizeof(int));

  for (int i = 0; i < N; i++) {
    arr[i] = (i + 4) - bsp_pid();
  }

  // sort each array
  qsort(arr, N, sizeof(int), cmpfunc);

  // sync
  bsp_sync();

  int pid = bsp_pid();

  for (size_t i = 0; i < bsp_nprocs(); i++) {
    // even threads left
    // odd threads right
    if (i % 2 == 0) {
      // if even p share with left neighbour
      if (pid % 2 == 0 && pid > 0) {
        bsp_put(pid - 1, arr, otherArr, 0, N * sizeof(int));
      }
      // if odd p share with right neighbour
      if (pid % 2 == 1 && pid < nprocs - 1) {
        bsp_put(pid + 1, arr, otherArr, 0, N * sizeof(int));
      }
    }

    // odd threads left
    // even threads right
    else {
      // if even p share with right neighbour
      if (pid % 2 == 0 && pid < nprocs - 1) {
        bsp_put(pid + 1, arr, otherArr, 0, N * sizeof(int));
      }
      // if odd p share with left neighbour
      if (pid % 2 == 1 && pid > 0) {
        bsp_put(pid - 1, arr, otherArr, 0, N * sizeof(int));
      }
    }

    bsp_sync();

    if (i % 2 == 0) {
      if (pid % 2 == 0 && pid > 0) {
        // arr should be larger than otherArr
        getLarger(arr, otherArr);
      }
      if (pid % 2 == 1 && pid < nprocs - 1) {
        // arr should be smaller than otherArr
        getLarger(otherArr, arr);
      }
    } else {
      if (pid % 2 == 0 && pid < nprocs - 1) {
        // arr should be smaller than otherArr
        getLarger(otherArr, arr);
      }
      if (pid % 2 == 1 && pid > 0) {
        // arr should be larger than otherArr
        getLarger(arr, otherArr);
      }
    }

    bsp_sync();
  }

  // check if sorted
  int sorted = 1;
  for (size_t i = 1; i < N; i++) {
    if (arr[i - 1] > arr[i]) {
      sorted = 0;
      break;
    }
  }

  if (!sorted) {
    printf("Thread %d is not sorted\n", pid);
  }

  printf("Thread %d largest value = %d\n", pid, arr[N - 1]);

  // Pop registers
  bsp_pop_reg(otherArr);
  // Free memory on heap
  free(otherArr);
  free(arr);

  bsp_end();
}

void getLarger(int *const arr, int *const otherArr) {
  for (size_t i = 0; i < N; i++) {
    if (otherArr[i] > arr[i]) {
      int temp = arr[i];
      arr[i] = otherArr[i];
      otherArr[i] = temp;
    }
  }
}
