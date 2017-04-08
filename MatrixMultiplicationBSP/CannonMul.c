#include "bspedupack.h"
#include <math.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

/*
 * A grid coordinate with i and j values
 */
typedef struct { int i, j; } PAIR;
/*
 * Cannons Algorithm
 *
 * using the BSP_MODEL
 *
 * dimen:          matrix Dimension
 * test_values:    test coordinates
 * num_test:       number of tests
 *
 *
 * Assumptions made:
 *  - number of processor is square
 *  - s = sqrt(number of processors-1)
 *  - s always divides n
 *
 */
void cannonMul(int dimen, PAIR *test_values, int num_tests);
/*
 * Sequential matrix multiplication algorithm with running time n^3
 */
void sequentialMulAdd(double **A, double **B, double **C, int n);

int nprocs = 4;

int main(int argc, char **argv) {

  nprocs = 4;
  srand(time(NULL));

  PAIR test_vals[4] = {
      {.i = 0, .j = 0}, {.i = 0, .j = 1}, {.i = 1, .j = 0}, {.i = 1, .j = 1}};

  cannonMul(4, test_vals, 4);
}

void cannonMul(int dimen, PAIR *test_values, int num_tests) {

  bsp_begin(nprocs);

  // Check if number of processors is a square
  double squareCheck = sqrt(nprocs);
  if (!((int)squareCheck * (int)squareCheck == nprocs)) {
    printf("Number of processors must be a square number\n");
    return;
  }

  // Check if number if elements is divisable by number of processors
  if ((dimen * dimen) % nprocs != 0) {
    printf("Dimension is not divisable by number of processors\n");
    return;
  }

  // Processors ID
  int pid = bsp_pid();
  // Processor matrix size
  int localDim = dimen / sqrt(nprocs);
  // Number of processors per row in processor grid
  int nprocsPerRow = sqrt(nprocs);

  // Process I grid ID
  int pidI = pid / nprocsPerRow;
  // Process J grid ID
  int pidJ = pid % nprocsPerRow;

  // Memory allocation
  double *A = malloc(localDim * localDim * sizeof(double));
  double *Ashared = malloc(localDim * localDim * sizeof(double));

  double *B = malloc(localDim * localDim * sizeof(double));
  double *Bshared = malloc(localDim * localDim * sizeof(double));

  double *C = calloc(localDim * localDim, sizeof(double));

  // Helper pointers to work on 2d array
  double **AA = malloc(localDim * sizeof(double *));
  double **BB = malloc(localDim * sizeof(double *));
  double **CC = malloc(localDim * sizeof(double *));

  for (size_t i = 0; i < localDim; i++) {
    AA[i] = A + localDim * i;
    BB[i] = B + localDim * i;
    CC[i] = C + localDim * i;
  }

  // Inserting random values
  for (size_t i = 0; i < localDim; i++) {
    for (size_t j = 0; j < localDim; j++) {
      AA[i][j] = (double)rand() / (double)(RAND_MAX);
      BB[i][j] = (double)rand() / (double)(RAND_MAX);
    }
  }

  // ================= Cij TEST start ================================

  // Get Ai row from each process and send to pid=0
  // Get Bj column from each process and send to pid=0
  // pid=0 calculates A*B and prints result.

  // Alternative test
  // pid==0 gets values instead of puting to 0

  // Alternative test
  // each pid get their respective B values from correct pid, and all C values
  // are sent to one process for summation

  // overly complicated test?

  // only malloc pid==0 and only share pid==0?
  double *Atest = malloc(dimen * sizeof(double));
  double *Btest = malloc(dimen * sizeof(double));
  bsp_push_reg(Atest, dimen * sizeof(double));
  bsp_push_reg(Btest, dimen * sizeof(double));
  bsp_sync();

  for (size_t i = 0; i < num_tests; i++) {

    int Ci = test_values[i].i;
    int Cj = test_values[i].j;

    // Starting PID
    int A_row_start_pid = (Ci / localDim) * nprocsPerRow;
    // Ending PID
    int A_row_end_pid = A_row_start_pid + nprocsPerRow - 1;
    // Local row start
    int A_local_row_start = (Ci - (Ci / localDim) * localDim);

    // For all processors in range(A_row_start_pid --> A_row_end_pid)
    for (size_t i = A_row_start_pid; i <= A_row_end_pid; i++) {
      if (pid == i) {
        bsp_put(0, AA[A_local_row_start], Atest,
                pidJ * localDim * sizeof(double), localDim * sizeof(double));
      }
    }

    // Starting PID
    int B_col_start_pid = (Cj / localDim) % nprocsPerRow;
    // Ending PID
    int B_col_end_pid = nprocs;
    // Local column start
    int B_local_col_start = (Cj - (Cj / localDim) * localDim);

    for (size_t i = B_col_start_pid; i < B_col_end_pid; i += nprocsPerRow) {
      if (pid == i) {
        // Assuming there is sufficient space on the stack for one column
        double BcolTmp[localDim];
        for (size_t j = 0; j < localDim; j++) {
          BcolTmp[j] = BB[j][B_local_col_start];
        }

        bsp_put(0, BcolTmp, Btest, pidI * localDim * sizeof(double),
                localDim * sizeof(double));
      }
    }

    bsp_sync();

    if (pid == 0) {
      // calculate Atest*Btest
      double cval = 0;
      for (size_t i = 0; i < dimen; i++) {
        cval += Atest[i] * Btest[i];
      }

      printf("\n\n Test_%d : (%d,%d) == %lf \n\n", i, Ci, Cj, cval);
    }
  }

  // Cleanup
  bsp_pop_reg(Atest);
  bsp_pop_reg(Btest);
  free(Atest);
  free(Btest);

  // ================= Cij TEST end ================================

  // ============ ALGORITHM START =================

  // Time start
  double timeStart = bsp_time();

  // Broadcast Ashared and Bshared
  bsp_push_reg(Ashared, localDim * localDim * sizeof(double));
  bsp_push_reg(Bshared, localDim * localDim * sizeof(double));

  // Copy A and B to Ashared and Bshared respectively
  memcpy(Ashared, A, localDim * localDim * sizeof(double));
  memcpy(Bshared, B, localDim * localDim * sizeof(double));

  bsp_sync();

  // Shift Aij i positions left
  if (pidI > 0) {

    int rowStart = pidI * nprocsPerRow;
    int rowEnd = rowStart + (nprocsPerRow - 1);

    if (pid - pidI < rowStart) {
      // loop around
      int overShoot = (pidI - pidJ) - 1; // overshoot must be variable of pidI
      int toPid = rowEnd - overShoot;
      bsp_put(toPid, A, Ashared, 0, localDim * localDim * sizeof(double));
    } else {
      int toPid = pid - pidI;
      bsp_put(toPid, A, Ashared, 0, localDim * localDim * sizeof(double));
    }
  }

  // Shift Bij j positions up
  if (pidJ > 0) {

    int rowStart = 0;
    int rowEnd = nprocsPerRow;

    if (pid - (pidJ * nprocsPerRow) < 0) {
      // loop around
      int overShoot = (pidJ - pidI) - 1;
      int toPid = (nprocs - nprocsPerRow + pidJ) - (overShoot * nprocsPerRow);
      bsp_put(toPid, B, Bshared, 0, localDim * localDim * sizeof(double));
    } else {
      int toPid = pid - (pidJ * nprocsPerRow);
      bsp_put(toPid, B, Bshared, 0, localDim * localDim * sizeof(double));
    }
  }

  bsp_sync();

  // Copy Ashared, and Bshared back to A and B
  memcpy(A, Ashared, localDim * localDim * sizeof(double));
  memcpy(B, Bshared, localDim * localDim * sizeof(double));

  // Local matrix multiplication, C += A * B
  sequentialMulAdd(AA, BB, CC, localDim);

  // Make sure every thread is finished copying from Ashared and Bshared
  // before giving them new values.
  bsp_sync();

  // Repeat sqrt(nprocs-1) times
  for (size_t i = 0; i < sqrt(nprocs) - 1; i++) {

    // Shift B up
    if (pidI == 0) {
      bsp_put((nprocsPerRow - 1) * nprocsPerRow + pidJ, B, Bshared, 0,
              localDim * localDim * sizeof(double));
    } else {
      bsp_put(pid - nprocsPerRow, B, Bshared, 0,
              localDim * localDim * sizeof(double));
    }

    // Shift A left
    if (pidJ == 0) {
      bsp_put(pid + nprocsPerRow - 1, A, Ashared, 0,
              localDim * localDim * sizeof(double));
    } else {
      bsp_put(pid - 1, A, Ashared, 0, localDim * localDim * sizeof(double));
    }

    bsp_sync();

    // Update A and B with new values from Ashared and Bshared
    memcpy(A, Ashared, localDim * localDim * sizeof(double));
    memcpy(B, Bshared, localDim * localDim * sizeof(double));

    // Local matrix multiplication, C += A * B
    sequentialMulAdd(AA, BB, CC, localDim);

    // Make sure every thread is finished copying from Ashared and Bshared
    // before giving them new values.
    bsp_sync();
  }

  double timeEnd = bsp_time() - timeStart;
  if (pid == 0) {
    printf("Cannon time = %lf\n", timeEnd);
  }

  // ============ ALGORITHM END =================

  // =================== PRINT REAL VALUES =============================

  for (size_t i = 0; i < num_tests; i++) {
    int Ci = test_values[i].i;
    int Cj = test_values[i].j;
    int test_pid =
        ((Ci / localDim) * nprocsPerRow) + ((Cj / localDim) % nprocsPerRow);

    if (pid == test_pid) {
      int local_i = Ci - (Ci / localDim) * localDim;
      int local_j = Cj - (Cj / localDim) * localDim;
      double cval = CC[local_i][local_j];
      printf("\n\n Real_val : (%d,%d) == %lf \n\n", Ci, Cj, cval);
    }
  }

  // =================== PRINT REAL VALUES =============================

  // Pop registers
  bsp_pop_reg(Bshared);
  bsp_pop_reg(Ashared);

  // Cleanup
  free(Ashared);
  free(Bshared);
  free(A);
  free(B);
  free(AA);
  free(BB);

  bsp_end();
}

/*
 * Multiples A with B and appends to C. Assumes A and B are sqaure matrices
 * C += A * B
 */
void sequentialMulAdd(double **A, double **B, double **C, int n) {

  for (int i = 0; i < n; i++) {
    for (int k = 0; k < n; k++) {
      for (int j = 0; j < n; j++) {
        C[i][j] += A[i][k] * B[k][j];
      }
    }
  }
}
