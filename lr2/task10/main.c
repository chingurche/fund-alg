#include "../utils.h"

#include <stdarg.h>
#include <math.h>

static status_t find_coeffs(double eps, double a, double **out, size_t count, ...) {
    if (out == NULL)    return STATUS_ERR_NULL_ARG;
    if (count == 0)     return STATUS_ERR_INVALID;
    
    double *tmp = malloc(count * sizeof *tmp);
    if (!tmp)           return STATUS_ERR_ALLOC;

    va_list ap;
    va_start(ap, count);
    for (size_t i = 0; i < count; i++) {
        tmp[i] = va_arg(ap, double);
    }
    va_end(ap);

    size_t n = count - 1;
    for (size_t i = 0; i <= n; i++)
        for (size_t j = n; j > i; j--)
            tmp[j - 1] += a * tmp[j];

    for (size_t i = 0; i < count; i++)
        if (fabs(tmp[i]) < eps)
            tmp[i] = 0.0;

    *out = tmp;

    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    status_t rc;

    double eps = 1e-10;
    double a = 5.0;
    int count = 4;
    double coeff[] = {
        30.0, 5.0, 6.0, 1.0
    };
    for (size_t i = 0; i < count; i++)
        printf( "%.0fx^%zu%s", 
                coeff[i], i, 
                (i+1 == count) ? "" : " + ");
    printf("\n");

    double *new_coeff;
    rc = find_coeffs(
        eps, 5.0, &new_coeff, count,
        coeff[0], coeff[1], coeff[2], coeff[3]
    );

    if (rc) {
        fprintf(stderr, "Произошла ошибка при выполнении функции.");
        return 1;
    }

    for (size_t i = 0; i < count; i++)
        printf( "%.0f(x - %.0f)^%zu%s", 
                new_coeff[i], a, i, 
                (i+1 == count) ? "" : " + ");
    printf("\n");


    free(new_coeff);

    return 0;
}