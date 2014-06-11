#include <mpi.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>


#include "matrix.h"
#include "commons.h"

const int ROOT = 0;

struct {
    int A, B, C;
}input_data;

Matrix X, Y, Z;
Matrix local_X_rows;
Matrix local_Y;

void init_matrixes() {
    Matrix_fill_with_random_data(&X);
    Matrix_fill_with_random_data(&Y);

    printf("X:\n");
    Matrix_print(&X);
    printf("\nY:\n");
    Matrix_print(&Y);
}

void allocate_matrixes() {
    Matrix_init(&X, input_data.A, input_data.B);
    Matrix_init(&Y, input_data.B, input_data.C);
    Matrix_init(&Z, input_data.A, input_data.C);

}

void free_matrixes() {
    Matrix_free(&X);
    Matrix_free(&Y);
    Matrix_free(&Z);
}

void parse_arguments(int argc, char ** argv) {
    if(argc == 4) {
        errno = 0;
        input_data.A = atoi(argv[1]);
        input_data.B = atoi(argv[2]);
        input_data.C = atoi(argv[3]);
        if (errno == 0) {
            return;
        }
    }

    printf("usage: A, B, C, where A, B, C are sizes of matrixes\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
    exit(1);
}

void multiply_matrixes(int world_rank, int world_size) {
    void * data;
    int size;
    int N = input_data.B / world_size;    

    data = Matrix_get_contiguous_data(&Y);
    size = input_data.B*input_data.C;
    MPI_Bcast(data, size, MPI_DOUBLE, ROOT, MPI_COMM_WORLD);

    //Matrix_print(&Y);

    data = Matrix_get_contiguous_data(&X);
    size = input_data.C*input_data.B;
    //printf("N: %d, size: %d\n", N, size);
    void * ignore=malloc(500);
    /*if(world_rank == 0) {
        MPI_Scatter(data,N*size,MPI_DOUBLE,ignore,N*size,MPI_DOUBLE,ROOT,MPI_COMM_WORLD);
    } else {
        MPI_Scatter(ignore,N*size,MPI_DOUBLE,data,N*size,MPI_DOUBLE,ROOT,MPI_COMM_WORLD);
    } */  
    MPI_Bcast(data, size, MPI_DOUBLE, ROOT, MPI_COMM_WORLD);
    double** Xdata = Matrix_get_data(&X);
    double** Ydata = Matrix_get_data(&Y);
    double** Zdata = Matrix_get_data(&Z);

    for(int x_row =0; x_row<N; ++x_row) {
        for(int i = 0; i <input_data.B; ++i) {
            Zdata[x_row][i] = 0;
            for (int k = 0; k < input_data.B; ++k) {
                Zdata[x_row][i] += Xdata[x_row+N*world_rank][k] * Ydata[k][i];
            }
        }
    }
    
    //MPI_Gather(Zdata, N*size, MPI_DOUBLE, Zdata, N*size, MPI_DOUBLE, ROOT, MPI_COMM_WORLD);
    
}


int main(int argc, char** argv) {
    int world_rank;
    int world_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    parse_arguments(argc, argv);
    srand(time(0)^world_rank);

    allocate_matrixes();

    if (world_rank == ROOT) {
        init_matrixes();
    }

    multiply_matrixes(world_rank, world_size);

    if (world_rank == ROOT) {
        Matrix_print(&Z);
    }

    free_matrixes();

    MPI_Finalize();
}
