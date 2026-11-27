#include "cmd_interp.h"
#include "../status.h"
#include "../std_parse.h"
#include "int_array.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

typedef struct {
    FILE *f;
    char *pending;
} TokenReader;

static int read_token(TokenReader *reader, char **out) {
    if (!reader || !out) return -1;

    *out = NULL;

    if (reader->pending) {
        *out = reader->pending;
        reader->pending = NULL;
        return 1;
    }

    int c;

    do {
        c = fgetc(reader->f);
    } while (c != EOF &&
             (isspace((unsigned char)c) ||
              c == ',' || c == ';' ||
              c == '(' || c == ')'));

    if (c == EOF) {
        return ferror(reader->f) ? -1 : 0;
    }

    char quote = 0;

    if (c == '"' || c == '\'') {
        quote = (char)c;
    }

    size_t len = 0;
    size_t capacity = 32;

    char *token = malloc(capacity);

    if (!token) return -1;

    if (!quote) {
        token[len++] = (char)c;
    }

    while (true) {
        c = fgetc(reader->f);

        if (c == EOF) {
            if (ferror(reader->f)) {
                free(token);
                return -1;
            }

            if (quote) {
                free(token);
                return -1;
            }

            break;
        }

        if (quote) {
            if (c == quote) {
                break;
            }

            if (c == '\\') {
                int next = fgetc(reader->f);

                if (next == EOF) {
                    free(token);
                    return -1;
                }

                if (next == quote || next == '\\') {
                    c = next;
                } else {
                    if (len + 2 > capacity) {
                        capacity *= 2;

                        char *tmp = realloc(token, capacity);

                        if (!tmp) {
                            free(token);
                            return -1;
                        }

                        token = tmp;
                    }

                    token[len++] = '\\';
                    c = next;
                }
            }
        } else {
            if (isspace((unsigned char)c) ||
                c == ',' || c == ';' ||
                c == '(' || c == ')') {

                break;
            }
        }

        if (len + 2 > capacity) {
            if (capacity > SIZE_MAX / 2) {
                free(token);
                return -1;
            }

            capacity *= 2;

            char *tmp = realloc(token, capacity);

            if (!tmp) {
                free(token);
                return -1;
            }

            token = tmp;
        }

        token[len++] = (char)c;
    }

    token[len] = '\0';

    *out = token;

    return 1;
}

static int peek_token(TokenReader *reader, char **out) {
    if (!reader || !out) return -1;

    if (!reader->pending) {
        int result = read_token(reader, &reader->pending);

        if (result <= 0) return result;
    }

    *out = reader->pending;

    return 1;
}

static bool str_equal(const char *a, const char *b) {
    if (!a || !b) return false;

    while (*a && *b) {
        if (tolower((unsigned char)*a) !=
            tolower((unsigned char)*b)) {
            return false;
        }

        ++a;
        ++b;
    }

    return *a == '\0' && *b == '\0';
}


static void token_reader_destroy(TokenReader *reader) {
    if (!reader) return;

    free(reader->pending);
    reader->pending = NULL;
}

static bool is_integer_token(const char *token) {
    int value;
    return parse_int(token, &value) == STATUS_OK;
}

static status_t parse_array_name(const char *token, size_t *out) {
    if (!token || !out) return STATUS_ERR_NULL_ARG;

    if (token[0] == '\0' || token[1] != '\0') {
        return STATUS_ERR_INVALID;
    }

    char c = (char)toupper((unsigned char)token[0]);

    if (c < 'A' || c > 'Z') {
        return STATUS_ERR_INVALID;
    }

    *out = (size_t)(c - 'A');

    return STATUS_OK;
}

static status_t get_token(TokenReader *reader, char **out) {
    int result = read_token(reader, out);

    if (result == 1) return STATUS_OK;

    return STATUS_ERR_INVALID;
}


static status_t get_array(
    TokenReader *reader,
    IntArray **arrays,
    IntArray **out
) {
    char *token = NULL;

    status_t rc = get_token(reader, &token);

    if (rc != STATUS_OK) return rc;

    size_t idx;

    rc = parse_array_name(token, &idx);

    free(token);

    if (rc != STATUS_OK) return rc;

    *out = arrays[idx];

    return STATUS_OK;
}


static status_t get_int(
    TokenReader *reader,
    int *out
) {
    char *token = NULL;

    status_t rc = get_token(reader, &token);

    if (rc != STATUS_OK) return rc;

    rc = parse_int(token, out);

    free(token);

    return rc;
}


static status_t get_size(
    TokenReader *reader,
    size_t *out
) {
    char *token = NULL;

    status_t rc = get_token(reader, &token);

    if (rc != STATUS_OK) return rc;

    rc = parse_size(token, out);

    free(token);

    return rc;
}

static status_t interpret_command(
    TokenReader *reader,
    IntArray **arrays,
    const char *cmd
) {
    IntArray *a = NULL;
    IntArray *b = NULL;

    status_t rc;

    if (str_equal(cmd, "load")) {
        char *path = NULL;

        if ((rc = get_array(reader, arrays, &a)) != STATUS_OK)
            return rc;

        if ((rc = get_token(reader, &path)) != STATUS_OK)
            return rc;

        rc = IntArray_load(a, path);

        free(path);

        return rc;
    }

    if (str_equal(cmd, "save")) {
        char *path = NULL;

        if ((rc = get_array(reader, arrays, &a)) != STATUS_OK)
            return rc;

        if ((rc = get_token(reader, &path)) != STATUS_OK)
            return rc;

        rc = IntArray_save(a, path);

        free(path);

        return rc;
    }

    if (str_equal(cmd, "rand")) {
        size_t count;
        int lb, rb;

        if ((rc = get_array(reader, arrays, &a)) != STATUS_OK)
            return rc;

        if ((rc = get_size(reader, &count)) != STATUS_OK)
            return rc;

        if ((rc = get_int(reader, &lb)) != STATUS_OK)
            return rc;

        if ((rc = get_int(reader, &rb)) != STATUS_OK)
            return rc;

        return IntArray_rand(a, count, lb, rb);
    }

    if (str_equal(cmd, "concat")) {
        if ((rc = get_array(reader, arrays, &a)) != STATUS_OK)
            return rc;

        if ((rc = get_array(reader, arrays, &b)) != STATUS_OK)
            return rc;

        return IntArray_concat(a, b);
    }

    if (str_equal(cmd, "free")) {
        if ((rc = get_array(reader, arrays, &a)) != STATUS_OK)
            return rc;

        return IntArray_free(a);
    }

    if (str_equal(cmd, "remove")) {
        size_t idx, count;

        if ((rc = get_array(reader, arrays, &a)) != STATUS_OK)
            return rc;

        if ((rc = get_size(reader, &idx)) != STATUS_OK)
            return rc;

        if ((rc = get_size(reader, &count)) != STATUS_OK)
            return rc;

        return IntArray_remove(a, idx, count);
    }

    if (str_equal(cmd, "copy")) {
        size_t start, end;

        if ((rc = get_array(reader, arrays, &a)) != STATUS_OK)
            return rc;

        if ((rc = get_size(reader, &start)) != STATUS_OK)
            return rc;

        if ((rc = get_size(reader, &end)) != STATUS_OK)
            return rc;

        if ((rc = get_array(reader, arrays, &b)) != STATUS_OK)
            return rc;

        return IntArray_copy(a, start, end, b);
    }

    if (str_equal(cmd, "sort")) {
        char *token = NULL;

        if ((rc = get_token(reader, &token)) != STATUS_OK)
            return rc;

        size_t idx;
        SortOrder order;

        size_t len = strlen(token);

        if (len != 2) {
            free(token);
            return STATUS_ERR_INVALID;
        }

        char suffix = token[1];

        token[1] = '\0';

        rc = parse_array_name(token, &idx);

        free(token);

        if (rc != STATUS_OK) return rc;

        if (suffix == '+') {
            order = InAscending;
        } else if (suffix == '-') {
            order = InDescending;
        } else {
            return STATUS_ERR_INVALID;
        }

        return IntArray_sort(arrays[idx], order);
    }

    if (str_equal(cmd, "shuffle")) {
        if ((rc = get_array(reader, arrays, &a)) != STATUS_OK)
            return rc;

        return IntArray_shuffle(a);
    }

    if (str_equal(cmd, "stats")) {
        if ((rc = get_array(reader, arrays, &a)) != STATUS_OK)
            return rc;

        return IntArray_stats(a);
    }

    if (str_equal(cmd, "print")) {
        if ((rc = get_array(reader, arrays, &a)) != STATUS_OK)
            return rc;

        char *token = NULL;

        int result = peek_token(reader, &token);

        if (result < 0) return STATUS_ERR_ALLOC;

        if (result == 1 && str_equal(token, "all")) {
            token = NULL;

            if ((rc = get_token(reader, &token)) != STATUS_OK)
                return rc;

            free(token);

            return IntArray_print_all(a);
        }

        if (result == 0 || !is_integer_token(token))
            return IntArray_print_all(a);

        size_t start;

        if ((rc = get_size(reader, &start)) != STATUS_OK)
            return rc;

        result = peek_token(reader, &token);

        if (result < 0) return STATUS_ERR_ALLOC;

        if (result == 0 || !is_integer_token(token))
            return IntArray_print_single(a, start);

        size_t end;

        if ((rc = get_size(reader, &end)) != STATUS_OK)
            return rc;

        return IntArray_print_range(a, start, end);
    }

    return STATUS_ERR_INVALID;
}


void interpret(FILE *f) {
    if (!f) return;

    IntArray *arrays[26] = { NULL };

    status_t rc = STATUS_OK;

    size_t i = 0;

    while (i < 26 && rc == STATUS_OK) {
        rc = IntArray_create_empty(&arrays[i]);

        if (rc == STATUS_OK)
            ++i;
    }

    if (rc == STATUS_OK) {
        TokenReader reader = {
            .f = f,
            .pending = NULL
        };

        while (true) {
            char *cmd = NULL;

            int result = read_token(&reader, &cmd);

            if (result == 0)
                break;

            if (result < 0) {
                rc = STATUS_ERR_ALLOC;
                break;
            }

            rc = interpret_command(&reader, arrays, cmd);

            if (rc != STATUS_OK) {
                fprintf(stderr, "Interpreter error during command '%s': ", cmd);
                free(cmd);
                switch (rc) {
                    default:
                        fprintf(stderr, "Unknown");
                        break;
                }
                fprintf(stderr, ".\n");
                break;
            }

            free(cmd);
        }

        token_reader_destroy(&reader);
    }

    for (size_t j = 0; j < 26; ++j) {
        if (arrays[j]) {
            IntArray_free(arrays[j]);
            free(arrays[j]);
        }
    }
}