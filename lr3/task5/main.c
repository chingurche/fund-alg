#include "student.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>

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

static char *read_line(FILE *f) {
    size_t cap = 128;
    size_t len = 0;
    char *buf = malloc(cap);
    if (!buf) return NULL;

    int c;
    while ((c = fgetc(f)) != EOF) {
        if (len + 1 >= cap) {
            cap *= 2;
            char *tmp = realloc(buf, cap);
            if (!tmp) { free(buf); return NULL; }
            buf = tmp;
        }
        buf[len++] = (char)c;
        if (c == '\n') {
            buf[len] = '\0';
            return buf;
        }
    }
    if (len > 0) {
        buf[len] = '\0';
        return buf;
    }
    free(buf);
    return NULL;
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
    char *line = NULL;
    while ((line = read_line(f)) != NULL) {
        if (line[0] == '\n' || line[0] == '\0') {
            free(line);
            continue;
        }

        if (w + 1 > cap) {
            cap *= 2;
            Student **tmp = realloc(buf, cap * sizeof *buf);
            if (!tmp) {
                free(line);
                for (size_t i = 0; i < w; i++) freeStudent(buf[i]);
                free(buf);
                fclose(f);
                return STATUS_ERR_ALLOC;
            }
            buf = tmp;
        }

        char *id_str   = strtok(line, " \t\r\n");
        char *fname    = strtok(NULL, " \t\r\n");
        char *lname    = strtok(NULL, " \t\r\n");
        char *group    = strtok(NULL, " \t\r\n");

        unsigned *g = malloc(GRADECOUNT * sizeof *g);
        if (!id_str || !fname || !lname || !group || !g) {
            free(g);
            free(line);
            for (size_t i = 0; i < w; i++) freeStudent(buf[i]);
            free(buf);
            fclose(f);
            return (!g) ? STATUS_ERR_ALLOC : STATUS_ERR_PARSE;
        }

        unsigned id;
        if (sscanf(id_str, "%u", &id) != 1) {
            free(g); free(line);
            for (size_t i = 0; i < w; i++) freeStudent(buf[i]);
            free(buf); fclose(f);
            return STATUS_ERR_PARSE;
        }

        bool grades_ok = true;
        for (int i = 0; i < GRADECOUNT; i++) {
            char *grade_str = strtok(NULL, " \t\r\n");
            if (!grade_str || sscanf(grade_str, "%u", &g[i]) != 1) {
                grades_ok = false;
                break;
            }
        }
        
        char *extra = strtok(NULL, " \t\r\n");
        if (!grades_ok || extra != NULL) {
            free(g); free(line);
            for (size_t i = 0; i < w; i++) freeStudent(buf[i]);
            free(buf); fclose(f);
            return STATUS_ERR_PARSE;
        }

        rc = newStudent(id, fname, lname, group, g, &buf[w]);
        
        free(line);

        if (rc) {
            free(g);
            for (size_t i = 0; i < w; i++) freeStudent(buf[i]);
            free(buf);
            fclose(f);
            return rc;
        }
        w++;
    }

    if (fclose(f) != 0) {
        for (size_t i = 0; i < w; i++) freeStudent(buf[i]);
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
    if (tmp) buf = tmp;

    *out = buf;
    *size = w;
    return STATUS_OK;
}

status_t serialize_student(Student **arr, size_t size, char *path) {
    if (arr == NULL || path == NULL)
        return STATUS_ERR_NULL_ARG;
    status_t rc;

    if (!is_student_file(path)) return STATUS_ERR_PATH;

    FILE *f = fopen(path, "w");
    if (!f) return STATUS_ERR_OPEN;

    for (int i = 0; i < size; i++) {
        Student *s = arr[i];
        int n = fprintf(
            f, "%u %s %s %s %u %u %u %u %u\n", 
            s->id, s->first_name, s->last_name, s->group, 
            s->grade[0], s->grade[1], s->grade[2], s->grade[3], s->grade[4]
        );

        if (n < 0) {
            fclose(f);
            return STATUS_ERR_WRITE;
        }
    }

    if (fclose(f) != 0) return STATUS_ERR_CLOSE;

    return STATUS_OK;
}

float get_average_grade(Student *s) {
    float out = 0;
    for (int i = 0; i < GRADECOUNT; i++)
        out += s->grade[i];
    return out / GRADECOUNT;
}

float get_average_grade_of_all(Student **arr, size_t size) {
    float out = 0;
    for (int i = 0; i < size; i++)
        out += get_average_grade(arr[i]);
    return out / size;
}

void read_console(char *buf, size_t size) {
    if (fgets(buf, size, stdin) != NULL) {
        buf[strcspn(buf, "\n")] = 0;
    } else {
        buf[0] = '\0';
    }
}

void handle_dialog(char *data_path, char *log_path) {
    status_t rc;

    Student **data = NULL;
    size_t data_count = 0;
    rc = parse_student(data_path, &data, &data_count);
    if (rc) {
        fprintf(stderr, "Произошла ошибка при парсинге '%s': ", data_path);
        switch (rc) {
            case STATUS_ERR_PATH:     fprintf(stderr, "Файл не расширения '.student'.\n"); break;
            case STATUS_ERR_OPEN:     fprintf(stderr, "Не удалось открыть файл.\n"); break;
            case STATUS_ERR_CLOSE:    fprintf(stderr, "Не удалось закрыть файл.\n"); break;
            case STATUS_ERR_PARSE:    fprintf(stderr, "Невалидный формат файла.\n"); break;
            case STATUS_ERR_ALLOC:    fprintf(stderr, "Не удалось выделить память.\n"); break;
            case STATUS_ERR_INVALID:  fprintf(stderr, "Невалидные данные в файле.\n"); break;
            case STATUS_ERR_NULL_ARG: fprintf(stderr, "Внутренняя ошибка.\n"); break;
            default:                  fprintf(stderr, "Неизвестная ошибка.\n"); break;
        }
        return;
    }

    char *line = NULL;

    while (true) {
        printf(
            "\n"
            "(1) Найти студента\n"
            "(2) Отсортировать студентов\n"
            "(3) Вывести в трассировочный файл найденного по id студента\n"
            "(4) Вывести в трассировочный файл всех студентов чей средний балл выше среднего\n"
            "(0) Выйти\n"
            "> "
        );

        line = read_line(stdin);
        if (!line) break;

        line[strcspn(line, "\n")] = 0;

        if (line[0] == '\0') {
            free(line);
            continue;
        }
        if (line[0] == '0' && line[1] == '\0') {
            break;
        }

        switch (line[0]) {
            case '1': {
                printf("(1) Найти по id\n(2) Найти по фамилии\n(3) Найти по имени\n(4) Найти по группе\n> ");
                free(line);
                line = read_line(stdin);
                if (!line) break;
                line[strcspn(line, "\n")] = 0;

                if (data_count == 0) {
                    printf("База данных пуста.\n");
                    break;
                }

                Student **fnd = malloc(data_count * sizeof(*fnd));
                if (!fnd) {
                    fprintf(stderr, "Не удалось выделить память.\n");
                    break;
                }
                size_t fnd_count = 0;

                if (strcmp(line, "1") == 0) {
                    printf("Введите id: ");
                    free(line);
                    line = read_line(stdin);
                    if (line) {
                        line[strcspn(line, "\n")] = 0;
                        unsigned id;
                        if (sscanf(line, "%u", &id) == 1) {
                            for (size_t i = 0; i < data_count; i++) {
                                if (data[i]->id == id) fnd[fnd_count++] = data[i];
                            }
                        } else {
                            printf("Неверный формат id.\n");
                        }
                    }
                } else if (strcmp(line, "2") == 0) {
                    printf("Введите фамилию: ");
                    free(line);
                    line = read_line(stdin);
                    if (line) {
                        line[strcspn(line, "\n")] = 0;
                        for (size_t i = 0; i < data_count; i++) {
                            if (strcmp(data[i]->last_name, line) == 0) 
                                fnd[fnd_count++] = data[i];
                        }
                    }
                } else if (strcmp(line, "3") == 0) {
                    printf("Введите имя: ");
                    free(line);
                    line = read_line(stdin);
                    if (line) {
                        line[strcspn(line, "\n")] = 0;
                        for (size_t i = 0; i < data_count; i++) {
                            if (strcmp(data[i]->first_name, line) == 0) 
                                fnd[fnd_count++] = data[i];
                        }
                    }
                } else if (strcmp(line, "4") == 0) {
                    printf("Введите группу: ");
                    free(line);
                    line = read_line(stdin);
                    if (line) {
                        line[strcspn(line, "\n")] = 0;
                        for (size_t i = 0; i < data_count; i++) {
                            if (strcmp(data[i]->group, line) == 0) 
                                fnd[fnd_count++] = data[i];
                        }
                    }
                } else {
                    fprintf(stderr, "Такой опции нет.\n");
                    free(fnd);
                    break;
                }

                if (fnd_count == 0) {
                    printf("Записей не найдено.\n");
                } else {
                    printf("Найдено записей: %zu\n", fnd_count);
                    for (size_t i = 0; i < fnd_count; i++) {
                        printf(
                            "%u %s %s %s %.2f\n",
                            fnd[i]->id,
                            fnd[i]->first_name,
                            fnd[i]->last_name,
                            fnd[i]->group,
                            get_average_grade(fnd[i])
                        );
                    }
                }
                free(fnd);
                break;
            }
            case '2': {
                printf("(1) Сортировка по id\n(2) Сортировка по фамилии\n(3) Сортировка по имени\n(4) Сортировка по группе\n> ");
                free(line);
                line = read_line(stdin);
                if (!line) break;
                line[strcspn(line, "\n")] = 0;

                if (strcmp(line, "1") == 0) {
                    qsort(data, data_count, sizeof(*data), compare_id);
                } else if (strcmp(line, "2") == 0) {
                    qsort(data, data_count, sizeof(*data), compare_last_name);
                } else if (strcmp(line, "3") == 0) {
                    qsort(data, data_count, sizeof(*data), compare_first_name);
                } else if (strcmp(line, "4") == 0) {
                    qsort(data, data_count, sizeof(*data), compare_group);
                } else {
                    fprintf(stderr, "Такой опции нет.\n"); 
                    break;
                }
                
                printf("Студенты отсортированы.\n");
                rc = serialize_student(data, data_count, data_path);
                if (rc) {
                    fprintf(stderr, "Ошибка сериализации в '%s': код %d\n", data_path, rc);
                    break;
                }
                printf("Обновлен файл '%s'.\n", data_path);
                break;
            }
            case '3': {
                printf("Введите id: ");
                free(line);
                line = read_line(stdin);
                if (!line) break;
                line[strcspn(line, "\n")] = 0;

                unsigned id;
                if (sscanf(line, "%u", &id) != 1) {
                    fprintf(stderr, "Это не unsigned.\n");
                    break;
                }
                    
                Student *student = NULL;
                for (size_t i = 0; i < data_count; i++) {
                    if (data[i]->id == id) {
                        student = data[i];
                        break;
                    }
                }
                if (student == NULL) {
                    fprintf(stderr, "Не найдено.\n"); 
                    break;
                }

                FILE *f = fopen(log_path, "a");
                if (!f) {
                    fprintf(stderr, "Не удалось открыть файл.\n");
                    break;
                }

                int n = fprintf(
                    f,
                    "Найдено по id(%u): %s %s %s %.2f\n",
                    student->id, student->first_name, student->last_name,
                    student->group, get_average_grade(student)
                );

                if (n < 0) {
                    fprintf(stderr, "Не удалось записать в файл.\n");
                } else if (fclose(f) != 0) {
                    fprintf(stderr, "Не удалось закрыть файл.\n");
                    break;
                } else {
                    printf("Данные были записаны в трассировочный файл.\n");
                }
                if (n < 0) fclose(f);
                break;
            }
            case '4': {
                if (data_count == 0) {
                    printf("База данных пуста.\n");
                    break;
                }
                float avg = get_average_grade_of_all(data, data_count);
                Student **fnd = malloc(data_count * sizeof(*fnd));
                if (!fnd) {
                    fprintf(stderr, "Не удалось выделить память.\n"); 
                    break;
                }
                
                size_t fnd_count = 0;
                for (size_t i = 0; i < data_count; i++) {
                    if (get_average_grade(data[i]) > avg)
                        fnd[fnd_count++] = data[i];
                }

                if (fnd_count == 0) {
                    printf("Записей не найдено.\n");
                    free(fnd);
                    break;
                }
                
                printf("Найдено записей: %zu\n", fnd_count);

                FILE *f = fopen(log_path, "a");
                if (!f) {
                    fprintf(stderr, "Не удалось открыть файл.\n");
                    free(fnd);
                    break;
                }
                
                fprintf(f, "Запись студентов чей средний балл за все экзамены выше среднего (%.2f).\n", avg);
                
                for (size_t i = 0; i < fnd_count; i++) {
                    fprintf(
                        f,
                        "%u %s %s %s %.2f\n",
                        fnd[i]->id, fnd[i]->first_name, fnd[i]->last_name,
                        fnd[i]->group, get_average_grade(fnd[i])
                    );
                }
                free(fnd);

                if (fclose(f) != 0) {
                    fprintf(stderr, "Не удалось закрыть файл.\n");
                    break;
                }
                printf("Данные были записаны в трассировочный файл.\n");
                break;
            }
            default:
                printf("Такой опции нет.\n");
                break;
        }
        
        free(line);
        line = NULL;
    }
    
    if (line) free(line);

    for (size_t i = 0; i < data_count; i++) {
        freeStudent(data[i]);
    }
    free(data);
}

status_t path_equal(char *path1, char *path2) {
    struct stat st1, st2;
    
    if (stat(path1, &st1) != 0) {
        return STATUS_ERR_PATH;
    }
    if (stat(path2, &st2) != 0){
        return STATUS_OK;
    }
    
    if (st1.st_dev == st2.st_dev && st1.st_ino == st2.st_ino) {
        return STATUS_ERR_OPEN;
    }

    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Формат запуска программы: [программа] [путь] [путь].\n");
        return 1;
    }

    status_t rc = path_equal(argv[1], argv[2]);
    if (rc == STATUS_ERR_PATH) {
        fprintf(stderr, "Пути одинаковО!\n");
        return 1;
    }

    handle_dialog(argv[1], argv[2]);

    return 0;
}