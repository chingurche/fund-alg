#include <utils.h>

#include <stdio.h>
#include <math.h>

status_t polynomial(const double x, const int n, const double *cfs, double *out) {
    if (cfs == NULL || out == NULL) return STATUS_ERR_NULL_ARG;

    double result = 0;
    for (int i = 0; i <= n; i++) {
        result += cfs[i] * pow(x, n - i);
    }

    *out = result;
    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    status_t rc;

    if (argc < 4) {
        fprintf(stderr, "Формат запуска программы: [программа] [x] [n(>=0)] [a] ...\n");
		return 1;
    }
    
    double x;
    rc = parse_double(argv[1], &x);
    if (rc != STATUS_OK) {
        fprintf(stderr, "Переменная x должна быть вещественного типа\n");
        return 1;
    }
    
    int n;
    rc = parse_int(argv[2], &n);
    if (rc != STATUS_OK) {
        fprintf(stderr, "Переменная n должна быть целого типа\n");
        return 1;
    }
    if (n < 0) {
        fprintf(stderr, "n должен быть неотрицательным\n");
		return 1;
    }
    if (n != argc - 4) {
        fprintf(stderr, "Коэффициентов должно быть %d (n+1), а введено %d\n", n+1, argc - 3);
        return 1;
    }
    
    
    double cfs[n+1];
    for (int i = 0; i < n+1; i++) {
        rc = parse_double(argv[3 + i], &cfs[i]);
        if (rc != STATUS_OK) {
            fprintf(stderr, "Коэффициенты должны быть вещественного типа\n");
            return 1;
        }
    }

    double result;
    rc = polynomial(x, n, cfs, &result);

    switch (rc) {
        case STATUS_OK:
            break;
        case STATUS_ERR_NULL_ARG:
            fprintf(stderr, "Внутренняя ошибка\n");
		    return 1;
        default:
            fprintf(stderr, "Неизвестная ошибка\n");
		    return 1;
    }

    printf("%f\n", result);
}