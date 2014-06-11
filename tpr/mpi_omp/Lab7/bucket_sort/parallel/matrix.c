#include <sys/time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>

double *A, *B, *C;


void alloc_matrixes(int N) {
    A = (double*) malloc(N * N * sizeof(double) );
    B = (double*) malloc(N * N * sizeof(double) );
    C = (double*) malloc(N * N * sizeof(double) );
}

void free_matrixes() {
    free(A);
    free(B);
    free(C);
}

void init_matrixes(int N) {
    for (int row = 0; row < N; ++row) {
        for (int column = 0; column < N; ++column) {
            A[row * N + column] = 1 + rand() % 2;
            B[row * N + column] = 1 + rand() % 2;
        }
    }
}

void print_matrix(char* name, double * matrix, int N) {
    printf("\n %s:\n", name);
    for (int row = 0; row < N; ++row) {
        for (int column = 0; column < N; ++column) {
            printf("%4d", (int) matrix[row * N + column]);
        }
        printf("\n");
    }
    printf("\n");
}


void test(int N) {
    alloc_matrixes(N);
    init_matrixes(N);

    //print_matrix("A", A, N);
    //print_matrix("B", B, N);

    struct timeval tv0, tv1;
    gettimeofday(&tv0, NULL);

    int index, row, column, k;
    #pragma omp parallel default(none) private(index, row, column, k) shared(A, B, C, N)
    {
        #pragma omp for
        for (row = 0; row < N; ++row) {
            for (column = 0; column < N; ++column) {
                index = row * N + column;
                C[index] = 0;
                for (k = 0; k< N; ++k) {
                    C[index] += A[row * N + k] * B[k * N + column];
                }
            }
        }
    }

    gettimeofday(&tv1, NULL);
    float time = tv1.tv_sec - tv0.tv_sec + (tv1.tv_usec - tv0.tv_usec) / 1000000.0f;
    //print_matrix("C", C, N);

    printf("time: %f\n", time);
    free_matrixes();
}

int main(int argc, char** argv)
{
    srand(time(NULL));
    int N, processes_number;
    if (argc != 3) {
        printf("usage: %s matrix_size number_of_processes\n", argv[0]);
        return -1;
    }
    N = atoi(argv[1]);
    processes_number = atoi(argv[2]);
    omp_set_num_threads(processes_number);

    test(N);

    return 0;
}

