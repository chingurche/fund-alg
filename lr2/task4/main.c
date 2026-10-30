#include "utils.h"

#include <string.h>
#include <stdbool.h>

typedef struct {
    char *filename;
    size_t size;
    size_t importance;
} file_t;

typedef struct {
    size_t total_size;
    size_t total_importance;
    file_t *filev;
    size_t filec;
} res_t;

/*status_t select_files(const file_t *files, const size_t n,
                      const size_t cap, res_t *out) {
    if (files == NULL || out == NULL)
        return STATUS_ERR_NULL_ARG;

    double efficiency[n];
    for (int i = 0; i < n; i++) {
        efficiency[i] = (double)files[i].importance / (double)files[i].size;
    }

    res_t res;
    res.total_size       = 0;
    res.total_importance = 0;
    bool *taken = calloc(n, sizeof(*taken));
    if (!taken) return STATUS_ERR_ALLOC;
    res.filec = 0;

    while (true) {
        if (res.filec == n) break;
        int max = 0;
        for (int i = 1; i < n; i++) {
            if (efficiency[i] > efficiency[max])
                max = i;
        }
        efficiency[max] = 0;

        file_t file = files[max];
        if ((res.total_size + file.size) > cap) break;
        else {
            res.total_size += file.size;
            res.total_importance += file.importance;
            taken[max] = true; 
            res.filec++; 
        }
    }

    res.filev = malloc(n * sizeof(file_t));
    if (!res.filev) return STATUS_ERR_ALLOC;
    int j = 0;
    for (int i = 0; i < res.total_size; i++) {
        if (taken[i])
            res.filev[j++] = files[i];
    }
    
    *out = res;

    return STATUS_OK;
}*/
// старый неправильный алгоритм

status_t select_files(const file_t *files, const size_t n,
                      const size_t cap, res_t *out) {
    if (files == NULL || out == NULL)
        return STATUS_ERR_NULL_ARG;

    out->total_size = 0;
    out->total_importance = 0;
    out->filev = NULL;
    out->filec = 0;
    if (n == 0 || cap == 0) return STATUS_OK;

    size_t *dp = calloc(cap + 1, sizeof(*dp));
    if (!dp) return STATUS_ERR_ALLOC;

    bool *take = calloc(n * (cap + 1), sizeof(*take));
    if (!take) { free(dp); return STATUS_ERR_ALLOC; }

    for (size_t i = 0; i < n; i++) {
        size_t s = files[i].size;
        size_t v = files[i].importance;
        if (s > cap) continue;

        for (size_t w = cap; w >= s; w--) {
            size_t candidate = dp[w - s] + v;
            if (candidate > dp[w]) {
                dp[w] = candidate;
                take[i * (cap + 1) + w] = true;
            }
        }
    }
    
    bool *chosen_flag = calloc(n, sizeof(*chosen_flag));
    if (!chosen_flag) { free(dp); free(take); return STATUS_ERR_ALLOC; }

    size_t w = cap;
    size_t filec = 0;
    for (size_t i = n; i-- > 0; ) {
        if (take[i * (cap + 1) + w]) {
            chosen_flag[i] = true;
            w -= files[i].size;
            filec++;
        }
    }

    free(dp);
    free(take);

    file_t *chosen = NULL;
    if (filec > 0) {
        chosen = malloc(filec * sizeof(*chosen));
        if (!chosen) { free(chosen_flag); return STATUS_ERR_ALLOC; }

        size_t j = 0;
        size_t total_size = 0, total_importance = 0;
        for (size_t i = 0; i < n; i++) {
            if (chosen_flag[i]) {
                chosen[j++] = files[i];
                total_size       += files[i].size;
                total_importance += files[i].importance;
            }
        }
        out->total_size       = total_size;
        out->total_importance = total_importance;
    }

    free(chosen_flag);

    out->filev = chosen;
    out->filec = filec;
    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    status_t rc;

    unsigned int n;
    unsigned int capacity;
    file_t *files;

    char line[BUFSIZ];
    int i = -1;
    while (fgets(line, sizeof line, stdin) != NULL) {
        if (i == -1) {
            char *token1 = strtok(line, " \n");
            char *token2 = strtok(NULL, " \n");
            
            rc = parse_uint(token1, &n);
            if (rc != STATUS_OK) {
                fprintf(stderr, "Что-то пошло не так при парсе n.\n");
                return 1;
            }
            rc = parse_uint(token2, &capacity);
            if (rc != STATUS_OK) {
                fprintf(stderr, "Что-то пошло не так при парсе capacity.\n");
                return 1;
            }
            
            files = malloc(n * sizeof(file_t));
            if (!files) {
                fprintf(stderr, "Не удалось выделить память для file_t.\n");
                return 1;
            }

            i++;
            continue;
        }

        char *token1 = strtok(line, " \n");
        char *token2 = strtok(NULL, " \n");
        char *token3 = strtok(NULL, " \n");
        
        for (int j = 0; j < i; j++) {
            if (strcmp(token1, files[j].filename) == 0) {
                for (size_t k = 0; k < n; k++)
                    free(files[k].filename);
                free(files);
                fprintf(stderr, "Файл с таким именем уже существует.\n");
                return 1;
            }
        }
        files[i].filename = strdup(token1);

        rc = parse_size_t(token2, &files[i].size);
        if (rc != STATUS_OK) {
            for (size_t k = 0; k < n; k++)
                free(files[k].filename);
            free(files);
            fprintf(stderr, "Что-то пошло не так при парсе size.\n");
            return 1;
        }
        rc = parse_size_t(token3, &files[i].importance);
        if (rc != STATUS_OK) {
            for (size_t k = 0; k < n; k++)
                free(files[k].filename);
            free(files);
            fprintf(stderr, "Что-то пошло не так при парсе importance.\n");
            return 1;
        }

        i++;
        if (i == n) break;
    }

    res_t res;
    rc = select_files(files, n, capacity, &res);
    
    printf( "TOTAL_SIZE %lu\n"
            "TOTAL_IMPORTANCE %lu\n"
            "FILES %lu\n", 
            res.total_size, 
            res.total_importance, 
            res.filec);

    for (int i = 0; i < res.filec; i++)
        printf("%s\n", res.filev[i].filename);
 
    for (size_t k = 0; k < n; k++)
        free(files[k].filename);
    free(files);
    free(res.filev);

    return 0;
}