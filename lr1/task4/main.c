#include <utils.h>

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

// здесь начал использовать size_t в соответствии с общепринятыми нормами
// а также начал писать return 0; в main как хорошо, а return 1; как обработанная ошибка

status_t add_out_prefix_to(const char *path, char *buf, size_t capacity) {
    if (path == NULL || buf == NULL)
        return STATUS_ERR_NULL_ARG;

    static const char PREFIX[] = "out_";
    const size_t prefix_len = sizeof(PREFIX) - 1;   /* 4 */

    const char *slash = strrchr(path, '/');
    size_t dir_len = (slash == NULL) ? 0 : (size_t)(slash - path) + 1;

    size_t path_len = strlen(path);
    size_t name_len = path_len - dir_len;

    if (dir_len + prefix_len + name_len + 1 > capacity)
        return STATUS_ERR_RANGE;

    memcpy(buf, path, dir_len);

    memcpy(buf + dir_len, PREFIX, prefix_len);

    memcpy(buf + dir_len + prefix_len,
           path + dir_len,
           name_len + 1);

    return STATUS_OK;
}

status_t read_file(const char *path, char **buf, size_t *size) {
    if (path == NULL || buf == NULL || size == NULL)
        return STATUS_ERR_NULL_ARG;

    FILE *f = fopen(path, "rb");
    if (!f)
        return STATUS_ERR_OPEN;

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return STATUS_ERR_READ;
    }

    long sz = ftell(f);
    if (sz < 0) {
        fclose(f);
        return STATUS_ERR_READ;
    }

    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return STATUS_ERR_READ;
    }

    char *data = malloc((size_t)sz + 1);
    if (!data) {
        fclose(f);
        return STATUS_ERR_ALLOC;
    }

    size_t read = fread(data, 1, (size_t)sz, f);
    if (read != (size_t)sz) {
        free(data);
        fclose(f);
        return STATUS_ERR_READ;
    }

    data[sz] = '\0';

    if (fclose(f) != 0) {
        free(data);
        return STATUS_ERR_CLOSE;
    }

    *buf  = data;
    *size = (size_t)sz;
    return STATUS_OK;
}

status_t remove_digits(const char *src, const size_t src_size,
                       char **dst, size_t *dst_size) {
    if (src == NULL || dst == NULL || dst_size == NULL)
        return STATUS_ERR_NULL_ARG;

    char *out = malloc(src_size + 1);
    if (!out)
        return STATUS_ERR_ALLOC;

    size_t w = 0;
    for (size_t r = 0; r < src_size; ++r) {
        if (!isdigit((unsigned char)src[r]))
            out[w++] = src[r];
    }
    out[w] = '\0';

    *dst = out;
    *dst_size = w;
    return STATUS_OK;
}

status_t count_latin(const char *src, const size_t src_size,
                     char **dst, size_t *dst_size) {
    if (src == NULL || dst == NULL || dst_size == NULL)
        return STATUS_ERR_NULL_ARG;

    char *out = malloc(src_size + 1);
    if (!out)
        return STATUS_ERR_ALLOC;

    size_t w = 0;
    size_t latin = 0;
    for (size_t r = 0; r < src_size; ++r) {
        if (isalpha(src[r])) {
            latin++;    
        }
        if (src[r] == '\n') {
            size_t written = (size_t)snprintf(out + w, src_size + 1 - w, "%zu", latin);
            if (written < 0 || (size_t)written >= src_size + 1 - w) {
                free(out);
                return STATUS_ERR_RANGE;
            }
            w += (size_t)written;
            out[w++] = '\n';
            latin = 0;
        }
    }
    if (src_size > 0 && src[src_size - 1] != '\n') {
        size_t written = (size_t)snprintf(out + w, src_size + 1 - w, "%zu", latin);
        if (written < 0 || (size_t)written >= src_size + 1 - w) {
            free(out);
            return STATUS_ERR_RANGE;
        }
        w += (size_t)written;
    }
    out[w] = '\0';

    *dst = out;
    *dst_size = w;
    return STATUS_OK;
}

status_t count_special(const char *src, const size_t src_size,
                     char **dst, size_t *dst_size) {
    if (src == NULL || dst == NULL || dst_size == NULL)
        return STATUS_ERR_NULL_ARG;

    char *out = malloc(src_size + 1);
    if (!out)
        return STATUS_ERR_ALLOC;

    size_t w = 0;
    size_t special = 0;
    for (size_t r = 0; r < src_size; ++r) {
        if (src[r] == '\n') {
            size_t written = (size_t)snprintf(out + w, src_size + 1 - w, "%zu", special);
            if (written < 0 || (size_t)written >= src_size + 1 - w) {
                free(out);
                return STATUS_ERR_RANGE;
            }
            w += (size_t)written;
            out[w++] = '\n';
            special = 0;
        } else if (!isalpha(src[r]) && !isdigit(src[r]) && src[r] != ' ') {
            special++;
        }
    }
    if (src_size > 0 && src[src_size - 1] != '\n') {
        size_t written = (size_t)snprintf(out + w, src_size + 1 - w, "%zu", special);
        if (written < 0 || (size_t)written >= src_size + 1 - w) {
            free(out);
            return STATUS_ERR_RANGE;
        }
        w += (size_t)written;
    }
    out[w] = '\0';

    *dst = out;
    *dst_size = w;
    return STATUS_OK;
}

status_t replace_hex(const char *src, size_t src_size,
                     char **dst, size_t *dst_size) {
    if (src == NULL || dst == NULL || dst_size == NULL)
        return STATUS_ERR_NULL_ARG;

    size_t cap = src_size * 2 + 1;
    char *out = malloc(cap);
    if (!out)
        return STATUS_ERR_ALLOC;

    static const char hex[] = "0123456789ABCDEF";
    size_t w = 0;

    for (size_t r = 0; r < src_size; ++r) {
        unsigned char c = (unsigned char)src[r];

        if (isdigit(c)) {
            out[w++] = (char)c;
        } else {
            out[w++] = hex[(c >> 4) & 0x0F];
            out[w++] = hex[c & 0x0F];
        }
    }
    out[w] = '\0';

    *dst = out;
    *dst_size = w;
    return STATUS_OK;
}

status_t write_file(const char *path, const char *buf, size_t size) {
    if (path == NULL || buf == NULL)
        return STATUS_ERR_NULL_ARG;

    FILE *f = fopen(path, "wb");
    if (!f)
        return STATUS_ERR_OPEN;

    size_t written = fwrite(buf, 1, size, f);
    if (written != size) {
        fclose(f);
        return STATUS_ERR_WRITE;
    }

    if (fclose(f) != 0)
        return STATUS_ERR_CLOSE;

    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Формат запуска программы: [программа] [флаг] [вход] [опционально выход]\n");
        return 0;
    }

    char flag;
    bool is_n = false;
	if (argv[1][0] != '-' && argv[1][0] != '/') {
        fprintf(stderr, "Формат флага: -X или /X\n");
        return 0;
    }
    if (argv[1][1] == 'n') {
        is_n = true;
        if (argv[1][2] != '\0' && argv[1][3] == '\0') {
            flag = argv[1][2];
        } else {
            fprintf(stderr, "Формат флага с опциональным символом n: -nX или /nX\n");
            return 0;
        }
    } else if(argv[1][1] != '\0' && argv[1][2] == '\0') {
        is_n = false;
        flag = argv[1][1];
    } else {
        fprintf(stderr, "Формат флага: -X или /X\n");
        return 0;
    }

    const char *in_path = argv[2];
    const char *out_path;
    char nbuf[512];
    if (!is_n) {
        if (argc < 4) {
            fprintf(stderr, "Выходной путь должен записываться третьим аргументом (иначе запуск с опциональным флагом n)\n");
            return 0;
        }
        out_path = argv[3];
    } else {
        status_t rc = add_out_prefix_to(in_path, nbuf, sizeof(nbuf));
        switch (rc) {
            case STATUS_ERR_RANGE:
                fprintf(stderr, "Ошибка формирования пути: буфера не хватило\n");
                return 1;
            case STATUS_ERR_NULL_ARG:
                fprintf(stderr, "Ошибка формирования пути: NULL\n");
                return 1;
        }
        out_path = nbuf;
    }

    char *src = NULL;
    size_t src_size;

    status_t rc = read_file(in_path, &src, &src_size);
    switch (rc) {
        case STATUS_OK:
            break;
        case STATUS_ERR_OPEN:
            fprintf(stderr, "Не удалось открыть '%s'\n", in_path);
            free(src);
            return 1;
        case STATUS_ERR_ALLOC:
            fprintf(stderr, "Ошибка выделения памяти\n");
            free(src);
            return 1;
        case STATUS_ERR_READ:
            fprintf(stderr, "Ошибка чтения '%s'\n", in_path);
            free(src);
            return 1;
        case STATUS_ERR_CLOSE:
            fprintf(stderr, "Ошибка закрытия '%s'\n", in_path);
            free(src);
            return 1;
        default:
            fprintf(stderr, "Внутренняя ошибка чтения\n");
            free(src);
            return 1;
    }

    char *dst = NULL;
    size_t dst_size;
    switch (flag) {
        case 'd': {
            rc = remove_digits(src, src_size, &dst, &dst_size);

            if (rc != STATUS_OK) {
                fprintf(stderr, "Ошибка обработки\n");
                free(src);
                free(dst);
                return 1;
            }
            break;
        }
        case 'i': {
            rc = count_latin(src, src_size, &dst, &dst_size);

            if (rc != STATUS_OK) {
                fprintf(stderr, "Ошибка обработки\n");
                free(src);
                free(dst);
                return 1;
            }
            break;
        }
        case 's': {
            rc = count_special(src, src_size, &dst, &dst_size);

            if (rc != STATUS_OK) {
                fprintf(stderr, "Ошибка обработки\n");
                free(src);
                free(dst);
                return 1;
            }
            break;
        }
        case 'a': {
            rc = replace_hex(src, src_size, &dst, &dst_size);

            if (rc != STATUS_OK) {
                fprintf(stderr, "Ошибка обработки\n");
                free(src);
                free(dst);
                return 1;
            }
            break;
        }
        default:
            fprintf(stderr, "Существующие флаги: d, i, s, a\n");
            free(src);
            free(dst);
            return 1;
    }

    rc = write_file(out_path, dst, dst_size);
    switch (rc) {
        case STATUS_OK:
            break;
        case STATUS_ERR_OPEN:
            fprintf(stderr, "Не удалось открыть '%s' для записи\n", out_path);
            free(src);
            free(dst);
            return 1;
        case STATUS_ERR_WRITE:
            fprintf(stderr, "Ошибка записи в '%s'\n", out_path);
            free(src);
            free(dst);
            return 1;
        case STATUS_ERR_CLOSE:
            fprintf(stderr, "Ошибка закрытия '%s'\n", out_path);
            free(src);
            free(dst);
            return 1;
        default:
            fprintf(stderr, "Внутренняя ошибка записи\n");
            free(src);
            free(dst);
            return 1;
    }

    free(src);
    free(dst);
}