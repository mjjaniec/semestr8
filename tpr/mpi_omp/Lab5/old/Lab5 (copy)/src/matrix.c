#include "matrix.h"
#include "commons.h"

#include <stdio.h>
#include <stdlib.h>



void Matrix_init(Matrix *self, int columns, int rows) {
    self->columns = columns;
    self->rows = rows;
    self->contiguous_data = (double*)malloc(rows * columns * sizeof(double));
    self->data = (double**)malloc(rows * sizeof(double*));
    for(int i = 0; i < rows; ++ i) {
        self->data[i] = self->contiguous_data + i * columns;
    }
}

void Matrix_free(Matrix *self) {
    free(self->contiguous_data);
    free(self->data);
}


inline double** Matrix_get_data(Matrix *self) {
    return self->data;
}

inline double* Matrix_get_contiguous_data(Matrix *self) {
    return self->contiguous_data;
}

void Matrix_print(Matrix *self) {
    printf("matrix: columns: %d, rows: %d\n", self->columns, self->rows);
    for (int row = 0; row < self->rows; ++row) {
        for (int column = 0; column < self->columns; ++column) {
            printf("%5f ", (float)self->data[row][column]);
        }
        printf("\n");
    }
    printf("\n");
}

inline int Matrix_get_columns(Matrix *self) {
    return self->columns;
}

inline int Matrix_get_rows(Matrix *self) {
    return self->rows;
}

void Matrix_fill_with_random_data(Matrix *self) {
    int rows, columns;
    double **data;
    data = Matrix_get_data(self);
    rows = Matrix_get_rows(self);
    columns = Matrix_get_columns(self);
    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            data[row][column] = random_double();
        }
    }
}
