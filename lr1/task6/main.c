#include <utils.h>

#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

// 1
typedef struct {
    double x, y;
} point_t;

static double cross(const point_t *a, const point_t *b, const point_t *c)
{
    return (b->x - a->x) * (c->y - a->y)
         - (b->y - a->y) * (c->x - a->x);
}

status_t is_convex(const size_t n, bool *out, ...)
{
    if (out == NULL)  return STATUS_ERR_NULL_ARG;
    if (n < 3)        return STATUS_ERR_INVALID;

    point_t pts[n];

    va_list args;
    va_start(args, out);

    for (size_t i = 0; i < n; ++i) {
        pts[i].x = va_arg(args, double);
        pts[i].y = va_arg(args, double);
    }

    va_end(args);


    *out = true;

    int sign = 0;

    for (size_t i = 0; i < n; ++i) {
        const point_t *a = &pts[i];
        const point_t *b = &pts[(i + 1) % n];
        const point_t *c = &pts[(i + 2) % n];

        double cr = cross(a, b, c);

        if (fabs(cr) <= 1e-10) // чтобы каждый раз не вводить эпсилон установлен такой
            continue;

        int s = (cr > 0.0) ? 1 : -1;

        if (sign == 0) {
            sign = s;
        } else if (s != sign) {
            *out = false;
            return STATUS_OK;
        }
    }

    return STATUS_OK;
}

// 2
status_t polynomial(const double x, const int n, double *out, ...) {
    if (out == NULL) return STATUS_ERR_NULL_ARG;

    va_list args;
    va_start(args, out);

    double result = va_arg(args, double);

    for (int i = 1; i <= n; ++i) {
        double coef = va_arg(args, double);
        result = result * x + coef;
    }

    va_end(args);

    *out = result;
    return STATUS_OK;
}

// 3
typedef struct {
    char repr[64];
    size_t value;
    bool kprkr;
} kaprekar_t;

static bool is_kaprekar(unsigned long long n, int base)
{
    unsigned long long sq = n * n;

    unsigned long long pow_base = base;

    while (pow_base <= sq) {
        unsigned long long left  = sq / pow_base;
        unsigned long long right = sq % pow_base;

        if (left + right == n)
            return true;

        if (pow_base > sq / base)
            break;

        pow_base *= base;
    }

    if (n == 0 || n == 1)
        return true;

    return false;
}

status_t find_kaprekar(const size_t base, const size_t count, kaprekar_t *out, ...)
{
    if (base < 2 || base > 36)  return STATUS_ERR_INVALID;
    if (out == NULL)  return STATUS_ERR_NULL_ARG;

    va_list args;
    va_start(args, out);

    status_t result = STATUS_OK;

    for (size_t i = 0; i < count; ++i) {
        const char *s = va_arg(args, const char *);

        if (s == NULL) {
            result = STATUS_ERR_NULL_ARG;
            break;
        }

        strncpy(out[i].repr, s, 64);
        out[i].repr[63] = '\0';

        size_t n;
        status_t rc = parse_in_base(s, base, &n);
        if (rc != STATUS_OK) {
            result = rc;
            break;
        }

        out[i].value = n;
        out[i].kprkr = is_kaprekar(n, base);
    }

    va_end(args);
    return result;

}

// 4
status_t geometric_mean(const int n, double *out, ...) {
    if (out == NULL)  return STATUS_ERR_NULL_ARG;
    if (n <= 0)       return STATUS_ERR_INVALID;

    va_list args;
    va_start(args, out);

    double sum_log = 0.0;

    for (size_t i = 0; i < n; ++i) {
        sum_log += log(va_arg(args, double));
    }

    va_end(args);

    *out = exp(sum_log / (double)n);
    return STATUS_OK;
}

// 5
status_t ipow_rec(double x, int n, double *out)
{
    if (out == NULL)  return STATUS_ERR_NULL_ARG;

    if (n == 0) {
        *out = 1.0;
        return STATUS_OK;
    }

    if (n < 0) {
        if (x == 0.0)
            return STATUS_ERR_INVALID;

        double pos;
        status_t rc = ipow_rec(x, -n, &pos);
        if (rc != STATUS_OK) return rc;

        *out = 1.0 / pos;
        return STATUS_OK;
    }

    double half;
    status_t rc = ipow_rec(x, n / 2, &half);
    if (rc != STATUS_OK) return rc;

    double result = half * half;

    if (n % 2 == 1)
        result *= x;

    *out = result;
    return STATUS_OK;
}

//6
typedef double (*func_t)(double);

static double f1(double x) { return x * x - 2.0; }

status_t bisection(func_t f, double a, double b, double eps, double *out)
{
    if (f == NULL || out == NULL)  return STATUS_ERR_NULL_ARG;
    if (eps <= 0.0)                return STATUS_ERR_INVALID;
    if (b <= a)                    return STATUS_ERR_INVALID;

    double fa = f(a);
    double fb = f(b);

    if (fa * fb >= 0.0)
        return STATUS_ERR_INVALID;

    const int LIMIT = 1000000;

    for (int n = 0; n < LIMIT; ++n) {
        double mid = 0.5 * (a + b);
        double fm  = f(mid);

        if (fm == 0.0) {
            *out = mid;
            return STATUS_OK;
        }

        if (fa * fm < 0.0) {
            b  = mid;
            fb = fm;
        } else {
            a  = mid;
            fa = fm;
        }

        if ((b - a) < eps) {
            *out = 0.5 * (a + b);
            return STATUS_OK;
        }
    }

    return STATUS_ERR_RANGE;
}

int main(int argc, char *argv[]) {
    status_t rc;

    // 1
    bool convex_res;
    rc = is_convex(4, &convex_res, 0.0, 0.0, 5.0, 0.0, 5.0, 5.0, 1.0, 4.0);

    switch (rc) {
        case STATUS_OK:
            break;
        case STATUS_ERR_INVALID:
            fprintf(stderr, "Для работы программы необходимо как минимум 3 точки\n");
		    return 1;
        case STATUS_ERR_NULL_ARG:
            fprintf(stderr, "Внутренняя ошибка\n");
		    return 1;
        default:
            fprintf(stderr, "Неизвестная ошибка\n");
		    return 1;
    }

    if (convex_res) {
        printf("1. Этот многоугольник является выпуклым\n");
    } else {
        printf("1. Этот многоугольник не является выпуклым\n");
    }

    // 2
    double x = 1.5;
    int n = 4;
    double cfs[5] = {  };

    double polynomial_res;
    rc = polynomial(1.5, 4, &polynomial_res, 1.0, -2.0, 3.0, -4.0, 5.0);

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

    printf("2. %f\n", polynomial_res);

    // 3
    const size_t kapr_count = 6;
    kaprekar_t capr_res[kapr_count];
    rc = find_kaprekar(16, kapr_count, capr_res, "1", "6", "A", "FF", "2", "5");

    switch (rc) {
        case STATUS_OK:
            printf("3. \n");
            for (int i = 0; i < (int)kapr_count; i++) {
                printf("%s %zu %d\n", capr_res[i].repr, capr_res[i].value, capr_res[i].kprkr);
            }
            break;
        case STATUS_ERR_NULL_ARG:
            fprintf(stderr, "Внутренняя ошибка\n");
		    return 1;
        default:
            fprintf(stderr, "Неизвестная ошибка\n");
		    return 1;
    }

    // 4
    double geometric_ans;
    rc = geometric_mean(3, &geometric_ans, 1.0, 3.0, 9.0);
    switch (rc) {
        case STATUS_OK:
            printf("4. %f\n", geometric_ans);
            break;
        case STATUS_ERR_NULL_ARG:
            fprintf(stderr, "Внутренняя ошибка\n");
		    return 1;
        default:
            fprintf(stderr, "Неизвестная ошибка\n");
		    return 1;
    }

    // 5
    double ipow_ans;
    rc = ipow_rec(2.0, 10, &ipow_ans);
    switch (rc) {
        case STATUS_OK:
            printf("5. %f\n", ipow_ans);
            break;
        case STATUS_ERR_NULL_ARG:
            fprintf(stderr, "Внутренняя ошибка\n");
		    return 1;
        default:
            fprintf(stderr, "Неизвестная ошибка\n");
		    return 1;
    }

    // 6
    double bisection_ans;
    rc = bisection(f1, 1.0, 2.0, 1e-5, &bisection_ans);
    switch (rc) {
        case STATUS_OK:
            printf("6. %f\n", bisection_ans);
            break;
        case STATUS_ERR_NULL_ARG:
            fprintf(stderr, "Внутренняя ошибка\n");
		    return 1;
        default:
            fprintf(stderr, "Неизвестная ошибка\n");
		    return 1;
    }
}