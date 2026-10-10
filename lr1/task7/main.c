#include <utils.h>

#include <stdbool.h>
#include <ctype.h>
#include <string.h>

static size_t max(size_t a, size_t b) {
    return (a > b) ? a : b;
}

static size_t min(size_t a, size_t b) {
    return (a < b) ? a : b;
}

status_t merge_lexemes(const char *src1, const size_t src1_size,
                       const char *src2, const size_t src2_size,
                       char **dst, size_t *dst_size) {
    if (src1 == NULL || src2 == NULL || dst == NULL || dst_size == NULL)
        return STATUS_ERR_NULL_ARG;

    const size_t cap = 2 * (src1_size + src2_size) + 1;
    char *out = malloc(cap);
    if (!out)
        return STATUS_ERR_ALLOC;

    size_t max_src_size = max(src1_size, src2_size);
    // во втором оказывается тоже нужны лексемы
    // поэтому придется второй флаг другим способом
    const size_t BUF_LEXEME_SIZE = 64;
    const size_t BUF_SYMBOL_SIZE = 512;
    char buf1[BUF_LEXEME_SIZE][BUF_SYMBOL_SIZE];
    size_t b1lex = 0; size_t b1sym = 0;
    char buf2[BUF_LEXEME_SIZE][BUF_SYMBOL_SIZE];
    size_t b2lex = 0; size_t b2sym = 0;
    size_t waiting_lex = 0;
    size_t w = 0;
    for (size_t r = 0; r < max_src_size; ++r) {
        if (r < src1_size) {
            char r1 = src1[r];
            if ((r1 == ' ' || r1 == '\t' || r1 == '\n')) {
                if (b1sym != 0) {
                    if (b1lex == BUF_LEXEME_SIZE)
                        return STATUS_ERR_RANGE;
                    buf1[b1lex][b1sym] = '\0';
                    b1lex++;
                    b1sym = 0;
                }
            } else {
                if (b1sym >= BUF_SYMBOL_SIZE - 1)
                    return STATUS_ERR_RANGE;
                buf1[b1lex][b1sym++] = r1;
            }
        }
        if (r < src2_size) {
            char r2 = src2[r];
            if ((r2 == ' ' || r2 == '\t' || r2 == '\n')) {
                if (b2sym != 0) {
                    if (b2lex == BUF_LEXEME_SIZE)
                        return STATUS_ERR_RANGE;
                    buf2[b2lex][b2sym] = '\0';
                    b2lex++;
                    b2sym = 0;
                }
            } else {
                if (b2sym >= BUF_SYMBOL_SIZE - 1)
                    return STATUS_ERR_RANGE;
                buf2[b2lex][b2sym++] = r2;
            }
        }

        if (min(b1lex, b2lex) == (waiting_lex + 1)) {
            w += snprintf(out + w, cap - w, "%s ", buf1[waiting_lex]);
            w += snprintf(out + w, cap - w, "%s ", buf2[waiting_lex++]);
        } else if ((src1_size <= r) && (b2lex > waiting_lex)) {
            w += snprintf(out + w, cap - w, "%s ", buf2[waiting_lex++]);
        } else if ((src2_size <= r) && (b1lex > waiting_lex)) {
            w += snprintf(out + w, cap - w, "%s ", buf1[waiting_lex++]);
        }
    }
    if (b1sym != 0) { buf1[b1lex][b1sym] = '\0'; b1lex++; }
    if (b2sym != 0) { buf2[b2lex][b2sym] = '\0'; b2lex++; }

    while (b1lex > waiting_lex && b2lex > waiting_lex) {
        w += snprintf(out + w, cap - w, "%s ", buf1[waiting_lex]);
        w += snprintf(out + w, cap - w, "%s ", buf2[waiting_lex++]);
    }
    while (b1lex > waiting_lex) {
        w += snprintf(out + w, cap - w, "%s ", buf1[waiting_lex++]);
    }
    while (b2lex > waiting_lex) {
        w += snprintf(out + w, cap - w, "%s ", buf2[waiting_lex++]);
    }

    out[w] = '\0';

    *dst = out;
    *dst_size = w;
    return STATUS_OK;
}

static size_t num_to_base(unsigned char v, int base, char *buf) {
    char tmp[16];
    int  t = 0;

    if (v == 0) {
        buf[0] = '0';
        return 1;
    }

    while (v > 0) {
        tmp[t++] = (char)('0' + (v % (unsigned)base));
        v /= (unsigned)base;
    }

    for (int i = 0; i < t; ++i)
        buf[i] = tmp[t - 1 - i];

    return (size_t)t;
}

static void lower_inplace(char *s) {
    for (; *s; ++s)
        if (isalpha((unsigned char)*s))
            *s = (char)tolower((unsigned char)*s);
}

static size_t to_base_codes(const char *tok, int base,
                            char *out, size_t cap) {
    size_t w = 0;
    for (size_t i = 0; tok[i]; ++i) {
        char code[16];
        size_t clen = num_to_base((unsigned char)tok[i], base, code);

        if (w + clen + 1 > cap) break;
        memcpy(out + w, code, clen);
        w += clen;

        if (tok[i + 1] != '\0') {
            if (w + 1 > cap) break;
            out[w++] = ' ';
        }
    }
    return w;
}

status_t change_lexemes(const char *src, size_t src_size,
                        char **dst, size_t *dst_size) {
    if (src == NULL || dst == NULL || dst_size == NULL)
        return STATUS_ERR_NULL_ARG;

    char *copy = malloc(src_size + 1);
    if (!copy) return STATUS_ERR_ALLOC;
    memcpy(copy, src, src_size);
    copy[src_size] = '\0';

    size_t cap = src_size * 5 + src_size + 1;
    char *out = malloc(cap);
    if (!out) { free(copy); return STATUS_ERR_ALLOC; }

    size_t w = 0;
    size_t n = 0;

    char *token = strtok(copy, " \t\n");
    while (token != NULL) {
        ++n;

        if (n % 10 == 0) {
            char tmp[256];
            size_t tlen = strlen(token);
            if (tlen >= sizeof tmp) tlen = sizeof tmp - 1;
            memcpy(tmp, token, tlen);
            tmp[tlen] = '\0';

            lower_inplace(tmp);

            char buf[1024];
            size_t blen = to_base_codes(tmp, 4, buf, sizeof buf);

            if (w + blen + 1 > cap) {
                free(copy); free(out);
                return STATUS_ERR_RANGE;
            }
            memcpy(out + w, buf, blen);
            w += blen;
        }
        else if (n % 5 == 0) {
            char buf[1024];
            size_t blen = to_base_codes(token, 8, buf, sizeof buf);

            if (w + blen + 1 > cap) {
                free(copy); free(out);
                return STATUS_ERR_RANGE;
            }
            memcpy(out + w, buf, blen);
            w += blen;
        }
        else if (n % 2 == 0) {
            size_t tlen = strlen(token);
            if (w + tlen + 1 > cap) {
                free(copy); free(out);
                return STATUS_ERR_RANGE;
            }
            memcpy(out + w, token, tlen);
            out[w + tlen] = '\0';
            lower_inplace(out + w);
            w += tlen;
        }
        else {
            size_t tlen = strlen(token);
            if (w + tlen + 1 > cap) {
                free(copy); free(out);
                return STATUS_ERR_RANGE;
            }
            memcpy(out + w, token, tlen);
            w += tlen;
        }

        if (w + 1 > cap) {
            free(copy); free(out);
            return STATUS_ERR_RANGE;
        }
        if (n > 1) out[w++] = ' ';

        token = strtok(NULL, " \t\n");
    }

    out[w] = '\0';
    free(copy);

    *dst = out;
    *dst_size = w;
    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Формат запуска программы: [программа] [флаг] [flag species args]\n");
        return 1;
    }

    char flag;
	if ((argv[1][0] == '-' || argv[1][0] == '/') && 
         argv[1][1] != '\0' && argv[1][2] == '\0') {
        flag = argv[1][1];
    } else {
        fprintf(stderr, "'%s' не флаг формата -X или /X\n", argv[1]);
        return 1;
    }

    char *dst = NULL;
    size_t dst_size; 
    
    char *out_path;

    switch (flag) {
        case 'r' : {
            if (argc != 5) {
                fprintf(stderr, "Формат флага r: [программа] [r] [file1(in)] [file2(in)] [file3(out)]\n");
                return 1;
            }

            char *in_path1 = argv[2];
            char *in_path2 = argv[3];

            char *src1 = NULL, *src2 = NULL;
            size_t src1_size, src2_size;

            status_t rc1 = read_file(in_path1, &src1, &src1_size);
            status_t rc2 = read_file(in_path2, &src2, &src2_size);

            if (rc1 != STATUS_OK || rc2 != STATUS_OK) {
                fprintf(stderr, "Что-то пошло не так, но мне лень расписывать почему.\n");
                free(src1);
                free(src2);
                return 1;
            }

            status_t rc = merge_lexemes(src1, src1_size,
                                        src2, src2_size,
                                        &dst, &dst_size);
            
            if (rc != STATUS_OK) {
                fprintf(stderr, "Что-то пошло не так, но мне лень расписывать почему.\n");
                free(src1);
                free(src2);
                free(dst);
                return 1;
            }

            free(src1);
            free(src2);

            out_path = argv[4];

            break;
        }
        case 'a' : {
            if (argc != 4) {
                fprintf(stderr, "Формат флага a: [программа] [a] [file1(in)] [file2(out)]\n");
                return 1;
            }

            char *in_path = argv[2];

            char *src = NULL;
            size_t src_size;

            status_t rc = read_file(in_path, &src, &src_size);

            if (rc != STATUS_OK) {
                fprintf(stderr, "Что-то пошло не так, но мне лень расписывать почему.\n");
                free(src);
                return 1;
            }

            rc = change_lexemes(src, src_size, &dst, &dst_size);
            
            if (rc != STATUS_OK) {
                fprintf(stderr, "Что-то пошло не так, но мне лень расписывать почему.\n");
                free(src);
                free(dst);
                return 1;
            }

            free(src);

            out_path = argv[3];

            break;
        }
        default: {
            fprintf(stderr, "Существуют только флаги r и a");
            return 1;
        }
    }

    status_t rc = write_file(out_path, dst, dst_size);
    switch (rc) {
        case STATUS_OK:
            break;
        case STATUS_ERR_OPEN:
            fprintf(stderr, "Не удалось открыть '%s' для записи\n", out_path);
            free(dst);
            return 1;
        case STATUS_ERR_WRITE:
            fprintf(stderr, "Ошибка записи в '%s'\n", out_path);
            free(dst);
            return 1;
        case STATUS_ERR_CLOSE:
            fprintf(stderr, "Ошибка закрытия '%s'\n", out_path);
            free(dst);
            return 1;
        default:
            fprintf(stderr, "Внутренняя ошибка записи\n");
            free(dst);
            return 1;
    }

    free(dst);
}