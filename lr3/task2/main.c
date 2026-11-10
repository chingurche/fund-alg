#include "models.h"
#include <stdlib.h>
#include <stdio.h>

status_t getMaxPresentations(
    Presentation * const presv, 
    int const presc, 
    int const concpres, 
    int * const out) {
    if (presv == NULL || out == NULL) {
        return STATUS_ERR_NULL_ARG;
    }
    if (presc <= 0 || concpres <= 0) {
        return STATUS_ERR_RANGE;
    }

    qsort(presv, presc, sizeof(Presentation), comparePresentation);

    int *hall_ends = calloc(concpres, sizeof(int));
    if (hall_ends == NULL) {
        return STATUS_ERR_ALLOC;
    }

    int max_scheduled = 0;

    for (int i = 0; i < presc; i++) {
        int start = toMinutes(presv[i].start_time);
        int finish = toMinutes(presv[i].finish_time);

        int best_hall = -1;
        int best_hall_end = -1;

        for (int j = 0; j < concpres; j++) {
            if (hall_ends[j] <= start) {
                if (hall_ends[j] > best_hall_end) {
                    best_hall_end = hall_ends[j];
                    best_hall = j;
                }
            }
        }

        if (best_hall != -1) {
            hall_ends[best_hall] = finish;
            max_scheduled++;
        }
    }

    free(hall_ends);
    *out = max_scheduled;

    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    status_t rc;
    char line[BUFSIZ];

    Presentation *presv;
    int presc = 0;
    int concpres;

    if (fgets(line, sizeof line, stdin) == NULL) {
        fprintf(stderr, "Пустой ввод.\n");
        return 1;
    }

    if (sscanf(line, "%d %d", &presc, &concpres) != 2 || presc <= 0) {
        fprintf(stderr, "Формат ввода первой строки: %%d %%d.\n");
        return 1;
    }

    presv = malloc(presc * sizeof(*presv));
    if (!presv) {
        fprintf(stderr, "Не удалось выделить память.\n");
        return 1;
    }
    
    for (int i = 0; i < presc; i++) {
        if (fgets(line, sizeof line, stdin) == NULL) {
            fprintf(stderr, "Неожиданный конец файла. Ожидалось строк: %d\n", presc);
            free(presv);
            return 1;
        }

        int id, time[4];
        if (sscanf(
            line, "%d %d:%d %d:%d", 
            &id, &time[0], &time[1],
            &time[2], &time[3]) != 5) {
            fprintf(stderr, "Формат ввода первой строки: %%d %%d:%%d %%d:%%d.\n");
            free(presv);
            return 1;
        }

        Time start_time, finish_time;
        rc = newTime(time[0], time[1], &start_time);
        if (rc) {
            fprintf(stderr, "Не удалось создать Time.\n");
            free(presv);
            return 1;
        }
        rc = newTime(time[2], time[3], &finish_time);
        if (rc) {
            fprintf(stderr, "Не удалось создать Time.\n");
            free(presv);
            return 1;
        }
        rc = newPresentation(
            id,
            start_time,
            finish_time,
            &presv[i]
        );
        if (rc) {
            fprintf(stderr, "Не удалось создать Presentation.\n");
            free(presv);
            return 1;
        }
    }

    int max;
    rc = getMaxPresentations(presv, presc, concpres, &max);
    free(presv);

    switch (rc) {
        case STATUS_OK:
            break;
        case STATUS_ERR_ALLOC:
            fprintf(stderr, "Не удалось выделить память.\n");
            return 1;
        case STATUS_ERR_RANGE:
            fprintf(stderr, "Значение аргумента вне допустимогго диапазона.\n");
            return 1;
        case STATUS_ERR_NULL_ARG:
            fprintf(stderr, "Произошла внутренняя ошибка.\n");
            return 1;
        default:
            fprintf(stderr, "Произошла неизвестная ошибка.\n");
            return 1;
    }

    printf("%d\n", max);

    return 0;
}