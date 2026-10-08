#include <utils.h>

#include <stdbool.h>
#include <stdio.h>

// typedef struct michael_t michael_t;
// struct michael_t { michael_t mt; };

struct {
    char b_num;
    size_t num;
    bool kprkr;
}   kaprekar_t;

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

status_t find_kaprekar(kaprekar_t *arr, const int count, const int base)
{
    if (base < 2 || base > 36)  return STATUS_ERR_INVALID;
    if (count < 0)              return STATUS_ERR_INVALID;

    va_list args;
    va_start(args, count);

    status_t result = STATUS_OK;

    for (int i = 0; i < count; ++i) {
        const char *s = va_arg(args, const char *);
        if (s == NULL) {
            result = STATUS_ERR_NULL_ARG;
            break;
        }

        size_t n;
        status_t rc = parse_in_base(s, base, &n);
        if (rc != STATUS_OK) {
            result = rc;
            break;
        }

        if (is_kaprekar(n, base)) {
            printf("'%s' (%llu) — число Капрекара в base=%d\n",
                   s, n, base);
        } else {
            printf("'%s' (%llu) — не число Капрекара\n", s, n);
        }
    }

    va_end(args);
    return result;
}


int main(int argc, char *argv[]) {
    status_t rc;

    if (argc < 3) {
        fprintf(stderr, "Формат запуска программы: [программа] [base] [число] ...\n");
		return 1;
    }

    const size_t count = argc - 2;
    size_t base;
    rc = parse_int(argv[2], &n);
    if (rc != STATUS_OK) {
        fprintf(stderr, "base должна быть целого типа\n");
        return 1;
    }

    char nums_based[count];
    for (int i = 0; i < count; i++) {
        nums_based[i] = argv[]
    }
    
    size_t nums_;
}