#ifndef INT_ARRAY_H
#define INT_ARRAY_H

#include "../status.h"
#include <stdlib.h>

typedef enum { InAscending, InDescending } SortOrder;

typedef struct {
    int *data;
    size_t len;
} IntArray;

status_t IntArray_create_empty(IntArray **out);

status_t IntArray_load(IntArray *arr, char *path);
status_t IntArray_save(IntArray *arr, char *path); 
status_t IntArray_rand(IntArray *arr, size_t count, int lb, int rb);
status_t IntArray_concat(IntArray *a_arr, IntArray *b_arr);
status_t IntArray_free(IntArray *arr);
status_t IntArray_remove(IntArray *arr, size_t idx, size_t count);
status_t IntArray_copy(IntArray *a_arr, size_t start_idx, size_t end_idx, IntArray *b_arr);
status_t IntArray_sort(IntArray *arr, SortOrder order);
status_t IntArray_shuffle(IntArray *arr);
status_t IntArray_stats(IntArray *arr);
status_t IntArray_print_single(IntArray *arr, size_t idx);
status_t IntArray_print_range(IntArray *arr, size_t start, size_t end);
status_t IntArray_print_all(IntArray *arr);

#endif