#include <utils.h>

#include <ctype.h>
#include <string.h>

typedef struct {
    char *repr;
    int base;
    size_t value;
} num_t;

static const char *strip_leading_zeros(const char *s)
{
    while (*s == '0' && *(s + 1) != '\0')
        ++s;
    return s;
}

status_t min_base(const char *s, int *out)
{
    if (s == NULL || out == NULL)
        return STATUS_ERR_NULL_ARG;
    if (*s == '\0')
        return STATUS_ERR_INVALID;

    int max_digit = 0;
    int has_digit = 0;

    for (; *s; ++s) {
        int c = tolower((unsigned char)*s);
        int d;

        if (c >= '0' && c <= '9')
            d = c - '0';
        else if (c >= 'a' && c <= 'z')
            d = c - 'a' + 10;
        else
            return STATUS_ERR_FORMAT;

        if (d > max_digit)
            max_digit = d;
        has_digit = 1;
    }

    if (!has_digit)
        return STATUS_ERR_INVALID;

    *out = (max_digit < 2) ? 2 : (max_digit + 1);
    return STATUS_OK;
}

status_t solve_values(const char *src, const size_t src_size,
                      char **dst, size_t *dst_size) {
    char *copy = malloc(src_size + 1);
    if (!copy) return STATUS_ERR_ALLOC;
    memcpy(copy, src, src_size);
    copy[src_size] = '\0';

    size_t cap = src_size * 32 + 64;
    char *out = malloc(cap);
    if (!out) { free(copy); return STATUS_ERR_ALLOC; }

    size_t w = 0;

    char *token = strtok(copy, " \t\n");
    while (token != NULL) {

        num_t num;
        num.repr = token;
        status_t rc = min_base(num.repr, &num.base);

        if (rc != STATUS_OK) {
            free(copy);
            free(out);
            return rc;
        }

        rc = parse_in_base(num.repr, num.base, &num.value);

        if (rc != STATUS_OK) {
            free(copy);
            free(out);
            return rc;
        }

        const char *clean = strip_leading_zeros(num.repr);
        w += snprintf(out + w, cap - w, "%s ", clean);
        w += snprintf(out + w, cap - w, "%d ", num.base);
        w += snprintf(out + w, cap - w, "%zu\n", num.value);

        token = strtok(NULL, " \t\n");
    }

    free(copy);

    *dst = out;
    *dst_size = w;
    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Формат запуска программы: [программа] [file1(in)] [file2(out)]\n");
        return 1;
    }

    char *in_path = argv[1];
    char *out_path = argv[2];

    char *src = NULL;
    size_t src_size;

    status_t rc = read_file(in_path, &src, &src_size);
    switch (rc) {
        case STATUS_OK:
            break;
        case STATUS_ERR_OPEN:
            fprintf(stderr, "Не удалось открыть '%s'\n", in_path);
            free(src);
            return 1;
        case STATUS_ERR_ALLOC:
            fprintf(stderr, "Ошибка выделения памяти\n");
            free(src);
            return 1;
        case STATUS_ERR_READ:
            fprintf(stderr, "Ошибка чтения '%s'\n", in_path);
            free(src);
            return 1;
        case STATUS_ERR_CLOSE:
            fprintf(stderr, "Ошибка закрытия '%s'\n", in_path);
            free(src);
            return 1;
        default:
            fprintf(stderr, "Внутренняя ошибка чтения\n");
            free(src);
            return 1;
    }

    char *dst = NULL;
    size_t dst_size;
    rc = solve_values(src, src_size,
                      &dst, &dst_size);
    free(src);

    if (rc != STATUS_OK) {
        fprintf(stderr, "Что-то пошло не так, но мне лень расписывать почему.\n");
        free(dst);
        return 1;
    }

    rc = write_file(out_path, dst, dst_size);
    switch (rc) {
        case STATUS_OK:
            break;
        case STATUS_ERR_OPEN:
            fprintf(stderr, "Не удалось открыть '%s' для записи\n", out_path);
            free(dst);
            return 1;
        case STATUS_ERR_WRITE:
            fprintf(stderr, "Ошибка записи в '%s'\n", out_path);
            free(dst);
            return 1;
        case STATUS_ERR_CLOSE:
            fprintf(stderr, "Ошибка закрытия '%s'\n", out_path);
            free(dst);
            return 1;
        default:
            fprintf(stderr, "Внутренняя ошибка записи\n");
            free(dst);
            return 1;
    }

    free(dst);
}