#include "utils.h"

#include <ctype.h>
#include <stddef.h>

status_t get_length(const char *s, int *out) {
    if (s == NULL || out == NULL)
        return STATUS_ERR_NULL_ARG;

    int n = 0;
    while (*s != '\0') {
        ++n;
        ++s;
    }
    *out = n;

    return STATUS_OK;
}

status_t get_reversed(const char *s, char *out) {
    if (s == NULL || out == NULL)
        return STATUS_ERR_NULL_ARG;

    int l;
    status_t rc = get_length(s, &l);
    if (rc != STATUS_OK) return rc;
    for (int i = 0; i < l; i++) {
        out[l - i - 1] = s[i];
    }
    out[l] = '\0';

    return STATUS_OK;
}

status_t get_upper_odd(const char *s, char *out) {
    if (s == NULL || out == NULL)
        return STATUS_ERR_NULL_ARG;

    int i = 0;
    while (*s != '\0') {
        if (i % 2 == 1)
            out[i] = (char)toupper((unsigned char)*s);
        else
            out[i] = *s;
        ++i;
        ++s;
    }
    out[i] = '\0';

    return STATUS_OK;
}

status_t get_normalized(const char *s, char *out) {
    if (s == NULL || out == NULL)
        return STATUS_ERR_NULL_ARG;

    char num[BUFSIZ];   int wnum = 0;
    char let[BUFSIZ];   int wlet = 0;
    char sym[BUFSIZ];   int wsym = 0;
    while (*s != '\0') {
        if (isdigit((unsigned char)*s))
            num[wnum++] = *s;
        else if (isalpha((unsigned char)*s))
            let[wlet++] = *s;
        else
            sym[wsym++] = *s;
        ++s;
    }

    int w = 0;
    for (int i = 0; i < wnum; i++)
        out[w++] = num[i];
    for (int i = 0; i < wlet; i++)
        out[w++] = let[i];
    for (int i = 0; i < wsym; i++)
        out[w++] = sym[i];
    out[w] = '\0';

    return STATUS_OK;
}

status_t get_concatenated_randomly(char **arr, const size_t n, char *out) {
    if (arr == NULL || out == NULL)
        return STATUS_ERR_NULL_ARG;
    if (n < 0)
        return STATUS_ERR_RANGE;

    size_t idx[n];
    for (size_t i = 0; i < n; ++i)
        idx[i] = i;
    for (size_t i = n; i > 1; --i) {
        size_t j = (size_t)rand() % i;
        size_t tmp = idx[i - 1];
        idx[i - 1] = idx[j];
        idx[j] = tmp;
    }

    size_t widx = 0;
    size_t w = 0;
    while (widx < n) {
        while (*arr[idx[widx]] != '\0') {
            out[w++] = *arr[idx[widx]];
            arr[idx[widx]]++;
        }
        widx++;
    }
    out[w] = '\0';

    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    status_t rc;

    if (argc < 3) {
		fprintf(stderr, "Формат запуска программы: [программа] [флаг] [строка]\n");
        return 1;
	}

    char flag;
    if (sscanf(argv[1], "-%c", &flag) != 1 || argv[1][2] != '\0') {
		fprintf(stderr, "'%s' не флаг формата -X.\n", argv[1]);
        return 1;
	}

    if (flag == 'c' && argc < 4) {
		fprintf(stderr, "Формат запуска программы с флагом -r: [программа] [флаг] [строка] [unsigned int] [...].\n");
        return 1;
	} else if (flag != 'c' && argc != 3) {
		fprintf(stderr, "Формат запуска программы: [программа] [флаг] [строка].\n");
        return 1;
    }

    char *string = argv[2];

    switch (flag) {
        case 'l': {
            int length = 0;
            rc = get_length(string, &length);
            switch (rc) {
                case STATUS_OK: { break; }
                case STATUS_ERR_NULL_ARG: {
                    fprintf(stderr, "Произошла внутренняя ошибка.\n");
                    return 1;
                }
                default: {
                    fprintf(stderr, "Произошла неизвестная ошибка.\n");
                    return 1;
                }
            }
            
            printf("length: %d\n", length);
            break;
        }
        case 'r': {
            char reversed[BUFSIZ];
            rc = get_reversed(string, reversed);
            switch (rc) {
                case STATUS_OK: { break; }
                case STATUS_ERR_NULL_ARG: {
                    fprintf(stderr, "Произошла внутренняя ошибка.\n");
                    return 1;
                }
                default: {
                    fprintf(stderr, "Произошла неизвестная ошибка.\n");
                    return 1;
                }
            }

            printf("reversed: %s\n", reversed);
            break;
        }
        case 'u': {
            char upper_odds[BUFSIZ];
            rc = get_upper_odd(string, upper_odds);
            switch (rc) {
                case STATUS_OK: { break; }
                case STATUS_ERR_NULL_ARG: {
                    fprintf(stderr, "Произошла внутренняя ошибка.\n");
                    return 1;
                }
                default: {
                    fprintf(stderr, "Произошла неизвестная ошибка.\n");
                    return 1;
                }
            }

            printf("upper odds: %s\n", upper_odds);
            break;
        }
        case 'n': {
            char normalized[BUFSIZ];
            rc = get_normalized(string, normalized);
            switch (rc) {
                case STATUS_OK: { break; }
                case STATUS_ERR_NULL_ARG: {
                    fprintf(stderr, "Произошла внутренняя ошибка.\n");
                    return 1;
                }
                default: {
                    fprintf(stderr, "Произошла неизвестная ошибка.\n");
                    return 1;
                }
            }

            printf("normalized: %s\n", normalized);
            break;
        }
        case 'c': {
            unsigned int seed;
            rc = parse_uint(argv[3], &seed);
            switch (rc) {
                case STATUS_OK: { break; }
                case STATUS_ERR_RANGE: {
                    fprintf(stderr, "seed не вмещается в unsigned int.\n");
                    return 1;
                }
                case STATUS_ERR_FORMAT: {
                    fprintf(stderr, "unsigned int неправильного формата.\n");
                    return 1;
                }
                case STATUS_ERR_NULL_ARG: {
                    fprintf(stderr, "Произошла внутренняя ошибка.\n");
                    return 1;
                }
                default: {
                    fprintf(stderr, "Произошла неизвестная ошибка.\n");
                    return 1;
                }
            }
            srand(seed);

            size_t string_count = argc - 3;
            char **strings = malloc(string_count * sizeof(*strings));
            if (!strings) {
                fprintf(stderr, "Не удалось выделить память.\n");
                return 1;
            }
            strings[0] = string;
            for (size_t i = 1; i < string_count; i++) {
                strings[i] = argv[i + 3];
            }

            char randconcstring[BUFSIZ];
            rc = get_concatenated_randomly(strings, string_count, randconcstring);
            switch (rc) {
                case STATUS_OK: { break; }
                case STATUS_ERR_RANGE: {
                    fprintf(stderr, "Размер массива не может быть отрицательным.\n");
                    return 1;
                }
                case STATUS_ERR_NULL_ARG: {
                    fprintf(stderr, "Произошла внутренняя ошибка.\n");
                    return 1;
                }
                default: {
                    fprintf(stderr, "Произошла неизвестная ошибка.\n");
                    return 1;
                }
            }

            printf("randomly concatenated: %s\n", randconcstring);
            break;
        }
        default: {
		    fprintf(stderr, "Допустимые флаги: l; r; u; n; c.\n");
            return 1;
        }
    }
}