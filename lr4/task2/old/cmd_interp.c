#include "cmd_interp.h"
#include "int_array.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

int strcasecmp(const char *a, const char *b) {
    while (*a && *b) {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        a++;
        b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

static bool is_empty_c(char c) {
    return (c == ' ') || (c == '\t');
}

static bool is_cmd_name_char(char ch) {
    return isalpha((unsigned char)ch);
}

static status_t read_token(FILE *f, char **out) {
    if (!f || !out) return STATUS_ERR_NULL_ARG;

    *out = NULL;

    int c;
    while ((c = fgetc(f)) != EOF &&
           (is_empty_c(c) || c == '\n' || c == '\r' || c == ','));

    if (c == EOF)
        return ferror(f) ? STATUS_ERR_PARSE : STATUS_END_PARSE;

    size_t len = 0;
    size_t cap = 16;

    char *str = malloc(cap);
    if (!str) return STATUS_ERR_ALLOC;

    do {
        if (len + 1 >= cap) {
            if (cap > SIZE_MAX / 2) {
                free(str);
                return STATUS_ERR_ALLOC;
            }

            cap *= 2;

            char *tmp = realloc(str, cap);
            if (!tmp) {
                free(str);
                return STATUS_ERR_ALLOC;
            }

            str = tmp;
        }

        str[len++] = (char)c;
        c = fgetc(f);

    } while (c != EOF && !is_empty_c(c) &&
             c != '\n' && c != '\r' && c != ',');

    if (c == EOF && ferror(f)) {
        free(str);
        return STATUS_ERR_PARSE;
    }

    str[len] = '\0';
    *out = str;

    return STATUS_OK;
}

static status_t read_arg(FILE *f, char **out) {
    status_t rc = read_token(f, out);

    if (rc == STATUS_END_PARSE)
        return STATUS_ERR_PARSE;

    return rc;
}

// ну чтоб точно не придрались для ссылок
static status_t interpret_cmd(FILE *f, IntArray **storage) {
    status_t rc;

    char *cmd_name = NULL;
    char *arg = NULL;

    rc = read_arg(f, &cmd_name);
    if (rc != STATUS_OK) return rc;

    IntArray *arr = NULL;
    int idx;

    if (strcasecmp(cmd_name, "load") == 0) {
        rc = read_arg(f, &arg);
        if (rc != STATUS_OK) goto cleanup;

        if (arg[1] != '\0' || (idx = char_to_idx(arg[0])) < 0) {
            rc = STATUS_ERR_INVALID;
            goto cleanup;
        }

        arr = storage[idx];
        free(arg);
        arg = NULL;

        rc = read_arg(f, &arg);
        if (rc != STATUS_OK) goto cleanup;

        rc = IntArray_load(arr, arg);

    } else if (strcasecmp(cmd_name, "save") == 0) {
        rc = read_arg(f, &arg);
        if (rc != STATUS_OK) goto cleanup;

        if (arg[1] != '\0' || (idx = char_to_idx(arg[0])) < 0) {
            rc = STATUS_ERR_INVALID;
            goto cleanup;
        }

        arr = storage[idx];
        free(arg);
        arg = NULL;

        rc = read_arg(f, &arg);
        if (rc != STATUS_OK) goto cleanup;

        rc = IntArray_save(arr, arg);

    } else if (strcasecmp(cmd_name, "rand") == 0) {
        size_t count;
        int lb, rb;

        rc = read_arg(f, &arg);
        if (rc != STATUS_OK) goto cleanup;

        if (arg[1] != '\0' || (idx = char_to_idx(arg[0])) < 0) {
            rc = STATUS_ERR_INVALID;
            goto cleanup;
        }

        arr = storage[idx];
        free(arg);
        arg = NULL;

        // count
        rc = read_arg(f, &arg);
        if (rc != STATUS_OK) goto cleanup;

        rc = parse_size(arg, &count);
        free(arg);
        arg = NULL;

        if (rc != STATUS_OK) {
            free(arg);
            free(cmd_name);
            return rc;
        }

        // lb
        rc = read_arg(f, &arg);
        if (rc != STATUS_OK) {
            free(arg);
            free(cmd_name);
            return rc;
        }

        rc = parse_int(arg, &lb);
        free(arg);
        arg = NULL;

        if (rc != STATUS_OK) {
            free(arg);
            free(cmd_name);
            return rc;
        }

        // rb
        rc = read_arg(f, &arg);
        if (rc != STATUS_OK) {
            free(arg);
            free(cmd_name);
            return rc;
        }

        rc = parse_int(arg, &rb);
        free(arg);
        arg = NULL;

        if (rc != STATUS_OK) {
            free(arg);
            free(cmd_name);
            return rc;
        }

        rc = IntArray_rand(arr, count, lb, rb);

    } else if (strcasecmp(cmd_name, "concat") == 0) {
        rc = read_arg(f, &arg);
        if (rc != STATUS_OK) {
            free(arg);
            free(cmd_name);
            return rc;
        }

        if (arg[1] != '\0' || (idx = char_to_idx(arg[0])) < 0) {
            free(arg);
            free(cmd_name);
            return STATUS_ERR_INVALID;
        }

        arr = storage[idx];
        free(arg);
        arg = NULL;

        rc = read_arg(f, &arg);
        if (rc != STATUS_OK) {
            free(arg);
            free(cmd_name);
            return rc;
        }

        if (arg[1] != '\0' || (idx = char_to_idx(arg[0])) < 0) {
            free(arg);
            free(cmd_name);
            return STATUS_ERR_INVALID;
        }

        rc = IntArray_concat(arr, storage[idx]);

    } else if (strcasecmp(cmd_name, "free") == 0) {
        rc = read_arg(f, &arg);
        if (rc != STATUS_OK) {
            free(arg);
            free(cmd_name);
            return rc;
        }

        if (arg[1] != '\0' || (idx = char_to_idx(arg[0])) < 0) {
            free(arg);
            free(cmd_name);
            return STATUS_ERR_INVALID;
        }

        rc = IntArray_free(storage[idx]);

    } else {
        rc = STATUS_ERR_PARSE;
    }

    free(arg);
    free(cmd_name);

    return rc;
}

status_t interpret(FILE *f) {
    status_t rc;

    IntArray *storage[26];


    for (;;) {
        status_t rc = interpret_cmd(f, storage);

    }


    return STATUS_OK;
}