#include <mpi.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

const int ROOT = 0;
double random_double() {
    return ((double)abs(rand()) / RAND_MAX);
}

int64_t compute(int64_t tries) {
    int64_t hits = 0;
    double x, y;
    double t0, t1;
    //t0 = MPI_Wtime();
    for(int64_t i = 0; i < tries; ++i) {
        x = random_double();
        y = random_double();
        if (x*x + y*y <= 1) {
            ++hits;
        }
    }
  //  t1 = MPI_Wtime();
//    printf("compute: %ld time: %f\n", tries, (float)(t1 - t0));
    return hits;
}

int64_t task_size(int index, int64_t problem_size, int world_size) {
    int64_t result = problem_size / world_size;
    if (index < problem_size % world_size) {
        ++result;
    }
    return result;
}

void master(const int64_t tries, const int world_size) {
    int64_t* tasks = (int64_t*) malloc(world_size * sizeof(int64_t));
    int64_t* results = (int64_t*) malloc(world_size * sizeof(int64_t));
    int64_t task[1];
    int64_t result[1];
    double t0, t1;

    MPI_Barrier(MPI_COMM_WORLD);

    t0 = MPI_Wtime();
    for(int i = 0; i < world_size; ++i) {
        tasks[i] = task_size(i, tries, world_size);
    }

    MPI_Scatter(tasks, 1, MPI_LONG_LONG, task, 1, MPI_LONG_LONG, ROOT, MPI_COMM_WORLD); 
    result[0] = compute(task[0]); 
    MPI_Gather(result, 1, MPI_LONG_LONG, results, 1, MPI_LONG_LONG, ROOT, MPI_COMM_WORLD); 

    int64_t all = 0;
    for (int64_t i = 0; i < world_size; ++i) {
        all += results[i];
    }

    double pi = 4.0 * all / tries;
    t1 = MPI_Wtime();
    printf("pi: %f\t computation time: %12.6f\n", (float)pi, (float)(1.6*(t1 - t0)));
}

void slave(const int world_size) {
    int64_t* ignore = NULL;
    int64_t task[1];
    int64_t result[1];

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Scatter(ignore, 1, MPI_LONG_LONG, task, 1, MPI_LONG_LONG, ROOT, MPI_COMM_WORLD); 
    result[0] = compute(task[0]);
    MPI_Gather(result, 1, MPI_LONG_LONG, ignore, 1, MPI_LONG_LONG, ROOT, MPI_COMM_WORLD);
}

int main(int argc, char** argv) {

    int world_rank;
    int world_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (argc != 2) {
        printf("Provide number of tries");
        return 0;
    }

    int64_t tries = atoll(argv[1]);
    srand(time(0)^world_rank);
    
    if (world_rank == 0) {
        master(tries, world_size);
    } else {
        slave(world_size);
    }

    MPI_Finalize();

    return 0;
}

