#include "utils.h"

#include <string.h>

typedef struct {
    unsigned int id;
    char *first_name;
    char *last_name;
    double salary;
} Employee;

static int in_ascending(const void *a, const void *b) {
    const Employee *ea = a;
    const Employee *eb = b;
    int c;

    if (ea->salary < eb->salary) return -1;
    if (ea->salary > eb->salary) return  1;

    c = strcmp(ea->last_name, eb->last_name);
    if (c != 0) return c;
    
    c = strcmp(ea->first_name, eb->first_name);
    if (c != 0) return c;

    if (ea->id < eb->id) return -1;
    if (ea->id > eb->id) return  1;

    return 0;
}


static int in_descending(const void *a, const void *b) {
    const Employee *ea = a;
    const Employee *eb = b;
    int c;

    if (ea->salary > eb->salary) return -1;
    if (ea->salary < eb->salary) return  1;

    c = strcmp(ea->last_name, eb->last_name);
    if (c != 0) return -c;
    
    c = strcmp(ea->first_name, eb->first_name);
    if (c != 0) return -c;

    if (ea->id > eb->id) return -1;
    if (ea->id < eb->id) return  1;

    return 0;
}

status_t parse_employee(const char *src, const size_t src_size, 
                        Employee **emp, size_t *emp_size) {
    if (src == NULL || emp == NULL || emp_size == NULL)
        return STATUS_ERR_NULL_ARG;
    status_t rc;
    
    char fields[BUFSIZ][4][64];
    size_t wemployee = 0;
    size_t wfields   = 0;
    size_t wfield    = 0;
    for (size_t r = 0; r < src_size; r++) {
        char rd = src[r];
        if (rd == '\n') { continue; }
        
        if (wemployee == BUFSIZ || wfield == 64)
            return STATUS_ERR_OVERFLOW;
        if (rd == ';') {
            if (wfields != 3)
                return STATUS_ERR_PARCE;
            fields[wemployee++][wfields][wfield] = '\0';
            wfields = 0;
            wfield  = 0;
            continue;
        }
        if (rd == ',') {
            if (wfields == 3)
                return STATUS_ERR_PARCE;
            fields[wemployee][wfields++][wfield] = '\0';
            wfield = 0;
            continue;
        }
        fields[wemployee][wfields][wfield++] = rd;
    }
    if (wfields != 0 || wfield != 0)
        return STATUS_ERR_PARCE;

    size_t EMPSIZ = wemployee;
    Employee *res = malloc(EMPSIZ * sizeof(Employee));
    if (!res) return STATUS_ERR_ALLOC;
    for (int i = 0; i < EMPSIZ; i++) {
        rc = parse_uint(fields[i][0], &res[i].id);
        if (rc != STATUS_OK) return rc;
        res[i].first_name = fields[i][1];
        res[i].last_name  = fields[i][2];
        rc = parse_double(fields[i][3], &res[i].salary);
        if (rc != STATUS_OK) return rc;
        if (res[i].salary < 0) return STATUS_ERR_PARCE;
    } 
    *emp = res;
    *emp_size = EMPSIZ;
    
    return STATUS_OK;
}

status_t serialize_employee(const Employee *emp, const size_t emp_size,
                            char **dst, size_t *dst_size) {
    if (emp == NULL || dst == NULL || dst_size == NULL)
        return STATUS_ERR_NULL_ARG;
    
    size_t cap = BUFSIZ;
    char *buf = malloc(cap * sizeof(char));
    if (!buf) return STATUS_ERR_ALLOC;
    size_t w = 0;
    for (int i = 0; i < emp_size; i++) {
        size_t n = (size_t)snprintf(buf + w, cap - w, "%u,%s,%s,%.1f;\n", 
            emp[i].id, 
            emp[i].first_name, 
            emp[i].last_name, 
            emp[i].salary);
        if (w + n >= cap) {
            cap += BUFSIZ;
            char *tmp = realloc(buf, cap * sizeof(char));
            if (!tmp) {
                free(buf);
                return STATUS_ERR_ALLOC;
            }
            buf = tmp;
            i--;
            continue;
        }
        w += n;
    }
    buf[w] = '\0';
    *dst = buf;
    *dst_size = w;
    
    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    status_t rc;

    if (argc != 4) {
        fprintf(stderr, "Формат запуска программы: [программа] [path(in)] [flag] [path(out)]\n");
        return 1;
    }

    char flag;
	if ((argv[2][0] == '-' || argv[2][0] == '/') && 
         argv[2][1] != '\0' && argv[2][2] == '\0') {
        flag = argv[2][1];
    } else {
        fprintf(stderr, "'%s' не флаг формата -X или /X\n", argv[2]);
        return 1;
    }

    char *in_path = argv[1];
    char *out_path = argv[3];
    switch (path_equal(in_path, out_path)) {
        case STATUS_OK: { break; }
        case STATUS_ERR_PATH: {
            fprintf(stderr, "Не является валидным путем.\n");
            return 1;
        }
        case STATUS_ERR_OPEN: {
            fprintf(stderr, "Пути чтения и записи совпадают.\n");
            return 1;
        }
        case STATUS_ERR_NULL_ARG: {
            fprintf(stderr, "Внутренняя ошибка.\n");
            return 1;
        }
        default: {
            fprintf(stderr, "Неизвестная ошибка.\n");
            return 1;
        }
    }

    char *src = NULL;
    size_t src_size;

    rc = read_file(in_path, &src, &src_size);
    switch (rc) {
        case STATUS_OK: { break; }
        case STATUS_ERR_OPEN: {
            fprintf(stderr, "Не удалось открыть '%s'\n", in_path);
            free(src);
            return 1;
        }
        case STATUS_ERR_ALLOC: {
            fprintf(stderr, "Ошибка выделения памяти\n");
            free(src);
            return 1;
        }
        case STATUS_ERR_READ: {
            fprintf(stderr, "Ошибка чтения '%s'\n", in_path);
            free(src);
            return 1;
        }
        case STATUS_ERR_CLOSE: {
            fprintf(stderr, "Ошибка закрытия '%s'\n", in_path);
            free(src);
            return 1;
        }
        default: {
            fprintf(stderr, "Внутренняя ошибка чтения\n");
            free(src);
            return 1;
        }
    }

    Employee *emp = NULL;
    size_t emp_size;
    rc = parse_employee(src, src_size, &emp, &emp_size);
    free(src);
    switch (rc) {
        case STATUS_OK: { break; }
        case STATUS_ERR_PARCE: {
            fprintf(stderr, "Ошибка формата .employee\n");
            free(emp);
            return 1;
        }
        case STATUS_ERR_ALLOC: {
            fprintf(stderr, "Ошибка выделения памяти\n");
            free(emp);
            return 1;
        }
        case STATUS_ERR_OVERFLOW: {
            fprintf(stderr, "Переполнение буфера (внутри)\n");
            free(emp);
            return 1;
        }
        case STATUS_ERR_NULL_ARG: {
            fprintf(stderr, "Внутренняя ошибка\n");
            free(emp);
            return 1;
        }
        default: {
            fprintf(stderr, "Неизвестная ошибка\n");
            free(emp);
            return 1;
        }
    }

    switch (flag) {
        case 'a': {
            qsort(emp, emp_size, sizeof(*emp), in_ascending);
            break;
        }
        case 'd': {
            qsort(emp, emp_size, sizeof(*emp), in_descending);
            break;
        }
        default: {
            fprintf(stderr, "Существующие флаги: a и d.\n");
            free(emp);
            return 1;
        }
    }
    
    char *dst = NULL;
    size_t dst_size;
    rc = serialize_employee(emp, emp_size, &dst, &dst_size);
    free(emp);
    switch (rc) {
        case STATUS_OK: { break; }
        case STATUS_ERR_PARCE: {
            fprintf(stderr, "Ошибка парсинга .employee\n");
            free(emp);
            return 1;
        }
        case STATUS_ERR_ALLOC: {
            fprintf(stderr, "Ошибка выделения памяти\n");
            free(emp);
            return 1;
        }
        case STATUS_ERR_NULL_ARG: {
            fprintf(stderr, "Внутренняя ошибка\n");
            free(emp);
            return 1;
        }
        default: {
            fprintf(stderr, "Неизвестная ошибка\n");
            free(emp);
            return 1;
        }
    }

    rc = write_file(out_path, dst, dst_size);
    switch (rc) {
        case STATUS_OK: { break; }
        case STATUS_ERR_OPEN: {
            fprintf(stderr, "Не удалось открыть '%s' для записи\n", out_path);
            free(dst);
            return 1;
        }
        case STATUS_ERR_WRITE: {
            fprintf(stderr, "Ошибка записи в '%s'\n", out_path);
            free(dst);
            return 1;
        }
        case STATUS_ERR_CLOSE: {
            fprintf(stderr, "Ошибка закрытия '%s'\n", out_path);
            free(dst);
            return 1;
        }
        default: {
            fprintf(stderr, "Внутренняя ошибка записи\n");
            free(dst);
            return 1;
        }
    }

    free(dst);
}