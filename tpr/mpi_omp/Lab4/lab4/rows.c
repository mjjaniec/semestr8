#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define null NULL
#define N 10
const int ROOT = 0;


void * contignous_alloc(int columns, int rows, int element_size, void** contignous_space) {
    void* data = malloc(columns * rows * element_size);
    void ** result = malloc(rows * sizeof (void*));
    for (int i = 0; i < rows; ++ i) {
        result[i] = data + (i * columns * element_size);
    }
    if (contignous_space != null) {
        *contignous_space = data;
    }
    return result;
}

int **data;
int *contignous_space;
int chunk[N][N];

void init_data() {
    data = (int**) contignous_alloc(N, N, sizeof(int), (void**)&contignous_space);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j< N; ++j) {
            data[i][j] = N*i + j;
        }
    }

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            printf("%6d ",data[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char**argv) {
    int world_rank;
    int world_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_rank == ROOT) {
        init_data();
    }


    MPI_Datatype ROWS_TYPE;
    MPI_Type_contiguous(N * N / world_size, MPI_INT, &ROWS_TYPE);
    MPI_Type_commit(&ROWS_TYPE);

    MPI_Scatter(contignous_space, 1, ROWS_TYPE, chunk, 1, ROWS_TYPE, ROOT, MPI_COMM_WORLD);
    printf("rank: %d, first elem: %d\n", world_rank, chunk[0][0]);

    MPI_Finalize();
    return 0;
}

