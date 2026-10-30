#include "../utils.h"

#include <stdarg.h>
#include <string.h> 
#include <stdbool.h>

static int digit_val(int c, int base) {
    int d;
    if (c >= '0' && c <= '9')             d = c - '0';
    else if (c >= 'a' && c <= 'z')        d = c - 'a' + 10;
    else if (c >= 'A' && c <= 'Z')        d = c - 'A' + 10;
    else                                  return -1;

    if (d >= base) return -1;
    return d;
}

static char digit_char(int v, bool up) {
    if (v < 0 || v > 35) return '?';
    if (v < 10)          return (char)('0' + v);
    return (char)((up ? 'A' : 'a') + (v - 10));
}

static status_t stradd(const char *a, const char *b, int base,
                     char *out, size_t size) {
    if (a == NULL || b == NULL || out == NULL)  return STATUS_ERR_NULL_ARG;
    if (size == 0)                              return STATUS_ERR_OVERFLOW;
    if (base < 2 || base > 36)                  return STATUS_ERR_INVALID;

    size_t la = strlen(a), lb = strlen(b);
    size_t maxlen = (la > lb ? la : lb) + 1;
    if (maxlen + 1 > size)                      return STATUS_ERR_OVERFLOW;

    char *tmp = malloc(maxlen * sizeof *tmp);
    if (!tmp)                                   return STATUS_ERR_ALLOC;

    size_t pos = 0;
    int carry = 0;

    for (size_t i = 0; i < maxlen; i++) {
        int da = (i < la) ? digit_val(a[la - 1 - i], base) : 0;
        int db = (i < lb) ? digit_val(b[lb - 1 - i], base) : 0;
        if (da < 0 || db < 0)                   return STATUS_ERR_FORMAT;

        int sum = da + db + carry;
        tmp[pos++] = digit_char(sum % base, true);
        carry = sum / base;
    }
    if (carry > 0) tmp[pos++] = digit_char(carry, true);

    while (pos > 1 && tmp[pos - 1] == '0') pos--;

    for (size_t i = 0; i < pos; i++) out[i] = tmp[pos - 1 - i];
    free(tmp);
    out[pos] = '\0';
    return STATUS_OK;
}

static status_t strsum(size_t base, char **out, size_t count, ...) {
    if (out == NULL)    return STATUS_ERR_NULL_ARG;
    if (count < 1)      return STATUS_ERR_INVALID;
    status_t rc;

    char *str[count];

    va_list ap;
    va_start(ap, count);

    for (int i = 0; i < count; i++)
        str[i] = va_arg(ap, char *);

    va_end(ap);

    size_t maxlen = 0;
    for (int i = 0; i < count; i++) {
        size_t len = strlen(str[i]);
        if (len > maxlen) maxlen = len;
    }
    maxlen += count + 1; 
    // оценку можно улучшить, но заморачиваться не стал

    char *sum = malloc(maxlen * sizeof *sum);
    sum[0] = '0'; sum[1] = '\0';

    for (int i = 0; i < count; i++) {
        rc = stradd(sum, str[i], base, sum, maxlen);
        if (rc) { free(sum); return rc; }
    }

    *out = sum;
    
    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    char *addend[] = { "A7K3M9X2Q5B", "8R4T6W1Y7C3", "F0QZOB0XHE8" };

    char *str;
    status_t rc = strsum(36, &str, 3, addend[0], addend[1], addend[2]);
    switch (rc) {
        case STATUS_OK: 
            printf("%s + %s + %s = %s\n", addend[0], addend[1], addend[2], str);
            break;
        case STATUS_ERR_ALLOC:
            fprintf(stderr, "Не удалось выделить память.\n");
            return 1;
        case STATUS_ERR_OVERFLOW:
            fprintf(stderr, "В ходе программы возникло переполнение.\n");
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

    return 0;
}