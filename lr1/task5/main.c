#include <utils.h>

#include <math.h>
#include <stdio.h>

typedef double (*func_t)(double);

status_t sum1_series(const double x, const double eps, double *out) {
    if (out == NULL)  return STATUS_ERR_NULL_ARG;

    double term = 1.0;
    double sum  = term;
    int n = 0;
    const int LIMIT = 1000000;

    while (n < 1000000) {
        ++n;
        term *= x / (double)n;
        sum  += term;

        if (fabs(term) < eps)
            break;
    }

    if (n >= 1000000)
        return STATUS_ERR_RANGE;

    *out = sum;
    return STATUS_OK;
}

status_t sum2_series(double x, double eps, double *out) {
    if (out == NULL)  return STATUS_ERR_NULL_ARG;

    double term = 1.0;
    double sum  = term;
    int n = 0;

    while (n < 1000000) {
        term *= -x * x / ((2.0 * n + 1.0) * (2.0 * n + 2.0));
        ++n;

        sum += term;

        if (fabs(term) < eps)
            break;
    }

    if (n >= 1000000)
        return STATUS_ERR_RANGE;

    *out = sum;
    return STATUS_OK;
}

status_t sum3_series(double x, double eps, double *out) {
    if (out == NULL)  return STATUS_ERR_NULL_ARG;

    double term = 1.0;
    double sum  = term;
    int n = 0;

    while (n < 1000000) {
        double np1 = (double)(n + 1);
        term *= 27.0 * np1 * np1 * np1 * x * x
              / ((3.0 * n + 1.0) * (3.0 * n + 2.0) * (3.0 * n + 3.0));
        ++n;

        sum += term;

        if (fabs(term) < eps)
            break;
    }

    if (n >= 1000000)
        return STATUS_ERR_RANGE;

    *out = sum;
    return STATUS_OK;
}

status_t sum4_series(double x, double eps, double *out) {
    if (out == NULL)  return STATUS_ERR_NULL_ARG;

    double term = -x * x / 2.0;
    double sum  = term;
    int n = 1;

    while (n < 1000000) {
        term *= -(2.0 * n + 1.0) * x * x / (2.0 * n + 2.0);
        ++n;

        sum += term;

        if (fabs(term) < eps)
            break;
    }

    if (n >= 1000000)
        return STATUS_ERR_RANGE;

    *out = sum;
    return STATUS_OK;
}

// ниже 4 подинтегральные функции. будем считать в них эпсилон = 1e-15 (пусть будет только один красивый параметр)

static double f_1(const double x) {
    if (fabs(x) < 1e-15) {
        return 1.0;
    }
    return log(1.0 + x) / x;
}

static double f_2(const double x) {
    return exp(-x * x / 2.0);
}

static double f_3(const double x) {
    if (x >= 1.0)
        return 0.0;
    return log(1.0 / (1.0 - x));
}

static double f_4(const double x) {
    if (fabs(x) < 1e-15)
        return 1.0;
    return pow(x, x);
}

status_t integrate(func_t f, double a, double b, double eps, double *out) {
    if (f == NULL || out == NULL) return STATUS_ERR_NULL_ARG;
    if (b < a) return STATUS_ERR_INVALID;

    int n = 1;
    double prev = 0.0;
    double cur = 0.0;
    const int LIMIT = 1 << 28;
    
    while (n < LIMIT) {
        double h = (b - a) / (double) n;
        double sum = 0.0;
        
        for (int i = 0; i < n; i++)
            sum += f(a + (i + 0.5) * h);

        prev = cur;
        cur = h * sum;
        
        if (n > 1 && fabs(cur - prev) < eps) break;

        n *= 2;
    }


    *out = cur;
    
    if (n >= LIMIT) 
        return STATUS_ERR_RANGE;
    return STATUS_OK;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Формат запуска программы: [программа] [эпсилон] [x]\n");
		return 0;
	}
	
	double eps = 0.0;
    status_t rc1 = parse_double(argv[1], &eps);

    switch (rc1) {
        case STATUS_OK:
            if (eps <= 0 || eps >= 1) {
                fprintf(stderr, "Эпсилон должен быть в (0, 1)\n");
                return 0;
            }
            break;
        case STATUS_ERR_FORMAT:
            fprintf(stderr, "Ошибка: '%s' не является числом\n", argv[1]);
            return 0;
        case STATUS_ERR_RANGE:
            fprintf(stderr, "Эпсилон должен быть в (0, 1)\n");
            return 0;
	}

    double x;
    status_t rc2 = parse_double(argv[2], &x);
    if (rc2 != STATUS_OK) {
        fprintf(stderr, "Переменная x неправильного формата\n");
        return 0;
    }

    double sum1, sum2, sum3, sum4;
    double int1, int2, int3, int4;
    sum1_series(x, eps, &sum1);
    sum2_series(x, eps, &sum2);
    sum3_series(x, eps, &sum3);
    sum4_series(x, eps, &sum4);
    integrate(f_1, 0.0, 1.0, eps, &int1);
    integrate(f_2, 0.0, 1.0, eps, &int2);
    status_t rc = integrate(f_3, 0.0, 1.0, eps, &int3);
    integrate(f_4, 0.0, 1.0, eps, &int4);

    if (rc == STATUS_ERR_RANGE)
        printf("найдено\n"); //скахать семинаристу!

    printf("Σ(n=0..∞) xⁿ / n! = %.10f\n", sum1);
    printf("Σ(n=0..∞) (−1)ⁿ x²ⁿ / (2n)! = %.10f\n", sum2);
    printf("Σ(n=0..∞) 3³ⁿ (n!)³ x²ⁿ / (3n)! = %.10f\n", sum3);
    printf("Σ(n=1..∞) (−1)ⁿ (2n−1)!! x²ⁿ / (2n)!! = %.10f\n", sum4);
    printf("∫₀¹ ln(1+x)/x dx = %.10f\n", int1);
    printf("∫₀¹ e^(−x²/2) dx = %.10f\n", int2);
    printf("∫₀¹ ln(1/(1−x)) dx = %.10f\n", int3);
    printf("∫₀¹ xˣ dx = %.10f\n", int4);
}