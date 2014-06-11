#ifndef __MATRIX_H__
#define __MATRIX_H__

typedef struct Matrix_INTERNAL {
    int columns, rows;
    double** data;
    double* contiguous_data;
}Matrix;

void Matrix_init(Matrix* self, int columns, int rows);
void Matrix_free(Matrix* self);
void Matrix_print(Matrix* self);
double** Matrix_get_data(Matrix* self);
double* Matrix_get_contiguous_data(Matrix* self);
int Matrix_get_columns(Matrix* self);
int Matrix_get_rows(Matrix* self);

void Matrix_fill_with_random_data(Matrix* self);


#endif //__MATRIX_H__
