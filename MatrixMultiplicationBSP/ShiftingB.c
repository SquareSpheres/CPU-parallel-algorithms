#include "bspedupack.h"
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

/*
 * A grid coordinate with i and j values
 */
typedef struct { int i, j; } PAIR;

/*
 * Shifting-B-Algorithm
 *
 * using the BSP_MODEL
 *
 * dimen:          matrix Dimension
 * test_values:    test coordinates
 * num_test:       number of tests
 *
 *
 * Matrix multiplication algorithm where each processor is responsible for n/p
 * rows each. A and C stays stationary, while B is rotating in a cyclic fashion
 * p-1 times. Each processor then multiplies the appropriate A value with the
 * entire B matrix.
 *
 * Assumptions made:
 *  - p always divides n
 *  - each processor has already gotten their values from an outside source.
 *    This is simulated in the algorithm by just generating random values
 *
 */
void matrixMul(int dimen, PAIR *test_values, int num_tests);

// Global variable, number of processors
int nprocs = 4;

int main(int argc, char **argv) {

  nprocs = 4;

  // Seed the random function
  srand(time(NULL));

  // Test values
  PAIR test_vals[4] = {
      {.i = 0, .j = 0}, {.i = 0, .j = 1}, {.i = 1, .j = 0}, {.i = 1, .j = 1}};

  matrixMul(1800, test_vals, 4);
}

void matrixMul(int dimen, PAIR *test_values, int num_tests) {

  bsp_begin(nprocs);

  int pid = bsp_pid();

  if (dimen % nprocs != 0) {
    printf("Dimension is not divisable by number of processors\n");
    return;
  }

  // Memory allocation
  double *A = malloc((dimen / nprocs) * dimen * sizeof(double));
  double *B = malloc((dimen / nprocs) * dimen * sizeof(double));
  double *C = malloc((dimen / nprocs) * dimen * sizeof(double));

  double *Bshared = malloc((dimen / nprocs) * dimen * sizeof(double));

  double **AA = malloc(dimen / nprocs * sizeof(double *));
  double **BB = malloc(dimen / nprocs * sizeof(double *));
  double **CC = malloc(dimen / nprocs * sizeof(double *));

  // Helper pointers to make it easier working on 2d array
  for (size_t i = 0; i < dimen / nprocs; i++) {
    AA[i] = A + dimen * i;
    BB[i] = B + dimen * i;
    CC[i] = C + dimen * i;
  }

  // Inserting random values
  for (size_t i = 0; i < dimen / nprocs; i++) {
    for (size_t j = 0; j < dimen; j++) {
      AA[i][j] = (double)rand() / (double)(RAND_MAX);
      BB[i][j] = (double)rand() / (double)(RAND_MAX);
      CC[i][j] = 0;
    }
  }

  // ================= Cij TEST start ================================
  for (size_t i = 0; i < num_tests; i++) {

    int Ci = test_values[i].i;
    int Cj = test_values[i].j;

    // Find processor responsible for row Ai and make it responsible for
    // calculating A*B
    int A_test_row_pid = Ci / (dimen / nprocs);
    // Find local row index
    int A_test_row_index = Ci - (A_test_row_pid * (dimen / nprocs));

    // Create shared B_test array to put values to
    double *B_test_shared = malloc(dimen * sizeof(double));
    bsp_push_reg(B_test_shared, dimen * sizeof(double));
    bsp_sync();

    // Each process must send their B values to A_test_row_pid
    for (size_t j = 0; j < nprocs; j++) {
      if (pid == j) {
        // Creating temporary array to store column values. Assuming there is
        // enough space on the stack
        double B_test_local_col[dimen / nprocs];
        for (size_t k = 0; k < dimen / nprocs; k++) {
          B_test_local_col[k] = BB[k][Cj];
        }
        // offset for B_test_shared
        int OFFSET = (dimen / nprocs) * sizeof(double) * pid;
        // Copy values to responsible thread
        bsp_put(A_test_row_pid, B_test_local_col, B_test_shared, OFFSET,
                (dimen / nprocs) * sizeof(double));
      }
    }
    bsp_sync();

    // calculate A*B
    if (pid == A_test_row_pid) {
      // calculate Atest*Btest
      double cval = 0;

      for (size_t j = 0; j < dimen; j++) {
        cval += AA[A_test_row_index][j] * B_test_shared[j];
      }

      printf("\n\n Test_%d : (%d,%d) == %lf \n\n", i, Ci, Cj, cval);
    }

    // Cleanup
    bsp_pop_reg(B_test_shared);
    free(B_test_shared);
  }

  // ================= Cij TEST end ================================

  // ============ ALGORITHM START =================

  // Time start
  bsp_sync();
  double timeStart = bsp_time();

  // Broadcast Bshared
  bsp_push_reg(Bshared, (dimen / nprocs) * dimen * sizeof(double));
  bsp_sync();

  for (size_t i = 0; i < nprocs; i++) {
    for (size_t j = 0; j < dimen / nprocs; j++) {

      int valueA =
          (((pid * (dimen / nprocs)) + (i * (dimen / nprocs))) % dimen);

      for (size_t k = 0; k < dimen / nprocs; k++) {

        for (size_t p = 0; p < dimen; p++) {
          CC[j][p] += (AA[j][valueA] * BB[k][p]);
        }

        valueA++;
      }
    }

    bsp_sync();

    // Shift B one left
    if (bsp_pid() == 0) {
      bsp_put(nprocs - 1, B, Bshared, 0,
              (dimen / nprocs) * dimen * sizeof(double));
    } else {
      bsp_put(pid - 1, B, Bshared, 0,
              (dimen / nprocs) * dimen * sizeof(double));
    }

    bsp_sync();

    // Copy from Bshared to B
    memcpy(B, Bshared, (dimen / nprocs) * dimen * sizeof(double));
  }

  // Time end
  bsp_sync();
  double timeEnd = bsp_time() - timeStart;
  if (pid == 0) {
    printf("B-shift time = %lf\n", timeEnd);
  }

  // ============ ALGORITHM END =================

  // =================== PRINT REAL VALUES START =============================
  for (size_t i = 0; i < num_tests; i++) {

    int Ci = test_values[i].i;
    int Cj = test_values[i].j;

    int A_test_row_pid = Ci / (dimen / nprocs);

    if (pid == A_test_row_pid) {
      int local_i = Ci - (A_test_row_pid * (dimen / nprocs));
      int local_j = Cj;
      double cval = CC[local_i][local_j];
      printf("\n\n Real_val : (%d,%d) == %lf \n\n", Ci, Cj, cval);
    }
  }
  // =================== PRINT REAL VALUES END =============================

  // Pop registers
  bsp_pop_reg(Bshared);
  // Cleanup
  free(Bshared);
  free(A);
  free(B);
  free(C);
  free(AA);
  free(BB);
  free(CC);

  bsp_end();
}
