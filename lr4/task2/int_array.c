#include "int_array.h"
#include "../status.h"
#include "../std_parse.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <ctype.h>

static int rand_range(int lo, int hi) {
    int range = hi - lo + 1;
    int limit = RAND_MAX - (RAND_MAX % range) - 1;
    int r;
    do {
        r = rand();
    } while (r > limit);
    return lo + r % range;
}

status_t IntArray_create_empty(IntArray **out) {
    if (!out) return STATUS_ERR_NULL_ARG;
    
    IntArray *arr = malloc(sizeof(*arr));
    if (!arr) return STATUS_ERR_ALLOC;

    arr->data = NULL;
    arr->len = 0;

    *out = arr;
    return STATUS_OK;
}

static status_t append_int(
    int **data,
    size_t *len,
    size_t *capacity,
    int value
) {
    if (*len == *capacity) {
        size_t new_capacity = *capacity ? *capacity * 2 : 16;

        if (new_capacity < *capacity ||
            new_capacity > SIZE_MAX / sizeof(int))
            return STATUS_ERR_ALLOC;

        int *tmp = realloc(*data, new_capacity * sizeof(int));

        if (!tmp) return STATUS_ERR_ALLOC;

        *data = tmp;
        *capacity = new_capacity;
    }

    (*data)[(*len)++] = value;

    return STATUS_OK;
}

status_t IntArray_load(IntArray *arr, char *path) {
    if (!arr || !path) return STATUS_ERR_NULL_ARG;

    FILE *f = fopen(path, "r");

    if (!f) return STATUS_ERR_OPEN;

    int *data = NULL;
    size_t len = 0;
    size_t capacity = 0;

    char *token = NULL;
    size_t token_len = 0;
    size_t token_capacity = 0;

    status_t rc = STATUS_OK;

    int c;

    while ((c = fgetc(f)) != EOF) {
        if (isspace((unsigned char)c)) {
            if (token_len > 0) {
                token[token_len] = '\0';

                int value;

                if (!parse_int(token, &value)) {
                    rc = append_int(
                        &data, &len, &capacity, value
                    );

                    if (rc != STATUS_OK)
                        break;
                }

                token_len = 0;
            }

            continue;
        }

        if (token_len + 1 >= token_capacity) {
            size_t new_capacity = token_capacity
                ? token_capacity * 2
                : 32;

            if (new_capacity < token_capacity) {
                rc = STATUS_ERR_ALLOC;
                break;
            }

            char *tmp = realloc(token, new_capacity);

            if (!tmp) {
                rc = STATUS_ERR_ALLOC;
                break;
            }

            token = tmp;
            token_capacity = new_capacity;
        }

        token[token_len++] = (char)c;
    }

    if (rc == STATUS_OK && !ferror(f) && token_len > 0) {
        token[token_len] = '\0';

        int value;

        if (!parse_int(token, &value)) {
            rc = append_int(
                &data, &len, &capacity, value
            );
        }
    }

    if (ferror(f) && rc == STATUS_OK)
        rc = STATUS_ERR_INVALID;

    if (fclose(f) != 0 && rc == STATUS_OK)
        rc = STATUS_ERR_INVALID;

    free(token);

    if (rc != STATUS_OK) {
        free(data);
        return rc;
    }

    free(arr->data);

    arr->data = data;
    arr->len = len;

    return STATUS_OK;
}

status_t IntArray_save(IntArray *arr, char *path) {
    if (!arr || !path) return STATUS_ERR_NULL_ARG;

    FILE *f = fopen(path, "w");

    if (!f) return STATUS_ERR_INVALID;

    status_t rc = STATUS_OK;

    for (size_t i = 0; i < arr->len; ++i) {
        if (fprintf(f, "%d\n", arr->data[i]) < 0) {
            rc = STATUS_ERR_INVALID;
            break;
        }
    }

    if (fclose(f) != 0 && rc == STATUS_OK)
        rc = STATUS_ERR_INVALID;

    return rc;
}

status_t IntArray_rand(IntArray *arr, size_t count, int lb, int rb) {
    if (!arr) return STATUS_ERR_NULL_ARG;
    if (lb > rb) return STATUS_ERR_RANGE;

    int *tmp = malloc(count * sizeof(*tmp));
    if (!tmp) return STATUS_ERR_ALLOC;

    for (size_t i = 0; i < count; i++) tmp[i] = rand_range(lb, rb);

    free(arr->data);
    arr->data = tmp;
    arr->len = count;
    return STATUS_OK;
}

status_t IntArray_concat(IntArray *a_arr, IntArray *b_arr) {
    if (!a_arr || !b_arr) return STATUS_ERR_NULL_ARG;

    size_t new_len = a_arr->len + b_arr->len;
    int *tmp = malloc(new_len * sizeof(*tmp));
    if (!tmp) return STATUS_ERR_ALLOC;

    for (size_t i = 0; i < a_arr->len; i++) tmp[i] = a_arr->data[i];
    for (size_t i = 0; i < b_arr->len; i++) tmp[a_arr->len+i] = b_arr->data[i];

    free(a_arr->data);
    a_arr->data = tmp;
    a_arr->len = new_len;
    return STATUS_OK;
}

status_t IntArray_free(IntArray *arr) {
    if (!arr) return STATUS_ERR_NULL_ARG;

    free(arr->data);
    arr->data = NULL;
    arr->len = 0;
    return STATUS_OK;
}

status_t IntArray_remove(IntArray *arr, size_t idx, size_t count) {
    if (!arr) return STATUS_ERR_NULL_ARG;
    if (idx >= arr->len || count > arr->len - idx) return STATUS_ERR_RANGE;

    size_t new_len = arr->len - count;
    int *tmp = malloc(new_len * sizeof(*tmp));
    if (!tmp) return STATUS_ERR_ALLOC;

    for (size_t i = 0; i < idx; i++) tmp[i] = arr->data[i];
    for (size_t i = idx + count; i < arr->len; i++) tmp[i-count] = arr->data[i];

    free(arr->data);
    arr->data = tmp;
    arr->len = new_len;
    return STATUS_OK;
}

status_t IntArray_copy(IntArray *a_arr, size_t start_idx, size_t end_idx, IntArray *b_arr) {
    if (!a_arr || !b_arr) return STATUS_ERR_NULL_ARG;
    if (start_idx > end_idx) return STATUS_ERR_RANGE;
    
    size_t new_len = end_idx - start_idx + 1;
    int *tmp = malloc(new_len * sizeof(*tmp));
    if (!tmp) return STATUS_ERR_ALLOC;

    for (size_t i = 0; i < new_len; i++) tmp[i] = a_arr->data[i+start_idx];

    free(b_arr->data);
    b_arr->data = tmp;
    b_arr->len = new_len;
    return STATUS_OK;
}

static bool valid_order(SortOrder order) {
    return (order == InAscending) || (order == InDescending);
}

static int compare_int_asc(const void *a, const void *b) {
    const int *ia = (const int *)a;
    const int *ib = (const int *)b;
    
    if (*ia < *ib) return -1;
    if (*ia > *ib) return  1;
    return 0;
}

static int compare_int_dsc(const void *a, const void *b) {
    const int *ia = (const int *)a;
    const int *ib = (const int *)b;
    
    if (*ia < *ib) return  1;
    if (*ia > *ib) return -1;
    return 0;
}

status_t IntArray_sort(IntArray *arr, SortOrder order) {
    if (!arr) return STATUS_ERR_NULL_ARG;
    if (!valid_order(order)) return STATUS_ERR_INVALID;

    int *tmp = malloc(arr->len * sizeof(*tmp));
    if (!tmp) return STATUS_ERR_ALLOC;

    for (int i = 0; i < arr->len; i++) tmp[i] = arr->data[i];

    if (order == InAscending) 
        qsort(tmp, arr->len, sizeof(*tmp), compare_int_asc);
    else if (order == InDescending)
        qsort(tmp, arr->len, sizeof(*tmp), compare_int_dsc);

    free(arr->data);
    arr->data = tmp;
    return STATUS_OK;
}
 
status_t IntArray_shuffle(IntArray *arr) {
    if (!arr) return STATUS_ERR_NULL_ARG;
    if (arr->len < 2) return STATUS_OK;

    for (size_t i = arr->len - 1; i > 0; i--) {
        size_t j = (size_t)rand() % (i + 1);

        int tmp = arr->data[i];
        arr->data[i] = arr->data[j];
        arr->data[j] = tmp;
    }

    return STATUS_OK;
}

static int get_mode(IntArray *arr) {
    int freq[arr->len];

    for (size_t i = 0; i < arr->len; i++) {
        freq[i] = 0;
        for (size_t j = 0; j < arr->len; j++) {
            if (arr->data[i] == arr->data[j]) {
                freq[i] += 1;
            }
        }
    }

    int mode_idx = 0;
    for (size_t i = 1; i < arr->len; i++) {
        if (freq[i] > freq[mode_idx]) {
            mode_idx = i;
        } else if (freq[i] == freq[mode_idx] &&
            arr->data[i] > arr->data[mode_idx]) {
            mode_idx = i;   
        }
    }

    return arr->data[mode_idx];
}

status_t IntArray_stats(IntArray *arr) {
    if (!arr) return STATUS_ERR_NULL_ARG;

    printf("len: %lu\n", arr->len);
    if (arr->len == 0) return STATUS_OK;

    size_t idx_max = 0;
    size_t idx_min = 0;
    int sum = 0;
    for (size_t i = 0; i < arr->len; i++) {
        if (arr->data[idx_max] < arr->data[i]) idx_max = i;
        if (arr->data[idx_min] > arr->data[i]) idx_min = i;
        sum += arr->data[i];
    }
    float avg = sum / arr->len;
    float max_dev = 0;
    for (size_t i = 0; i < arr->len; i++) {
        int value = arr->data[i];
        float dev = (avg > value) ? (avg - value) : (value - avg);
        if (max_dev < dev) max_dev = dev;
    }

    printf("max: %d\n", arr->data[idx_max]);
    printf("min: %d\n", arr->data[idx_min]);
    printf("mode: %d\n", get_mode(arr));
    printf("avg: %.3f\n", avg);
    printf("max dev: %.3f\n", max_dev);
    
    return STATUS_OK;
}

status_t IntArray_print_single(IntArray *arr, size_t idx) {
    if (!arr) return STATUS_ERR_NULL_ARG;
    if (idx >= arr->len) return STATUS_ERR_RANGE;

    printf("%d\n", arr->data[idx]);

    return STATUS_OK;
}

status_t IntArray_print_range(IntArray *arr, size_t start, size_t end) {
    if (!arr) return STATUS_ERR_NULL_ARG;
    if (start > end) return STATUS_ERR_RANGE;
    if (end >= arr->len) return STATUS_ERR_RANGE;

    for (size_t i = start; i <= end; i++) 
        printf("%d ", arr->data[i]);
    printf("\n");

    return STATUS_OK;
}

status_t IntArray_print_all(IntArray *arr) {
    if (!arr) return STATUS_ERR_NULL_ARG;

    for (size_t i = 0; i < arr->len; i++) 
        printf("%d ", arr->data[i]);
    printf("\n");

    return STATUS_OK;
}