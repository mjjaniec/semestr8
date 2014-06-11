#include <mpi.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define N 100
const int ROOT;

int* data;
int chunk[N][N];

int init_data() {
    data = malloc(N*N*sizeof(int));
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            data[i*N+j] = N*10*i+j;
        }
    }
}

int main(int argc, char** argv) {

    int world_rank;
    int world_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    MPI_Datatype MAT_ROWS_TYPE;
    MPI_Type_contiguous(N * N / world_size, MPI_INT, &MAT_ROWS_TYPE);
    MPI_Type_commit(&MAT_ROWS_TYPE);


    if( world_rank == 0) {
        init_data();
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Scatter(data, 1, MAT_ROWS_TYPE, chunk, 1, MAT_ROWS_TYPE, ROOT, MPI_COMM_WORLD);

    printf("rank: %d first_element: %d\n", world_rank, chunk[0][0]);

    MPI_Finalize();

    return 0;
}

