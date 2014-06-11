#include <mpi.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define N 100
const int ROOT;

int data[N][N];
int chunk[N][N];

int init_data() {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            data[i][j]=N*10*i+j;
        }
    }
}

int main(int argc, char** argv) {
    MPI_Datatype columntype;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    MPI_Type_vector(N, 1, N, MPI_INT, &columntype);
    MPI_Type_commit(&columntype);

    MPI_Scatter(data, 1, columntype, chunk, 1, columntype, ROOT, MPI_COMM_WORLD);

    printf("rank: %d result: %d\n", world_rank, chunk[0][0]);

    MPI_Finalize();
}
