#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

double *A, *B, *C;
const int SIZES[] = {
	100, 150, 200, 300, 400, 500, 600, 700,
	800, 900, 1000, 1200, 1400, 1600, 1800, 2000,
	100, 126, 144, 159, 171, 182, 191, 200,
	1000, 1260, 1442, 1587, 1710, 1817, 1913, 2000
};
const int TESTS = sizeof(SIZES) / sizeof(int);
const int ROOT = 0;

#define $ printf("%d\n",__LINE__);

void alloc_matrixes(int N) {
    /**
      +10 -> tail
      e.g. matrix 3 x 3 divided for two processes

      x x x \_ first process
      x x x /
      x x x \_ second process
      t a i /
      l ~ ~

      */
    A = (double*) malloc(N * (N + 10) * sizeof(double) );
    B = (double*) malloc(N * (N + 10) * sizeof(double) );
    C = (double*) malloc(N * (N + 10) * sizeof(double) );
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

void compute(int per_worker, int N) {
    int index;
    for (int row = 0; row < per_worker; ++row) {
        for (int column = 0; column < N; ++column) {
            index = row * N + column;
            C[index] = 0;
            for (int k = 0; k< N; ++k) {
                C[index] += A[row * N + k] * B[k * N + column];
            }
        }
    }
}


void test(int N, int world_rank, int world_size) {
    double t0, t1;
    int per_worker = N / world_size;
    if (per_worker * world_size < N) {
        ++per_worker;
    }
    MPI_Datatype ROW_TYPE;
    MPI_Type_contiguous(N, MPI_DOUBLE, &ROW_TYPE);
    MPI_Type_commit(&ROW_TYPE);

    MPI_Datatype COLUMN_TYPE;
    MPI_Type_vector(N, 1, N, MPI_DOUBLE, &COUMN_TYPE);
    MPI_Type_commit(&COLUMN_TYPE);
    MPI_Type_create_resized(COLUMN_TYPE, 0, sizeof(double), &COLUMN_TYPE);
    MPI_Type_commit(&COLUMN_TYPE);

    alloc_matrixes(N);
    if (world_rank == ROOT) {
        init_matrixes(N);
    }

    /*if (world_rank == ROOT) {
        print_matrix("A", A, N);
        print_matrix("B", B, N);
    }*/

    MPI_Barrier(MPI_COMM_WORLD);

    t0 = MPI_Wtime();
    MPI_Bcast(B, N, COLUMN_TYPE, ROOT, MPI_COMM_WORLD);

    // so ugly api :)
    MPI_Scatter(A, per_worker, ROW_TYPE, world_rank==ROOT?MPI_IN_PLACE:A, per_worker, ROW_TYPE, ROOT, MPI_COMM_WORLD);


    compute(per_worker, N);

    // again :D
    MPI_Gather(world_rank==ROOT?MPI_IN_PLACE:C, per_worker, ROW_TYPE, C, per_worker, ROW_TYPE, ROOT, MPI_COMM_WORLD);
    t1 = MPI_Wtime();
    if (world_rank == ROOT)
    printf("problem_size: %7d time: %f s\n", N, (float)(t1 - t0));

    /*if (world_rank == ROOT) {
        print_matrix("C", C, N);
    }*/

    free_matrixes();
}


int main(int argc, char** argv)
{
    int world_rank;
    int world_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    srand(time(NULL));

    for (int i = 0; i < TESTS; ++i) {
        test(SIZES[i], world_rank, world_size);
    }

    MPI_Finalize();
    return 0;
}

