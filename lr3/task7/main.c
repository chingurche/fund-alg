#include "liver.h"
#include "undo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*  Здесь было много работы - очень много различных решений рассмотрено
    Идеи что можно добавить: .liver.bak, поиск не по первому попавшемуся */

static const char MSG_INV_CMD[] = "Неизвестная команда.\n";

bool is_liver_file(const char *path) {
    if (path == NULL) return false;

    const char *ext = strrchr(path, '.');
    if (ext == NULL) return false;

    const char *slash = strrchr(path, '/');
    if (slash != NULL && ext < slash) {
        return false;
    }
    
    return strcmp(ext, ".liver") == 0;
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
            if (!tmp) {
                free(buf);
                return NULL;
            }
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

status_t parse_liver(char *path, Liver **out) {
    if (path == NULL || out == NULL)
        return STATUS_ERR_NULL_ARG;
    status_t rc;

    if (!is_liver_file(path)) return STATUS_ERR_PATH;

    FILE *f = fopen(path, "r");
    if (!f) return STATUS_ERR_OPEN;

    Liver *root = NULL;

    char *line;
    while ((line = read_line(f)) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        
        char *p = line;
        char *lname = strsep(&p, ";");
        char *fname = strsep(&p, ";");
        char *patro = strsep(&p, ";");
        char *bdate = strsep(&p, ";");
        char *gender_str = strsep(&p, ";");
        char *salary_str = strsep(&p, ";");
        char *extra = strsep(&p, ";");

        if (!lname || !fname || !patro || !bdate || 
            !gender_str || !salary_str || !extra != '\0' || 
            p != NULL) {
            free(line);
            fclose(f);
            Liver_free(root, true);
            return STATUS_ERR_PARSE;
        }

        if (strlen(gender_str) > 1) {
            free(line);
            fclose(f);
            Liver_free(root, true);
            return STATUS_ERR_PARSE;
        }
        char gender_val = gender_str[0];
        char *endptr = NULL;
        float salary_val = strtof(salary_str, &endptr);
        if (*endptr != '\0' || endptr == salary_str) {
            free(line);
            fclose(f);
            Liver_free(root, true);
            return STATUS_ERR_PARSE;
        }

        Liver *liver = NULL;
        rc = Liver_init(
            lname,
            fname,
            patro,
            bdate,
            gender_val,
            salary_val,
            &liver
        );
        free(line);
        if (rc) {
            fclose(f);
            Liver_free(root, true);
            return rc;
        }
        
        rc = LiverList_add(&root, liver);
        if (rc) {
            fclose(f);
            Liver_free(root, true);
            return rc;
        }
    }

    if (fclose(f) != 0) { 
        Liver_free(root, true);
        return STATUS_ERR_CLOSE;
    }

    *out = root;
    return STATUS_OK;
}

status_t serialize_liver(Liver *root, char *path) {
    if (path == NULL) // корректно при root == NULL
        return STATUS_ERR_NULL_ARG;
    status_t rc;

    if (!is_liver_file(path)) return STATUS_ERR_PATH;

    FILE *f = fopen(path, "w");
    if (!f) return STATUS_ERR_OPEN;

    for (Liver *l = root; l; l = l->next) {
        int n = fprintf(
            f, "%s;%s;%s;%s;%c;%.2f;\n", 
            l->last_name,
            l->first_name,
            l->patronymic,
            l->birth_date,
            l->gender,
            l->salary
        );

        if (n < 0) {
            fclose(f);
            return STATUS_ERR_WRITE;
        }
    }

    if (fclose(f) != 0) return STATUS_ERR_CLOSE;

    return STATUS_OK;
}

int handle_liver_dialog(EventList **history, Liver *liver) {
    status_t rc;

    char *line = NULL;
    while (true) {
        int n = printf(
            "\n"
            "%s %s %s\n%s %c %.2f\n"
            "[1] Изменить фамилию  [2] Изменить имя  [3] Изменить отчество\n"
            "[4] Изменить дату     [5] Изменить пол  [6] Изменить зарплату\n"
            "[D] Удалить\n"
            "[Q] Вернуться\n"
            "> ", 
            liver->last_name,
            liver->first_name,
            liver->patronymic,
            liver->birth_date,
            liver->gender,
            liver->salary
        );

        line = read_line(stdin);
        if (!line) return -1;

        line[strcspn(line, "\r\n")] = '\0';

        if (strlen(line) != 1) {
            fprintf(stderr, MSG_INV_CMD);
            free(line);
            continue;
        }

        char command = line[0];

        free(line);
        line = NULL;

        switch (command) {
            case '1': {
                printf("> ");
                line = read_line(stdin);
                if (!line) return -1;
                line[strcspn(line, "\n")] = 0;
                Event *e;
                rc = Event_init(SET_LAST_NAME, &e);
                if (rc != STATUS_OK) {
                    free(line);
                    fprintf(stderr, "Не удалось инициализировать Event.\n");
                    break;
                }

                e->target = liver;
                e->set_last_name.old = strdup(liver->last_name);

                if (!e->set_last_name.old) {
                    Event_free(e);
                    free(line);
                    fprintf(stderr, MSG_ALLOC);
                    break;
                }
                rc = Liver_set_last_name(liver, line);
                free(line);
                if (rc != STATUS_OK) {
                    Event_free(e);
                    fprintf(stderr, "Не удалось изменить фамилию.\n");
                    break;
                }

                rc = EventList_add_Event(*history, e);
                if (rc != STATUS_OK) {
                    Liver_set_last_name(liver, e->set_last_name.old);
                    Event_free(e);
                    fprintf(stderr, "Не удалось записать в историю изменений.\n");
                    return rc;
                }

                break;
            }
            case '2': {
                printf("> ");
                line = read_line(stdin);
                if (!line) return -1;
                line[strcspn(line, "\r\n")] = 0;

                Event *e = NULL;
                rc = Event_init(SET_FIRST_NAME, &e);
                if (rc != STATUS_OK) {
                    free(line);
                    fprintf(stderr, "Не удалось инициализировать Event.\n");
                    break;
                }

                e->target = liver;
                e->set_first_name.old = strdup(liver->first_name);

                if (!e->set_first_name.old) {
                    Event_free(e);
                    free(line);
                    fprintf(stderr, MSG_ALLOC);
                    break;
                }

                rc = Liver_set_first_name(liver, line);
                free(line);

                if (rc != STATUS_OK) {
                    Event_free(e);
                    fprintf(stderr, "Не удалось изменить имя.\n");
                    break;
                }

                rc = EventList_add_Event(*history, e);
                if (rc != STATUS_OK) {
                    Liver_set_first_name(liver, e->set_first_name.old);
                    Event_free(e);
                    fprintf(stderr, "Ошибка записи в историю.\n");
                }

                break;
            }
            case '3': {
                printf("> ");
                line = read_line(stdin);
                if (!line) return -1;
                line[strcspn(line, "\r\n")] = 0;

                Event *e = NULL;
                rc = Event_init(SET_PATRONYMIC, &e);
                if (rc != STATUS_OK) {
                    free(line);
                    fprintf(stderr, "Не удалось инициализировать Event.\n");
                    break;
                }

                e->target = liver;
                e->set_patronymic.old = strdup(liver->patronymic);

                if (!e->set_patronymic.old) {
                    Event_free(e);
                    free(line);
                    fprintf(stderr, MSG_ALLOC);
                    break;
                }

                rc = Liver_set_patronymic(liver, line);
                free(line);

                if (rc != STATUS_OK) {
                    Event_free(e);
                    fprintf(stderr, "Не удалось изменить отчество.\n");
                    break;
                }

                rc = EventList_add_Event(*history, e);
                if (rc != STATUS_OK) {
                    Liver_set_patronymic(liver, e->set_patronymic.old);
                    Event_free(e);
                    fprintf(stderr, "Ошибка записи в историю.\n");
                }

                break;
            }
            case '4': {
                printf("> ");
                line = read_line(stdin);
                if (!line) return -1;
                line[strcspn(line, "\r\n")] = 0;

                Event *e = NULL;
                rc = Event_init(SET_BIRTH_DATE, &e);
                if (rc != STATUS_OK) {
                    free(line);
                    fprintf(stderr, "Не удалось инициализировать Event.\n");
                    break;
                }

                e->target = liver;
                e->set_birth_date.old = strdup(liver->birth_date);

                if (!e->set_birth_date.old) {
                    Event_free(e);
                    free(line);
                    fprintf(stderr, MSG_ALLOC);
                    break;
                }

                rc = Liver_set_birth_date(liver, line);
                free(line);

                if (rc != STATUS_OK) {
                    Event_free(e);
                    fprintf(stderr, "Не удалось изменить дату рождения.\n");
                    break;
                }

                rc = EventList_add_Event(*history, e);
                if (rc != STATUS_OK) {
                    Liver_set_birth_date(liver, e->set_birth_date.old);
                    Event_free(e);
                    fprintf(stderr, "Ошибка записи в историю.\n");
                }

                break;
            }
            case '5': {
                printf("> ");
                line = read_line(stdin);
                if (!line) return -1;
                line[strcspn(line, "\r\n")] = 0;

                if (strlen(line) != 1) {
                    fprintf(stderr, MSG_INV_CMD);
                    free(line);
                    break;
                }

                Event *e = NULL;
                rc = Event_init(SET_GENDER, &e);
                if (rc != STATUS_OK) {
                    free(line);
                    fprintf(stderr, "Не удалось инициализировать Event.\n");
                    break;
                }

                e->target = liver;
                e->set_gender.old = liver->gender;

                rc = Liver_set_gender(liver, line[0]);
                free(line);

                if (rc != STATUS_OK) {
                    Event_free(e);
                    fprintf(stderr, "Не удалось изменить пол.\n");
                    break;
                }

                rc = EventList_add_Event(*history, e);
                if (rc != STATUS_OK) {
                    Liver_set_gender(liver, e->set_gender.old);
                    Event_free(e);
                    fprintf(stderr, "Ошибка записи в историю.\n");
                }

                break;
            }
            case '6': {
                printf("> ");
                line = read_line(stdin);
                if (!line) return -1;
                line[strcspn(line, "\r\n")] = 0;

                char *endptr = NULL;
                float salary = strtof(line, &endptr);

                if (endptr == line || *endptr != '\0') {
                    fprintf(stderr, MSG_INV_CMD);
                    free(line);
                    break;
                }

                Event *e = NULL;
                rc = Event_init(SET_SALARY, &e);
                if (rc != STATUS_OK) {
                    free(line);
                    fprintf(stderr, "Не удалось инициализировать Event.\n");
                    break;
                }

                e->target = liver;
                e->set_salary.old = liver->salary;

                rc = Liver_set_salary(liver, salary);
                free(line);

                if (rc != STATUS_OK) {
                    Event_free(e);
                    fprintf(stderr, "Не удалось изменить зарплату.\n");
                    break;
                }

                rc = EventList_add_Event(*history, e);
                if (rc != STATUS_OK) {
                    Liver_set_salary(liver, e->set_salary.old);
                    Event_free(e);
                    fprintf(stderr, "Ошибка записи в историю.\n");
                }

                break;
            }
            case 'D': case 'd':
                return 0;
            case 'Q': case 'q':
                return 1;
            default:
                break;
        }
    }
}

void handle_main_dialog(EventList **history, Liver **root) {
    status_t rc;

    char *line = NULL;

    while (true) {
        printf(
            "\n"
            "[1] Найти жителя\n"
            "[2] Добавить жителя\n"
            "[3] Выгрузить в файл\n"
            "[Z] Undo (осталось: %lu)\n"
            "[Q] Выйти\n"
            "> ", (*history)->N
        );

        line = read_line(stdin);
        if (!line) return;

        line[strcspn(line, "\r\n")] = '\0';

        if (strlen(line) != 1) {
            fprintf(stderr, MSG_INV_CMD);
            free(line);
            continue;
        }

        char command = line[0];

        free(line);
        line = NULL;

        switch (command) {
            case '1':
                printf(
                    "\n"
                    "[1] По фамилии\n"
                    "[2] По имени\n"
                    "[3] По отчеству\n"
                    "[4] По дате рождения\n"
                    "> "
                );

                line = read_line(stdin);
                if (!line) return;
                line[strcspn(line, "\n")] = 0;
                FindOption option;
                if      (line[0] == '1') option = BY_LAST_NAME;
                else if (line[0] == '2') option = BY_FIRST_NAME;
                else if (line[0] == '3') option = BY_PATRONYMIC;
                else if (line[0] == '4') option = BY_BIRTH_DATE;
                else {
                    fprintf(stderr, MSG_INV_CMD);
                    free(line);
                    break;
                }

                if (strlen(line) != 1) {
                    fprintf(stderr, MSG_INV_CMD);
                    free(line);
                    break;
                }
                free(line);

                printf("> ");
                line = read_line(stdin);
                if (!line) return;
                line[strcspn(line, "\n")] = 0;
                
                Liver *found = NULL;
                rc = LiverList_find(
                    *root, 
                    option, 
                    line, 
                    &found
                );
                free(line);
                line = NULL;
                if (rc == STATUS_ERR_NOT_FOUND) {
                    fprintf(stderr, MSG_NOT_FOUND);
                    break;
                }
                
                int dialog_rc = handle_liver_dialog(history, found);

                if (dialog_rc == -1)
                    return;

                if (dialog_rc == 0) {
                    Event *e = NULL;

                    rc = Event_init(REMOVE_LIVER, &e);
                    if (rc != STATUS_OK) {
                        fprintf(stderr, "Не удалось создать событие.\n");
                        break;
                    }

                    Liver *prev = NULL;
                    Liver *cur = *root;

                    while (cur && cur != found) {
                        prev = cur;
                        cur = cur->next;
                    }

                    if (!cur) {
                        Event_free(e);
                        fprintf(stderr, MSG_NOT_FOUND);
                        break;
                    }

                    e->target = found;
                    e->remove_liver.predecessor = prev;

                    rc = LiverList_remove(root, found);

                    if (rc != STATUS_OK) {
                        Event_free(e);
                        fprintf(stderr, "Не удалось удалить жителя.\n");
                        break;
                    }

                    e->delete = true;

                    rc = EventList_add_Event(*history, e);

                    if (rc != STATUS_OK) {
                        if (prev)
                            prev->next = found;
                        else
                            *root = found;

                        e->delete = false;
                        Event_free(e);

                        fprintf(stderr, "Не удалось сохранить событие удаления.\n");
                        break;
                    }
                }

                break;
            case '2': {
                char *data[6] = {NULL};
                const char *prompts[] = {
                    "Фамилия: ",
                    "Имя: ",
                    "Отчество: ",
                    "Дата рождения (DD.MM.YYYY): ",
                    "Пол (M/W): ",
                    "Зарплата: "
                };

                bool input_error = false;

                for (size_t i = 0; i < 6; i++) {
                    printf("%s", prompts[i]);

                    data[i] = read_line(stdin);

                    if (!data[i]) {
                        input_error = true;
                        break;
                    }

                    data[i][strcspn(data[i], "\r\n")] = '\0';
                }

                if (input_error) {
                    for (size_t i = 0; i < 6; i++)
                        free(data[i]);
                    return;
                }

                if (strlen(data[4]) != 1) {
                    fprintf(stderr, MSG_INVALID);
                    for (size_t i = 0; i < 6; i++)
                        free(data[i]);
                    break;
                }

                char *endptr = NULL;
                float salary = strtof(data[5], &endptr);

                if (endptr == data[5] || *endptr != '\0') {
                    fprintf(stderr, MSG_INVALID);
                    for (size_t i = 0; i < 6; i++)
                        free(data[i]);
                    break;
                }

                Liver *other = NULL;

                rc = Liver_init(
                    data[0],
                    data[1],
                    data[2],
                    data[3],
                    data[4][0],
                    salary,
                    &other
                );

                for (size_t i = 0; i < 6; i++)
                    free(data[i]);

                if (rc != STATUS_OK) {
                    fprintf(stderr, "Не удалось создать жителя.\n");
                    break;
                }

                Event *e = NULL;

                rc = Event_init(ADD_LIVER, &e);

                if (rc != STATUS_OK) {
                    Liver_free(other, false);
                    fprintf(stderr, "Не удалось создать событие.\n");
                    break;
                }

                e->target = other;

                rc = LiverList_add(root, other);

                if (rc != STATUS_OK) {
                    e->target = NULL;
                    Event_free(e);
                    Liver_free(other, false);
                    fprintf(stderr, "Не удалось добавить жителя.\n");
                    break;
                }

                rc = EventList_add_Event(*history, e);

                if (rc != STATUS_OK) {
                    LiverList_remove(root, other);
                    other->next = NULL;

                    e->target = NULL;
                    Event_free(e);
                    Liver_free(other, false);

                    fprintf(stderr, "Ошибка записи в историю.\n");
                }

                break;
            }
            case '3':
                printf("Введите путь для сохранения (.liver): ");

                char *path = read_line(stdin);
                if (!path)
                    return;

                path[strcspn(path, "\r\n")] = '\0';

                if (!is_liver_file(path)) {
                    fprintf(stderr, MSG_PATH);
                    free(path);
                    break;
                }

                rc = serialize_liver(*root, path);

                if (rc != STATUS_OK) {
                    fprintf(stderr, "Ошибка сохранения файла: ");

                    switch (rc) {
                        case STATUS_ERR_PATH:   fprintf(stderr, MSG_PATH); break;
                        case STATUS_ERR_OPEN:   fprintf(stderr, MSG_OPEN); break;
                        case STATUS_ERR_WRITE:  fprintf(stderr, MSG_WRITE); break;
                        case STATUS_ERR_CLOSE:  fprintf(stderr, MSG_CLOSE); break;
                        default:                fprintf(stderr, MSG_UNKNOWN); break;
                    }

                    free(path);
                    break;
                }

                printf("Список успешно сохранён в %s\n", path);

                free(path);
                break;
            case 'Z': case 'z':
                rc = EventList_undo(*history, root);
                if (rc) {
                    fprintf(stderr, "Произошла ошибка при Undo: ");
                    switch (rc) {
                        case STATUS_ERR_NOT_FOUND:  fprintf(stderr, MSG_NOT_FOUND); break;
                        case STATUS_ERR_INVALID:    fprintf(stderr, MSG_INVALID); break;
                        case STATUS_ERR_ALLOC:      fprintf(stderr, MSG_ALLOC); break;
                        case STATUS_ERR_NULL_ARG:   fprintf(stderr, MSG_NULL_ARG); break;
                        default:                    fprintf(stderr, MSG_UNKNOWN); break;
                    }
                }
                break;
            case 'Q': case 'q':
                return;
            default:
                fprintf(stderr, MSG_INV_CMD);
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    status_t rc;
    if (argc < 2) {
        fprintf(stderr, "Использование: %s .liver\n", argv[0]);
        return 1;
    }
    
    EventList *history = NULL;
    rc = EventList_init(&history);
    if (rc) {
        fprintf(stderr, "Произошла ошибка при инициализации EventList: ");
        switch (rc) {
            default:                    fprintf(stderr, MSG_UNKNOWN); break;
        }
        return 1;
    }

    Liver *root = NULL;
    rc = parse_liver(argv[1], &root);
    if (rc) {
        fprintf(stderr, "Произошла ошибка при парсинге %s: ", argv[1]);
        switch (rc) {
            case STATUS_ERR_PATH:       fprintf(stderr, MSG_PATH); break;
            case STATUS_ERR_PARSE:      fprintf(stderr, MSG_PARSE); break;
            case STATUS_ERR_INVALID:    fprintf(stderr, MSG_INVALID); break;
            case STATUS_ERR_ALLOC:      fprintf(stderr, MSG_ALLOC); break;
            case STATUS_ERR_OPEN:       fprintf(stderr, MSG_OPEN); break;
            case STATUS_ERR_READ:       fprintf(stderr, MSG_READ); break;
            case STATUS_ERR_CLOSE:      fprintf(stderr, MSG_CLOSE); break;
            case STATUS_ERR_NULL_ARG:   fprintf(stderr, MSG_NULL_ARG); break;
            default:                    fprintf(stderr, MSG_UNKNOWN); break;
        }
        EventList_free(history);
        return 1;
    }

    handle_main_dialog(&history, &root);

    EventList_free(history);
    Liver_free(root, true);

    return 1;
}