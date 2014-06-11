#include <sys/time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>

/**
   Asumption: values in range(0,1)
  */

typedef struct ___Bucket {
    float* data;
    int used;
    int size;
    omp_lock_t lock;
}Bucket;

void Bucket_init(Bucket* self) {
    self->used = 0;
    self->size = 4;
    self->data = (float*) malloc(self->size * sizeof(float));
    omp_init_lock(&self->lock);
}

void Bucket_put(Bucket* self, float value) {
    omp_set_lock(&self->lock);
    if (self->used == self->size) {
        self->size = 2 * self->size;
        float* new_data = (float*) malloc(self->size * sizeof(float));
        memcpy(new_data, self->data, self->used * sizeof(float));
        free(self->data);
        self->data = new_data;
    }
    self->data[self->used++] = value;
    omp_unset_lock(&self->lock);
}

void print_tab(float *tab, int n) {
    int lb = 10;
    for (int i = 0; i < n; ++i) {
        printf("  %.6f", tab[i]);
        if (i % lb == lb - 1) {
            printf("\n");
        }
    }
    if (n % lb != lb - 1) {
        printf("\n");
    }
}

void Bucket_sort(Bucket* self) {
    float* data = self->data;
    float tmp;
    int n = self->used;
    for (int i = 1; i<n; ++i) {
        for (int j = i; j > 0 && data[j-1] > data[j]; --j) {
            tmp = data[j];
            data[j] = data[j-1];
            data[j-1] = tmp;
        }
    }
}

void Bucket_free(Bucket* self) {
    free(self->data);
    omp_destroy_lock(&self->lock);
}



void bucket_sort(float* tab, int n, int buckets_number) {
    Bucket* buckets = (Bucket*) malloc(buckets_number * sizeof(Bucket));
    int* offset = (int*) malloc(buckets_number * sizeof(int));
    int i;

    #pragma omp parallel default(none) private(i) shared(buckets_number, buckets, n, tab, offset)
    {
        #pragma omp for schedule(static)
        for (i = 0; i < buckets_number; ++i) {
            Bucket_init(buckets + i);
        }

        #pragma omp for schedule(guided, 10)
        for (int i = 0; i < n; ++i) {
            int index = buckets_number * tab[i];
            Bucket_put(buckets + index, tab[i]);
        }

        #pragma omp single nowait
        {
            offset[0] = 0;
            for (i = 1; i < buckets_number; ++i) {
                offset[i] = offset[i-1] + buckets[i-1].used;
            }
        }

        #pragma omp for schedule(guided, 10)
        for (i = 0; i < buckets_number; ++i) {
            Bucket_sort(buckets + i);
        }

        #pragma omp for schedule(guided, 10)
        for (i = 0; i < buckets_number; ++i) {
            memcpy(tab + offset[i], buckets[i].data, buckets[i].used * sizeof(float));
            Bucket_free(buckets + i);
        }
    }

    free(offset);
    free(buckets);
}

float* init_tab(int size) {
    float* tab = (float*) malloc(size * sizeof(float));
    for(int i = 0; i < size; ++i) {
        tab[i] = ((float)rand()) / RAND_MAX;
    }
    return tab;
}

bool check_sort_validity(float *tab, int n) {
    for (int i = 1; i < n; ++i) {
        if (tab[i-1] > tab[i]) {
            return false;
        }
    }
    return true;
}


int main(int argc, char** argv)
{
    srand(time(NULL));
    int size, buckets_number, processes_number;
    if (argc != 4) {
        printf("usage: %s tab_size number_of_buckets number_of_processes\n", argv[0]);
        return -1;
    }
    size = atoi(argv[1]);
    buckets_number = atoi(argv[2]);
    processes_number = atoi(argv[3]);

    omp_set_num_threads(processes_number);

    struct timeval tv0, tv1;
    float* tab = init_tab(size);
   // print_tab(tab, size);
    gettimeofday(&tv0, NULL);
    bucket_sort(tab, size, buckets_number);
    gettimeofday(&tv1, NULL);
   // print_tab(tab, size);
    float time = tv1.tv_sec - tv0.tv_sec + (tv1.tv_usec - tv0.tv_usec) / 1000000.0f;
    printf("time: %f\n", time);
    printf("result is: %s\n", check_sort_validity(tab, size) ? "valid" : "invalid");
    free(tab);
    return 0;
}

