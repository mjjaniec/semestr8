#include <stdio.h>
#include <mpi.h>

#define $1 printf("master: %d\n",__LINE__);
#define $2 printf("slave: %d\n",__LINE__);
#define MAX_SIZE 10 * 1024 * 1024
const int BYTES_TO_MBITS = 128 * 1024;
const int TAG = 0;
int world_rank;
int world_size;
char buff [MAX_SIZE];
typedef int (*send_type)(void*, int, MPI_Datatype, int, int, MPI_Comm);

void test_master(char* function_name, send_type send) {
    double t0, t1;
    double latency;
    double bandwidth;
    printf("%s\n", function_name);
    printf("size[kB]\tlatency[ms]\tbandwidth[Mb/s]\n");

    for (int size = 1024; size < MAX_SIZE; size += 1024) {
        t0 = MPI_Wtime();
        for (int i = 0; i < MAX_SIZE/size; ++i) {
            (*send)(buff, size, MPI_BYTE, 1, TAG, MPI_COMM_WORLD);
            MPI_Recv(buff, size, MPI_BYTE, 1, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        t1 = MPI_Wtime();
        latency = 1000 * (t1 - t0) / (MAX_SIZE / size);
        bandwidth = 80 / (t1 - t0);
        printf("%8d\t%f\t%f\n", size / 1024, (float)latency, (float)bandwidth);
    }
}

void test_slave(send_type send) {
    for (int size = 1024; size < MAX_SIZE; size += 1024) {
        for (int i = 0; i < MAX_SIZE/size; ++i) {
            MPI_Recv(buff, size, MPI_BYTE, 0, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            (*send)(buff, size, MPI_BYTE, 0, TAG, MPI_COMM_WORLD);
        }
    }
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
        test_master("MPI_Send", &MPI_Send);
        test_master("MPI_Ssend", &MPI_Ssend);

    } else {    //slave
        test_slave(&MPI_Send);
        test_slave(&MPI_Ssend);
    }
    return 0;
}
