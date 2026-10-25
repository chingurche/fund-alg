#include "utils.h"

#include <string.h>

typedef struct { int line; int sym; } pos_t;

status_t find_all_subs(const char *substr, const char *path)
{
    if (substr == NULL || path == NULL)
        return STATUS_ERR_NULL_ARG;

    size_t m = strlen(substr);
    if (m == 0)
        return STATUS_ERR_FORMAT;

    FILE *f = fopen(path, "rb");
    if (!f) return STATUS_ERR_OPEN;

    size_t pi[m];
    pos_t hist[m];
    memset(hist, 0, m * sizeof(*hist));

    pi[0] = 0;
    size_t k = 0;
    for (size_t i = 1; i < m; i++) {
        while (k > 0 && substr[i] != substr[k])
            k = pi[k - 1];
        if (substr[i] == substr[k])
            k++;
        pi[i] = k;
    }

    size_t j = 0;
    int line = 1, sym = 0, count = 0;

    int c;
    while ((c = fgetc(f)) != EOF) {
        unsigned char ch = (unsigned char)c;

        int cur_line = line;
        int cur_sym  = sym + 1;

        if (m > 1)
            memmove(hist, hist + 1, (m - 1) * sizeof(*hist));
        hist[m - 1].line = cur_line;
        hist[m - 1].sym  = cur_sym;

        sym++;

        while (j > 0 && ch != (unsigned char)substr[j])
            j = pi[j - 1];
        if (ch == (unsigned char)substr[j])
            j++;

        if (j == m) {
            pos_t start = hist[0];
            if (start.line > 0)
                printf("[%d:%d] '%.*s'\n", start.line, start.sym, (int)m, substr);
            count++;
            j = pi[j - 1];
        }

        if (ch == '\n') { line++; sym = 0; }
    }

    printf("Всего подстрок: %d\n", count);

    if (fclose(f) != 0) return STATUS_ERR_CLOSE;

    return STATUS_OK;
}


status_t scan_files_for(const char *substr, const size_t pathc, char **pathv) {
    if (substr == NULL || pathv == NULL)
        return STATUS_ERR_NULL_ARG;
    status_t rc;
    
    for (int i = 0; i < pathc; i++) {
        char *path = pathv[i];
        printf("\n%s\n", path);
        rc = find_all_subs(substr, path);
        if (rc != STATUS_OK) return rc;
    }

    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    status_t rc;

    if (argc < 3) {
        fprintf(stderr, "Формат запуска программы: [программа] [подстрока] [количество путей] ...(пути)\n");
        return 1;
    }

    char *substring = argv[1];
    unsigned int target_files_count;
    rc = parse_uint(argv[2], &target_files_count);
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

    unsigned int files_count = (unsigned int)argc - 3;
    if (files_count != target_files_count) {
        fprintf(stderr, "Ожидалось файлов - %d, а введено - %d.\n", target_files_count, files_count);
        return 1;
    }

    char *paths[files_count];
    for (int i = 0; i < files_count; i++)
        paths[i] = argv[i + 3];

    scan_files_for(substring, files_count, paths);

    return 0;
}