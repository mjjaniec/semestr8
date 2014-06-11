#include <omp.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

double *A, *B, *C;


void alloc_data(int N) {
    A = (double*) malloc(N * N * sizeof(double));
    B = (double*) malloc(N * sizeof(double));
    C = (double*) malloc(N * sizeof(double));
}

void free_data() {
    free(A);
    free(B);
    free(C);
}

void init_data(int N) {
    for (int row = 0; row < N; ++row) {
        for (int column = 0; column < N; ++column) {
            A[row * N + column] = 1 + rand() % 2;
        }
        B[row] = 1 + rand() % 2;
    }
}

void print_matrix(char* name, double * matrix, int rows, int columns) {
    printf("\n %s:\n", name);
    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            printf("%4d", (int) matrix[row * columns + column]);
        }
        printf("\n");
    }
    printf("\n");
}

void multiply(double *A, double *B, double *C, int N) {
#pragma omp parallel shared(A, B, C, N)
    {
#pragma omp for
        for (int row = 0; row < N; ++row) {
            C[row] = 0;
            for (int column = 0; column < N; ++column) {
                C[row] += A[N * row + column] * B[column];
            }
        }
    }
}

int main(int argc, char** argv) {

    if (argc != 3) {
        printf("usage: %s problem_size number_of_processes\n", argv[0]);
        return -1;
    }

    srand(time(NULL));
    int N = atoi(argv[1]);
    int P = atoi(argv[2]);
    omp_set_num_threads(P);

    alloc_data(N);
    init_data(N);

    //print_matrix("A", A, N, N);
    //print_matrix("B", B, N, 1);

    struct timeval  tv0, tv1;
    gettimeofday(&tv0, NULL);
    multiply(A, B, C, N);
    gettimeofday(&tv1, NULL);
    //print_matrix("C", C, N, 1);
    float time = tv1.tv_sec - tv0.tv_sec + (tv1.tv_usec - tv0.tv_usec) / 1000000.0f;
    printf("N: %d, time: %f\n", N, time);

    free_data();
}

float wall_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tb.tv_sec + tv.tv_usec / 1000000.0f;
}
