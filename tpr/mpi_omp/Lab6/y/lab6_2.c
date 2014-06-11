#include <omp.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>

int max_value;
int processes;

void print_tab(float *tab, int n) {
    int lb = 10;
    for (int i = 0; i < n; ++i) {
        printf("%6.1f", tab[i]);
        if (i % lb == lb - 1) {
            printf("\n");
        }
    }
    if (n % lb != lb - 1) {
        printf("\n");
    }
}

float* generate_random_nums(int n) {
    float* result = (float*) malloc(n * sizeof(float));
    for (int i = 0; i < n; ++i) {
        result[i] = rand() % max_value;
    }
    return result;
}

bool check_sort_validity(float *tab, int n) {
    for (int i = 1; i < n; ++i) {
        if (tab[i-1] > tab[i]) {
            return false;
        }
    }
    return true;
}



void count_sort(float* tab, int n) {
    int* values = (int*) malloc(max_value * sizeof(int));
    int i, index;
    #pragma omp parallel default(none) shared(values, max_value, index, tab, n) private(i)
    {
        #pragma omp for
        for (i = 0; i < max_value; ++i) {
            values[i] = 0;
        }
    }
        //#pragma omp for
        for (i = 0; i < n; ++i) {
            #pragma omp atomic
            ++values[(int)tab[i]];
        }

        for (i = 0, index = -1; i < max_value; ++i) {
            //#pragma omp for
            for (int k = 0; k < values[i]; ++k) {
              //  #pragma omp critical
                tab[++index] = i;
            }
        }


}

float wall_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (float)tv.tv_sec + ((float)tv.tv_usec / 1000000.0f);
}


int main(int argc, char** argv)
{
    if (argc != 3) {
        printf("usage: %s [tab_size] [processes]\n", argv[0]);
        return -1;
    }

    int n = atoi(argv[1]);
    processes = atoi(argv[2]);
    omp_set_num_threads(processes);
    max_value = sqrt(n);
    if (max_value < 10) {
        max_value = 10;
    }
    srand(time(NULL));

    float* tab = generate_random_nums(n);
    //printf("before sort:\n");
    //print_tab(tab, n);

    struct timeval  tv0, tv1;
    gettimeofday(&tv0, NULL);
    count_sort(tab, n);
    gettimeofday(&tv1, NULL);
    //print_matrix("C", C, N, 1);
    float time = tv1.tv_sec - tv0.tv_sec + (tv1.tv_usec - tv0.tv_usec) / 1000000.0f;

    //printf("after sort:\n");
    //print_tab(tab, n);

    printf("computed in: %f, result is %s\n", time, (check_sort_validity(tab, n) ? "valid" : "invalid"));

    return 0;
}

