#include "student.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

bool is_student_file(const char *path) {
    if (path == NULL) return false;

    const char *ext = strrchr(path, '.');
    if (ext == NULL) return false;

    const char *slash = strrchr(path, '/');
    if (slash != NULL && ext < slash) {
        return false;
    }
    
    return strcmp(ext, ".student") == 0;
}

status_t parse_student(char *path, Student ***out, size_t *size) {
    if (path == NULL || out == NULL || size == NULL)
        return STATUS_ERR_NULL_ARG;
    status_t rc;

    if (!is_student_file(path)) return STATUS_ERR_PATH;

    FILE *f = fopen(path, "r");
    if (!f) return STATUS_ERR_OPEN;

    size_t cap = 16;
    Student **buf = malloc(cap * sizeof *buf);
    if (!buf) {
        fclose(f);
        return STATUS_ERR_ALLOC;
    }

    size_t w = 0;
    char line[BUFSIZ];
    while (fgets(line, sizeof line, f) != NULL) {
        if (w + 1 > cap) {
            cap *= 2;
            Student **tmp = realloc(buf, cap * sizeof *buf);
            if (!tmp) {
                for (int i = w - 1; i >= 0; i--)
                    freeStudent(buf[i]);
                free(buf);
                fclose(f);
                return STATUS_ERR_ALLOC;
            }
            buf = tmp;
        }

        unsigned id;
        char fname_buf[64];
        char lname_buf[64];
        char group_buf[64];
        unsigned *g = malloc(GRADECOUNT * sizeof *g);
        if (!g) {
            for (int i = w - 1; i >= 0; i--)
                freeStudent(buf[i]);
            free(buf);
            fclose(f);
            return STATUS_ERR_ALLOC;
        }
        int consumed = 0;
        int n = sscanf(
            line, "%u %63s %63s %63s %u %u %u %u %u%n", 
            &id, fname_buf, lname_buf, group_buf, 
            &g[0], &g[1], &g[2], &g[3], &g[4],
            &consumed
        );
        if (n != (4 + GRADECOUNT)) {
            free(g);
            for (int i = w - 1; i >= 0; i--)
                freeStudent(buf[i]);
            free(buf);
            fclose(f);
            return STATUS_ERR_PARSE;
        }
        for (int i = consumed; line[i] != '\0'; i++) {
            if (!isspace((unsigned char)line[i])) {
                free(g);
                for (int i = w - 1; i >= 0; i--)
                    freeStudent(buf[i]);
                free(buf);
                fclose(f);
                return STATUS_ERR_PARSE;
            }
        }

        rc = newStudent(
            id,
            fname_buf,
            lname_buf,
            group_buf,
            g,
            &buf[w]
        );
        if (rc) {
            free(g);
            for (int i = w - 1; i >= 0; i--)
                freeStudent(buf[i]);
            free(buf);
            fclose(f);
            return rc;
        }
        w++;
    }

    if (fclose(f) != 0) {
        for (size_t i = 0; i < w; i++)
            freeStudent(buf[i]);
        free(buf);
        return STATUS_ERR_CLOSE;
    }

    if (w == 0) {
        free(buf);
        *out = NULL;
        *size = 0;
        return STATUS_OK;
    }

    Student **tmp = realloc(buf, w * sizeof *buf);
    if (!tmp) {
        for (int i = w - 1; i >= 0; i--)
            freeStudent(buf[i]);
        free(buf);
        return STATUS_ERR_ALLOC;
    }
    buf = tmp;

    *out = buf;
    *size = w;
    return STATUS_OK;
}

void handleDialog(char *data_path, char *log_path) {
    status_t rc;

    Student **data;
    size_t data_count;
    rc = parse_student(data_path, &data, &data_count);
    if (rc) {
        fprintf(stderr, "Произошла ошибка при парсе файла '%s': ", data_path);
        switch (rc) {
            case STATUS_ERR_PATH:
                fprintf(stderr, "Файл не расширения '.student'.\n");
                break;
            case STATUS_ERR_OPEN:
                fprintf(stderr, "Не удалось открыть файл.\n");
                break;
            case STATUS_ERR_CLOSE:
                fprintf(stderr, "Не удалось закрыть файл.\n");
                break;
            case STATUS_ERR_PARSE:
                fprintf(stderr, "Невалидный формат файла.\n");
                break;
            case STATUS_ERR_ALLOC:
                fprintf(stderr, "Не удалось выделить память.\n");
                break;
            case STATUS_ERR_INVALID:
                fprintf(stderr, "Невалидные данные в файле.\n");
                break;
            case STATUS_ERR_NULL_ARG:
                fprintf(stderr, "Внутренняя ошибка.\n");
                break;
            default:
                fprintf(stderr, "Неизвестная ошибка.\n");
                break;
        }
        return;
    }
    printf("(1) Найти студента\n");
    printf("(2) Отсортировать студентов\n");
    printf("(3) Найти студента по id и вывести его данные в трассировочный файл\n");
    printf("(4) Найти и вывести в трассировочный файл студентов, чей средний балл за все экзамены выше среднего\n");
    printf("(0) Выйти\n");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Формат запуска программы: [программа] [путь] [путь].\n");
        return 1;
    }

    handleDialog(argv[1], argv[2]);

    return 0;
}