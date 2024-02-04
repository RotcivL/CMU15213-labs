/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, si, sj, tmp, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;

    // 32 byte block size -> 8 ints
    // 32x32 -> every 8 rows conflict miss within a matrix. 
    //       -> conflict with A and B matrix when accessing same row.
    if (M == 32) {
        for (i = 0; i < N; i+=8) {
            for (j = 0; j < M; j+=8) {
                for (si = i; si < i + 8; si++) {
                    // store A cache line into tmp variables as they line is evicted when storing
                    // into B of same row when diagonal elements are access. ie. A[x][x] placed into
                    // B[x][x]. All of A is evicted when placing into B causing extra misses.
                    tmp = A[si][j];
                    tmp1 = A[si][j+1];
                    tmp2 = A[si][j+2];
                    tmp3 = A[si][j+3];
                    tmp4 = A[si][j+4];
                    tmp5 = A[si][j+5];
                    tmp6 = A[si][j+6];
                    tmp7 = A[si][j+7];

                    B[j][si] = tmp;
                    B[j+1][si] = tmp1;
                    B[j+2][si] = tmp2;
                    B[j+3][si] = tmp3;
                    B[j+4][si] = tmp4;
                    B[j+5][si] = tmp5;
                    B[j+6][si] = tmp6;
                    B[j+7][si] = tmp7;
                }
            }
        }
    // 64x64 -> every 4 rows conflict miss within a matrix. 
    //       -> conflict with A and B matrix when accessing same row.
    } else if (M == 64) {
        for (i = 0; i < N; i+=8) {
            for (j = 0; j < M; j+=8) {
                for (si = i; si < i + 4; si++) {
                    tmp = A[si][j];
                    tmp1 = A[si][j+1];
                    tmp2 = A[si][j+2];
                    tmp3 = A[si][j+3];
                    tmp4 = A[si][j+4];
                    tmp5 = A[si][j+5];
                    tmp6 = A[si][j+6];
                    tmp7 = A[si][j+7];

                    // can only store 4 rows/cache lines of B 
                    // store in correct position
                    B[j][si] = tmp;
                    B[j+1][si] = tmp1;
                    B[j+2][si] = tmp2;
                    B[j+3][si] = tmp3;

                    // store remaining in unused slots
                    // needs to be in reverse order (lower values to higher)
                    // otherwise moving into correct position will be exactly 4 rows apart
                    // causing cache misses
                    B[j][si+4] = tmp7;
                    B[j+1][si+4] = tmp6;
                    B[j+2][si+4] = tmp5;
                    B[j+3][si+4] = tmp4;

                }
                for (sj = 0; sj < 4; sj++) {
                    // get A values vertically
                    tmp = A[si][j+sj];
                    tmp1 = A[si+1][j+sj];
                    tmp2 = A[si+2][j+sj];
                    tmp3 = A[si+3][j+sj];
                    tmp4 = A[si][j+7-sj];
                    tmp5 = A[si+1][j+7-sj];
                    tmp6 = A[si+2][j+7-sj];
                    tmp7 = A[si+3][j+7-sj];

                    // move the values previously stored to correct positon
                    B[j+7-sj][i] = B[j+sj][si];
                    B[j+7-sj][i+1] = B[j+sj][si+1];
                    B[j+7-sj][i+2] = B[j+sj][si+2];
                    B[j+7-sj][i+3] = B[j+sj][si+3];

                    // store horizontally, notice rows being updated at not 4 rows apart
                    B[j+sj][si] = tmp;
                    B[j+sj][si+1] = tmp1;
                    B[j+sj][si+2] = tmp2;
                    B[j+sj][si+3] = tmp3;
                    B[j+7-sj][si] = tmp4;
                    B[j+7-sj][si+1] = tmp5;
                    B[j+7-sj][si+2] = tmp6;
                    B[j+7-sj][si+3] = tmp7;
                }
            }
        }
    } else {
        for (i = 0; i < N; i+=16) {
            for (j = 0; j < M; j+=16) {
                for (si = i; si < i + 16 && si < N; si++) {
                    for (sj = j; sj < j + 16 && sj < M; sj++) {
                        tmp = A[si][sj];
                        B[sj][si] = tmp;
                    }
                }
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

