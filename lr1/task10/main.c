#include <utils.h>

#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

static const char DIGITS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static char digit_to_char(unsigned int d)
{
    if (d < 10)
        return (char)('0' + d);
    return (char)('A' + (d - 10));
}

char *num_to_base_str(int value, int base, size_t *out_size) {
    if (base < 2 || base > 36)
        return NULL;

    unsigned int u;
    int neg = (value < 0);

    if (neg)
        u = (unsigned int)(-(value + 1)) + 1u;
    else
        u = (unsigned int)value;

    char tmp[64];
    int  t = 0;

    if (u == 0) {
        tmp[t++] = '0';
    } else {
        while (u > 0) {
            unsigned int d = u % (unsigned)base;
            tmp[t++] = digit_to_char(d);
            u /= (unsigned)base;
        }
    }

    size_t len = (size_t)t + (neg ? 1 : 0);

    char *buf = malloc(len + 1);
    if (!buf)
        return NULL;

    size_t w = 0;
    if (neg)
        buf[w++] = '-';

    while (t > 0)
        buf[w++] = tmp[--t];

    buf[w] = '\0';

    if (out_size)
        *out_size = w;

    return buf;
}

size_t strip_newline(char *s)
{
    char *p = strchr(s, '\n');
    if (p) *p = '\0';
    return p ? (size_t)(p - s) : strlen(s);
}

int main(int argc, char *argv[]) {
	if (argc != 1) {
		fprintf(stderr, "Формат запуска программы: [программа]\n");
		return 1;
	}

    size_t curcap = BUFSIZ;
    int *nums = malloc(curcap * sizeof(int));
    if (nums == NULL) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    } 
    char line[BUFSIZ];
    int base;
    int n = -2;
    while ((fgets(line, sizeof line, stdin) != NULL) && (strcmp(line, "Stop\n") != 0)) {
        line[strcspn(line, "\n")] = '\0';

        if (++n == -1) {
            status_t rc = parse_int(line, &base);
            switch (rc) {
                case STATUS_OK:
                    if (base < 2 || base > 36) {
                        fprintf(stderr, "Число не входит в [2, 36]\n");
                        free(nums);
                        return 1;
                    }
                    break;
                case STATUS_ERR_FORMAT:
                    fprintf(stderr, "Это не целое число\n");
                    free(nums);
                    return 1;
                case STATUS_ERR_RANGE:
                    fprintf(stderr, "Число вне диапазона int\n");
                    free(nums);
                    return 1;
                case STATUS_ERR_NULL_ARG:
                    fprintf(stderr, "NULL\n");
                    free(nums);
                    return 1;
            }
            continue;
        }

        size_t num;
        int sign = 1;
        if (line[0] == '-') {
            sign = -1;
            line[0] = '0';
        }
        status_t rc = parse_in_base(line, base, &num);
        switch (rc) {
            case STATUS_OK:
                break;
            case STATUS_ERR_FORMAT:
                fprintf(stderr, "Программа принимает числа в системе счисления с основанием %d\n", base);
                free(nums);
                return 1;
            case STATUS_ERR_NULL_ARG:
                fprintf(stderr, "NULL\n");
                free(nums);
                return 1;
        }
        int signum = sign * (int)num;

        if (n >= curcap) {
            curcap += BUFSIZ;
            int *tmp = realloc(nums, curcap * sizeof(int));
            if (tmp == NULL) {
                fprintf(stderr, "Ошибка выделения памяти\n");
                free(nums);
                return 1;
            }
            nums = tmp;
        }

        nums[n] = signum;
    }

    int nmax = 0;
    int nsum = 0;
    for (int i = 0; i <= n; i++) {
        if (fabs(nmax) < fabs(nums[i]))
            nmax = nums[i];
        nsum += nums[i];
    }

    free(nums);

    char *m9  = num_to_base_str(nmax, 9,  NULL);
    char *m18 = num_to_base_str(nmax, 18, NULL);
    char *m27 = num_to_base_str(nmax, 27, NULL);
    char *m36 = num_to_base_str(nmax, 36, NULL);
    printf("max %d %s %s %s %s\n", nmax,
        m9 ? m9 : "?", m18 ? m18 : "?", m27 ? m27 : "?", m36 ? m36 : "?");
    free(m9); free(m18); free(m27); free(m36);

    char *s9  = num_to_base_str(nsum, 9,  NULL);
    char *s18 = num_to_base_str(nsum, 18, NULL);
    char *s27 = num_to_base_str(nsum, 27, NULL);
    char *s36 = num_to_base_str(nsum, 36, NULL);
    printf("sum %d %s %s %s %s\n", nsum,
        s9 ? s9 : "?", s18 ? s18 : "?", s27 ? s27 : "?", s36 ? s36 : "?");
    free(s9); free(s18); free(s27); free(s36);
}