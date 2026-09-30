#include <stdio.h>
#include <math.h>

#include <utils.h>

double factorial(const int num) { // Извиняюсь что это не по регламенту
	double result = 1;
    for (int i = 1; i <= num; i++) {
        result *= i;
    }
	return result;
}

status_t calc_e_limit(const double eps, double *out) {
    if (out == NULL) return STATUS_ERR_NULL_ARG;

	int n = 1;
	double prev, res = 0;
	while (n < 10000000) {
		prev = res;
		res = pow(1 + (1 / (double)n), (double)n);
		//printf("%f\n", res);
		if (fabs(res - prev) * (double)n < eps) {
			break;
		}
		n++;
	}
	*out = prev;
	return STATUS_OK;
}

status_t calc_e_series(const double eps, double *out) {
    if (out == NULL) return STATUS_ERR_NULL_ARG;

	int n = 0;
	double prev, res = 0;
	while (n < 10000000) {
		prev = res;
		res += 1 / (double)factorial(n);
		//printf("%f\n", res);
		if (fabs(res - prev) < eps) {
			break;
		}
		n++;
	}
	*out = prev;
	return STATUS_OK;
}

status_t solve_e(double eps, double *out) {
    if (out == NULL) return STATUS_ERR_NULL_ARG;
	
    double a = 1.0, b = 3.0;
    double fa = log(a) - 1.0;
    double fb = log(b) - 1.0;

    if (fa * fb >= 0.0)
        return STATUS_ERR_INVALID; 

    int n = 0;
    while ((b - a) > eps) {
        double mid = 0.5 * (a + b);
        double fm  = log(mid) - 1.0;

        if (fm < 0.0) { a = mid; fa = fm; }
        else          { b = mid; fb = fm; }

        if (++n > 1000000)
            return STATUS_ERR_RANGE;
    }

    *out = 0.5 * (a + b);
    return STATUS_OK;
}

status_t calc_pi_limit(const double eps, double *out) {
    if (out == NULL) return STATUS_ERR_NULL_ARG;
	
	int n = 1;
	double prev, res = 4;
	while (n < 10000000) {
		prev = res;
		double num = 16.0 * pow((double)(n + 1), 3.0) * (double)n;
        double den = pow(2.0 * n + 1.0, 2.0) * pow(2.0 * n + 2.0, 2.0);

		res *= num / den;
		if (fabs(res - prev) * (double)n < eps) {
			break;
		}
		n++;
	}
	*out = res;
	return STATUS_OK;
}

status_t calc_pi_series(const double eps, double *out) {
    if (out == NULL) return STATUS_ERR_NULL_ARG;

	int n = 1;
	double prev, res = 0;
	while (n < 10000000) {
		prev = res;
		res += pow(-1, n-1) / (double)(2 * n - 1);
		//printf("%.10f\n", res);
		if (fabs(res - prev) < eps) {
			break;
		}
		n++;
	}
	*out = 4 * prev;
	return STATUS_OK;
}

status_t solve_pi(double eps, double *out) {
    if (out == NULL) return STATUS_ERR_NULL_ARG;

    double a = 3.0, b = 4.0;
    int n = 0;

    while ((b - a) > eps) {
        double m1 = a + (b - a) / 3.0;
        double m2 = b - (b - a) / 3.0;

        double f1 = cos(m1) + 1.0;
        double f2 = cos(m2) + 1.0;

        if (f1 < f2)  b = m2;
        else          a = m1;

        if (++n >= 1000000)
            return STATUS_ERR_RANGE;
    }

    *out = 0.5 * (a + b);
    return STATUS_OK;
}

status_t calc_ln2_limit(const double eps, double *out) {
    if (out == NULL) return STATUS_ERR_NULL_ARG;
	
	int n = 1;
	double prev, res = 0;
	while (n < 10000000) {
		prev = res;
		res = n * (pow(2, 1 / (double)n) - 1);

		if (fabs(res - prev) * (double)n < eps) {
			break;
		}
		n++;
	}
	*out = res;
	return STATUS_OK;
}

status_t calc_ln2_series(const double eps, double *out) {
    if (out == NULL) return STATUS_ERR_NULL_ARG;

	int n = 1;
	double prev, res = 0;
	while (n < 10000000) {
		prev = res;
		res += pow(-1, n-1) / (double)n;
		//printf("%.10f\n", res);
		if (fabs(res - prev) < eps) {
			break;
		}
		n++;
	}
	*out = prev;
	return STATUS_OK;
}

status_t solve_ln2(const double eps, double *out)
{
    if (out == NULL) return STATUS_ERR_NULL_ARG;

    double a = 0.0, b = 1.0;
    double fa = exp(a) - 2.0;
    double fb = exp(b) - 2.0;
    int n = 0;

    if (fa * fb >= 0.0)
        return STATUS_ERR_INVALID;
	
    while ((b - a) > eps) {
        double mid = 0.5 * (a + b);
        double fm  = exp(mid) - 2.0;

        if (fa * fm < 0.0) {
            b  = mid;
            fb = fm;
        } else {
            a  = mid;
            fa = fm;
        }

        if (++n >= 1000000)
            return STATUS_ERR_RANGE;
    }

    *out = 0.5 * (a + b);
    return STATUS_OK;
}

status_t calc_sq2_limit(const double eps, double *out) {
    if (out == NULL) return STATUS_ERR_NULL_ARG;
	
	int n = 1;
	double prev, res = -0.5;
	while (n < 10000000) {
		prev = res;
		res = prev - (pow(prev, 2) / 2) + 1;

		if (fabs(res - prev) * (double)n < eps) {
			break;
		}
		n++;
	}
	*out = res;
	return STATUS_OK;
}

status_t calc_sq2_series(const double eps, double *out) {
    if (out == NULL) return STATUS_ERR_NULL_ARG;

	int k = 2;
	double prev, res = 1;
	while (k < 10000000) {
		prev = res;
		res *= pow(2, pow(2, -1 * k));
		//printf("%.10f\n", res);
		if (fabs(res - prev) < eps) {
			break;
		}
		k++;
	}
	*out = prev;
	return STATUS_OK;
}

status_t solve_sq2(const double eps, double *out)
{
    if (out == NULL) return STATUS_ERR_NULL_ARG;

    double a = 1.0, b = 2.0;
    double fa = a * a - 2.0;
    double fb = b * b - 2.0;
    int n = 0;

    if (fa * fb >= 0.0)
        return STATUS_ERR_INVALID;

    while ((b - a) > eps) {
        double mid = 0.5 * (a + b);
        double fm  = mid * mid - 2.0;

        if (fa * fm < 0.0) {
            b  = mid;
            fb = fm;
        } else {
            a  = mid;
            fa = fm;
        }

        if (++n >= 1000000)
            return STATUS_ERR_RANGE;
    }

    *out = 0.5 * (a + b);
    return STATUS_OK;
}

status_t calc_y_limit(const double eps, double *out)
{
    if (out == NULL) return STATUS_ERR_NULL_ARG;

    double prev_sum = 0.0;
    double sum      = 0.0;
    int    m        = 1;
    while (m < 40) {
        prev_sum = sum;
        sum = 0.0;

        double Cmk     = 1.0;
        double ln_kfac = 0.0;
        double sign    = -1.0;

        for (int k = 1; k <= m; ++k) {
            Cmk *= (double)(m - k + 1) / (double)k;

            ln_kfac += log((double)k);

            sum += Cmk * sign * ln_kfac / (double)k;

            sign = -sign;
        }

        if (m > 1 && fabs(sum - prev_sum) < eps)
            break;

    	++m;
    }

    *out = sum;
    return STATUS_OK;
}


status_t calc_y_series(const double eps, double *out)
{
    if (out == NULL) return STATUS_ERR_NULL_ARG;

    double sum  = 0.0;
    double term = 0.0;
    int    k    = 2;
    const int LIMIT = 100000000;

    while (k) {
        double sq = floor(sqrt((double)k));

        term = 1.0 / (sq * sq) - 1.0 / (double)k;
        if (term != 0.0) { sum += term; if (fabs(term) < eps) break; }

		if (++k >= 100000000)
			return STATUS_ERR_RANGE;
    }


    const double PI2_6 = 1.64493406684822643647;
    *out = -PI2_6 + sum;

    return STATUS_OK;
}

static int is_prime(int n)
{
    if (n < 2) return 0;
    for (int d = 2; (long)d * d <= n; ++d)
        if (n % d == 0) return 0;
    return 1;
}

static double product_over_primes(int t)
{
    double product = 1.0;
    for (int p = 2; p <= t; ++p)
        if (is_prime(p))
            product *= (double)(p - 1) / (double)p;
    return log((double)t) * product;
}

status_t solve_y(const double eps, double *out)
{
    if (out == NULL) return STATUS_ERR_NULL_ARG;
    if (eps <= 0.0)  return STATUS_ERR_INVALID;

    const int T = 1000000;

    double L = product_over_primes(T);
    if (L < 0.0)
        return STATUS_ERR_ALLOC;

    double a = 0.0, b = 1.0;
    double fa = exp(-a) - L;
    double fb = exp(-b) - L;

    if (fa * fb >= 0.0)
        return STATUS_ERR_INVALID;

    int n = 0;

    while ((b - a) > eps) {
        double mid = 0.5 * (a + b);
        double fm  = exp(-mid) - L;

        if (fa * fm < 0.0) {
            b  = mid;
            fb = fm;
        } else {
            a  = mid;
            fa = fm;
        }

        if (++n >= 1000000)
            return STATUS_ERR_RANGE;
    }

    *out = 0.5 * (a + b);
    return STATUS_OK;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Формат запуска программы: [программа] [эпсилон] \n");
		return 0;
	}
	
	double eps = 0.0;
    status_t rc = parse_eps(argv[1], &eps);

    switch (rc) {
        case STATUS_ERR_FORMAT:
            fprintf(stderr, "Ошибка: '%s' не является числом\n", argv[1]);
            return 0;
        case STATUS_ERR_RANGE:
            fprintf(stderr, "Эпсилон должен быть в (0, 1)\n");
            return 0;
	}
	
	double e_limit; calc_e_limit(eps, &e_limit);
	double e_series; calc_e_series(eps, &e_series);
	double e_solved; solve_e(eps, &e_solved);
	double pi_limit; calc_pi_limit(eps, &pi_limit);
	double pi_series; calc_pi_series(eps, &pi_series);
	double pi_solved; solve_pi(eps, &pi_solved);
	double ln2_limit; calc_ln2_limit(eps, &ln2_limit);
	double ln2_series; calc_ln2_series(eps, &ln2_series);
	double ln2_solved; solve_ln2(eps, &ln2_solved);
	double sq2_limit; calc_sq2_limit(eps, &sq2_limit);
	double sq2_series; calc_sq2_series(eps, &sq2_series);
	double sq2_solved; solve_sq2(eps, &sq2_solved);
	double y_limit; calc_y_limit(eps, &y_limit);
	double y_series; calc_y_series(eps, &y_series);
	double y_solved; solve_y(eps, &y_solved);

	printf("Эпсилон: 			%.10f\n", eps);
	printf("e (экспонента)\n");
	printf("Предел: 			%.10f\n", e_limit);
	printf("Ряд/Произведение: 		%.10f\n", e_series);
	printf("Уравнение: 			%.10f\n", e_solved);
	printf("π\n");
	printf("Предел: 			%.10f\n", pi_limit);
	printf("Ряд/Произведение: 		%.10f\n", pi_series);
	printf("Уравнение: 			%.10f\n", pi_solved);
	printf("ln 2 (Натуральный логарифм двух)\n");
	printf("Предел: 			%.10f\n", ln2_limit);
	printf("Ряд/Произведение: 		%.10f\n", ln2_series);
	printf("Уравнение: 			%.10f\n", ln2_solved);
	printf("√2\n");
	printf("Предел: 			%.10f\n", sq2_limit);
	printf("Ряд/Произведение: 		%.10f\n", sq2_series);
	printf("Уравнение: 			%.10f\n", sq2_solved);
	printf("γ (Постоянная Эйлера-Маскерони)\n");
	printf("Предел: 			%.10f\n", y_limit);
	printf("Ряд/Произведение: 		%.10f\n", y_series);
	printf("Уравнение: 			%.10f\n", y_solved);
}