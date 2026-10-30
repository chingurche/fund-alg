#include "../utils.h"

#include <math.h>
#include <stdarg.h>
#include <stdbool.h>

static status_t is_finite(double x, int base, double eps, bool *out) {
    if (out == NULL)    return STATUS_ERR_NULL_ARG;
    if (base < 2)       return STATUS_ERR_RANGE;
    if (eps <= 0.0)     return STATUS_ERR_RANGE;

    double frac      = x;
    double threshold = eps;

    while (threshold < 1.0) {
        frac *= base;
        double digit = floor(frac);
        frac -= digit;

        if (frac < eps) {
            *out = true;
            return STATUS_OK;
        }

        threshold *= base;
    }

    *out = false;
    return STATUS_OK;
}

static status_t finite_all(size_t base, bool *out, size_t count, ...) {
    if (out == NULL)                return STATUS_ERR_NULL_ARG;
    if (count < 1)                  return STATUS_ERR_INVALID;
    status_t rc;

    double value[count];

    va_list ap;
    va_start(ap, count);

    for (int i = 0; i < count; i++) {
        double v = va_arg(ap, double);
        if (v <= 0.0 || v >= 1.0)   return STATUS_ERR_RANGE;
        value[i] = v;
    }

    va_end(ap);

    static const double eps = 1e-10;
    for (int i = 0; i < count; i++) {
        rc = is_finite(value[i], base, eps, &out[i]);
        if (rc) return rc;
    } 

    return STATUS_OK;
}

int main(int argc, char *argv) {
    status_t rc;

    size_t count = 3;
    double prop_fract[] = {
        0.111111111111111111,
        0.25,
        0.5
    };
    // для того чтобы записать например 1/9
    // нужно продлить запись до вышеуказанного eps

    bool finite[count];
    rc = finite_all(3, finite, count, 
                    prop_fract[0], 
                    prop_fract[1], 
                    prop_fract[2]); // 3 и 20
    switch (rc) {
        case STATUS_OK: 
            break;
        case STATUS_ERR_RANGE:
            fprintf(stderr, "Какое-то там число не входит в допустимый промежуток.\n");
            return 1;
        case STATUS_ERR_INVALID:
            fprintf(stderr, "В качестве аргумента было передано недействительное число.\n");
            return 1;
        case STATUS_ERR_NULL_ARG:
            fprintf(stderr, "Произошла внутренняя ошибка.\n");
            return 1;
        default:
            fprintf(stderr, "Произошла неизвестная ошибка.\n");
            return 1;
    }

    for (int i = 0; i < count; i++) {
        printf("%.10f %s.\n", 
            prop_fract[i], 
            finite[i] ? "is finite" : "isn't finite");
    }

    return 0;
}