#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define null NULL
#define COLUMNS 10
#define ROWS 12
const int ROOT = 0;
const int TAG = 0;


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
int chunk[COLUMNS*ROWS];

void init_data() {
    data = (int**) contignous_alloc(COLUMNS, ROWS, sizeof(int), (void**)&contignous_space);
    for (int row = 0; row < ROWS; ++row) {
        for (int column = 0; column< COLUMNS; ++column) {
            data[row][column] = COLUMNS*row + column;
        }
    }

    for (int row = 0; row < ROWS; ++row) {
        for (int column = 0; column< COLUMNS; ++column) {
            printf("%3d ",data[row][column]);
        }
        printf("\n");
    }
}

void print_received_atomically(int world_rank, int world_size, int max) {
    char ignore[0];
    if (world_rank > 0) {
        MPI_Recv(ignore,0,MPI_INT,world_rank-1,TAG,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    printf("rank: %d\n", world_rank);
    for (int i = 0; i< max; ++i) {
        printf("%3d ", chunk[i]);
    }
    printf("\n");

    fflush(stdout);

    //how to wait for actual flush?
    sleep(1);
    if (world_rank + 1 < world_size) {
        MPI_Send(ignore,0,MPI_INT,world_rank+1,TAG, MPI_COMM_WORLD);
    }
}

void clear_chunk() {
    for (int row = 0; row < ROWS; ++row) {
        for (int column = 0; column< COLUMNS; ++column) {
            chunk[COLUMNS*row+column]=0;
        }
    }
}

int main(int argc, char**argv) {
    int world_rank;
    int world_size;
    int data_per_process;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_rank == ROOT) {
        init_data();
        printf("\nrows:\n");
    }
    clear_chunk();

    MPI_Datatype ROWS_TYPE;
    data_per_process = COLUMNS * (ROWS / world_size);
    MPI_Type_contiguous(data_per_process, MPI_INT, &ROWS_TYPE);
    MPI_Type_commit(&ROWS_TYPE);
    MPI_Scatter(contignous_space, 1, ROWS_TYPE, chunk, data_per_process, MPI_INT, ROOT, MPI_COMM_WORLD);
    print_received_atomically(world_rank, world_size, data_per_process);

    MPI_Barrier(MPI_COMM_WORLD);
    if (world_rank == 0) {
        printf("\n\ncolumns:\n");
    }
    clear_chunk();
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Datatype COLUMNS_TYPE;
    MPI_Type_vector(ROWS, COLUMNS/world_size, COLUMNS, MPI_INT, &COLUMNS_TYPE);
    MPI_Type_commit(&COLUMNS_TYPE);
    MPI_Type_create_resized(COLUMNS_TYPE, 0, COLUMNS/world_size * sizeof(int), &COLUMNS_TYPE);
    MPI_Type_commit(&COLUMNS_TYPE);

    data_per_process = ROWS * (COLUMNS / world_size);
    MPI_Scatter(contignous_space, 1, COLUMNS_TYPE, chunk, data_per_process, MPI_INT, ROOT, MPI_COMM_WORLD);
    print_received_atomically(world_rank, world_size, data_per_process);

    MPI_Barrier(MPI_COMM_WORLD);
    if (world_rank == 0) {
        printf("\n\ncolumns2:\n");
    }
    clear_chunk();
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Type_vector(ROWS, 1, COLUMNS, MPI_INT, &COLUMNS_TYPE);
    MPI_Type_commit(&COLUMNS_TYPE);
    MPI_Type_create_resized(COLUMNS_TYPE, 0, 1 * sizeof(int), &COLUMNS_TYPE);
    MPI_Type_commit(&COLUMNS_TYPE);


    MPI_Scatter(contignous_space, COLUMNS/world_size, COLUMNS_TYPE, chunk, data_per_process, MPI_INT, ROOT, MPI_COMM_WORLD);
    print_received_atomically(world_rank, world_size, data_per_process);


    MPI_Finalize();
}

