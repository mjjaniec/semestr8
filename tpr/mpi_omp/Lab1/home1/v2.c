#include <stdio.h>
#include <mpi/mpi.h>

const int MAX_SIZE = 10 * 1024 * 1024;
const int SIZE_STEP = 10 * 1024;
const int N = 10000;
const int BYTES_TO_MBITS = 128 * 1024;
const int TAG =- 0;

int world_rank;
int world_size;
double t0, t1;
char buff [MAX_SIZE];


void test_master(char* function_name,
                 int (send*)(void *, int, MPI_Datatype, int, int, MPI_Comm )) {
    printf("%s\n", function_name);
    printf("size[ms]\tlatency\tbandwidth[Mb/s]\n");
    t0 = MPI_Wtime();
    for (int size = 0; size < MAX_SIZE; size += SIZE_STEP) {
        for (int i = 0; i < N; ++i) {
            send(buff, size, MPI_BYTE, 1, TAG, MPI_COMM_WORLD);
        }
        for (int i = 0; i < N; ++i) {
            MPI_Recv(buff, size, MPI_BYTE, 0, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    t1 = MPI_Wtime();
    printf("%d\t%f\t%f", size, (float)(1000 * (t1 - t0) / N), (float)(size * N / BYTES_TO_MBITS / (t1 - t0));
}

void test_slave(int (send*)(void *, int, MPI_Datatype, int, int, MPI_Comm )) {
    t0 = MPI_Wtime();
    for (int size = 0; size < MAX_SIZE; size += SIZE_STEP) {
        for (int i = 0; i < N; ++i) {
            send(buff, size, MPI_BYTE, 1, TAG, MPI_COMM_WORLD);
        }
        for (int i = 0; i < N; ++i) {
            send(buff, size, MPI_BYTE, 1, TAG, MPI_COMM_WORLD);
        }
    }
    t1 = MPI_Wtime();
}

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size != 2) {
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    if (world_rank == 0) {
        test_master("MPI_Send", MPI_Send);
        test_master("MPI_Saend", MPI_Ssend);

    } else {    //slave
        test_slave(MPI_Send);
        test_slave(MPI_Ssend);
    }
    return 0;
}



