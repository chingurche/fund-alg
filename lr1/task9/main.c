#include <utils.h>

#include <stdlib.h>
#include <time.h>
#include <math.h>

status_t switch_max_min(int *arr, const size_t size) {
    if (arr == NULL)  return STATUS_ERR_NULL_ARG;
    if (size == 0)    return STATUS_ERR_INVALID;
    
    int *max = &arr[0];
    int *min = &arr[0];
    printf("%d ", arr[0]);
    for (size_t i = 1; i < size; i++) {
        printf("%d ", arr[i]);

        if (*max < arr[i])
            max = &arr[i];
        if (*min > arr[i])
            min = &arr[i];
    }

    int temp = *max;
    *max = *min;
    *min = temp;

    printf("\n%d и %d поменялись местами\n", *max, *min);
    for (size_t i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    return STATUS_OK;
}

int rand_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

int main(int argc, char *argv[]) {
    srand((unsigned)time(NULL));
    if (argc != 3) {
        fprintf(stderr, "Формат запуска программы: [программа] [a] [b]\n");
        return 1;
    }

    int a, b;
    status_t rc1 = parse_int(argv[1], &a);
    status_t rc2 = parse_int(argv[2], &b);
    if (rc1 != STATUS_OK || rc2 != STATUS_OK) {
        fprintf(stderr, "a и b целые числа\n");
        return 1;
    } else if (a >= b) {
        fprintf(stderr, "a должно быть меньше b\n");
        return 1;
    }

    // 1
    printf("1. \n");
    const size_t ARRSIZ = 10;
    int arr[ARRSIZ];
    for (size_t i = 0; i < ARRSIZ; i++) {
        arr[i] = rand_range(a, b);
    }
    switch_max_min(arr, ARRSIZ);

    // 2
    size_t cap = (size_t)rand_range(10, 10000);
    int *a_arr = malloc(cap * sizeof(int));
    int *b_arr = malloc(cap * sizeof(int));
    if (a_arr == NULL || b_arr == NULL) {
        fprintf(stderr, "Не удалось выделить память\n");
        free(a_arr); free(b_arr);
        return 1;
    }
    for (size_t i = 0; i < cap; i++) {
        a_arr[i] = rand_range(-1000, 1000);
        b_arr[i] = rand_range(-1000, 1000);
    }

    int *c_arr = malloc(cap * sizeof(int));
    if (c_arr == NULL) {
        fprintf(stderr, "Не удалось выделить память\n");
        free(a_arr); free(b_arr);
        return 1;
    }
    for (size_t i = 0; i < cap; i++) {
        c_arr[i] = a_arr[i];
        int closest = 1001;
        for (int j = 0; j < (int)cap; j++) {
            if ((fabs(a_arr[i] - b_arr[j])) < (fabs(a_arr[i] - closest)))
                closest = b_arr[j];
        }
        c_arr[i] += closest;
    }

    printf("\n2. По инструкции были созданы динамические массивы A, B, C\n");

    free(a_arr); free(b_arr); free(c_arr);

    return 0;
}
