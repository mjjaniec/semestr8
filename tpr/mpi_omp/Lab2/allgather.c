#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>

const int ROOT = 0;
const int TAG = 0;
const int N = 10;

const int sizes[]={1,2,5,10,20,50,100,200,500,1024,2048,5120,10240};
const int len = sizeof(sizes) / sizeof(int);

void master(const int world_rank, const int world_size) {

   char *data = (char*) malloc(11*1024*1024*10);
   char *chunk = (char*) malloc(11*1024*1024);

   double t1, t0;
   double latency;
   double bandwidth;
   int size;

   printf("native MPI_Allgather\n");
   for (int k = 0; k < len; ++k ) {
       t0 = MPI_Wtime();
       size = sizes[k] * 1024;
       for (int i = 0; i<N; ++i) {
           for (int rank = 1; rank < world_size; ++rank) {
               MPI_Allgather(chunk, size, MPI_BYTE, data, size, MPI_BYTE, MPI_COMM_WORLD);
           }
       }
       t1 = MPI_Wtime();
       latency = 1000 * (t1 - t0) / N;
       bandwidth = (world_size - 1) * (world_size - 1) * N * size / 128.0 / 1024.0 / (t1 - t0);
       printf("latency %f[ms] bandwidth %f[Mbps]\n", (float)latency, (float)bandwidth);
   }


   printf("my implementation\n");
   for (int k = 0; k < len; ++k ) {
       t0 = MPI_Wtime();
       size = sizes[k] * 1024;
       for (int i = 0; i < N; ++i) {
           memcpy(data, chunk, size);
           for(int rank = 1; rank < world_size; ++rank) {
               MPI_Send(chunk, size, MPI_BYTE, rank, TAG, MPI_COMM_WORLD);
           }

           for(int rank = 1; rank < world_size; ++rank) {
               MPI_Recv(data+rank*size, size, MPI_BYTE, rank, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
           }
       }
       t1 = MPI_Wtime();
       latency = 1000 * (t1 - t0) / N;
       bandwidth = (world_size - 1) * (world_size - 1) * N * size / 128.0 / 1024.0 / (t1 - t0);
       printf("latency %f[ms] bandwidth %f[Mbps]\n", (float)latency, (float)bandwidth);
   }
}


void slave(int world_rank, int world_size) {
   char *data = (char*) malloc(11*1024*1024*10);
   char *chunk = (char*) malloc(11*1024*1024);
   int size;

   for (int k = 0; k < len; ++k ) {
       size = sizes[k] * 1024;
       for (int i = 0; i<N; ++i) {
           for (int rank = 1; rank < world_size; ++rank) {
               MPI_Allgather(chunk, size, MPI_BYTE, data, size, MPI_BYTE, MPI_COMM_WORLD);
           }
       }
   }

   for (int k = 0; k < len; ++k ) {
       size = sizes[k] * 1024;
       for (int i = 0; i < N; ++i) {
           memcpy(data + world_rank*size, chunk, size);
           for (int rank = 0; rank < world_rank; ++rank) {
               MPI_Recv(data+rank*size, size, MPI_BYTE, rank, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
           }
           for (int rank = 0; rank < world_size; ++rank) {
               if (rank != world_rank) {
                   MPI_Send(chunk, size, MPI_BYTE, rank, TAG, MPI_COMM_WORLD);
               }
           }
           for (int rank = world_rank + 1; rank < world_size; ++rank) {
               MPI_Recv(data+rank*size, size, MPI_BYTE, rank, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
           }
       }
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
       master(world_rank, world_size);
   } else {
       slave(world_rank, world_size);
   }

   MPI_Finalize();

   return 0;
}
