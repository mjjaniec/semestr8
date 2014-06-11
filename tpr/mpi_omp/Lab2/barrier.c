#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

const int TAG = 0;
const int N = 1000;

char * data;

void master(const int world_size) {
    double t1, t0;
    double latency;

    t0 = MPI_Wtime();
    for (int i = 0; i < N; ++i) {
        MPI_Barrier(MPI_COMM_WORLD);
    }
    t1 = MPI_Wtime();
    latency = 1000 * (t1 - t0) / N;
    printf("native MPI_Barrier: latency %f[ms]\t", (float)latency);

    t0 = MPI_Wtime();
    for (int i = 0; i<N; ++i) {
        for (int rank = 1; rank < world_size; ++rank) {
            MPI_Recv(data, 0, MPI_BYTE, rank, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        for (int rank = 1; rank < world_size; ++rank) {
            MPI_Send(data, 0, MPI_BYTE, rank, TAG, MPI_COMM_WORLD);
        }
    }
    t1 = MPI_Wtime();
    latency = 1000 * (t1 - t0) / N;
    printf("my implementation: latency %f\n", (float)latency);
}


void slave() {
    for (int i = 0; i < N; ++i) {
        MPI_Barrier(MPI_COMM_WORLD);
    }
    for (int i = 0; i < N; ++i) {
        MPI_Send(data, 0, MPI_BYTE, 0, TAG, MPI_COMM_WORLD);
        MPI_Bcast(data, 0, MPI_BYTE, 0, MPI_COMM_WORLD);
    }
}



int main(int argc, char** argv)
{
    int world_rank;
    int world_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_rank == 0) {
        master(world_size);
    } else {    
        slave();
    }

    MPI_Finalize();

    return 0;
}
