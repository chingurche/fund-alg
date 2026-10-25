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

status_t select_files(const file_t *files, const size_t n, 
                      const size_t cap, res_t *out) {
    if (files == NULL || out == NULL)
        return STATUS_ERR_NULL_ARG;

    double efficiency[n];
    for (int i = 0; i < n; i++) {
        efficiency[i] = files[i].importance / files[i].size;
    }

    res_t res;
    res.total_size       = 0;
    res.total_importance = 0;
    res.filev = malloc(n * sizeof(file_t));
    res.filec = 0;

    int t = 0;
    while (true) {
        if (t == n) break;
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
            res.filev[t++] = file; 
            res.filec += 1; 
        }
    }

    *out = res;

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
                fprintf(stderr, "Файл с таким именем уже существует.\n");
                return 1;
            }
        }
        files[i].filename = token1;

        rc = parse_size_t(token2, &files[i].size);
        if (rc != STATUS_OK) {
            fprintf(stderr, "Что-то пошло не так при парсе size.\n");
            return 1;
        }
        rc = parse_size_t(token3, &files[i].importance);
        if (rc != STATUS_OK) {
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

    return 0;
}