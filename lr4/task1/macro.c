#include "../status.h"
#include "macro.h"
#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

static bool is_empty_c(char c) {
    return (c == ' ') || (c == '\t');
}

static bool is_def_name_char(char ch) {
    return isalnum((unsigned char)ch);
}

static char *create_out_path(const char *path) {
    static const char EXTENSION[] = ".rep";

    size_t old_len = strlen(path);
    size_t ext_len = sizeof(EXTENSION) - 1;
    size_t new_len = old_len + ext_len + 1;

    char *new_path = malloc(new_len);
    if (!new_path) return NULL;

    memcpy(new_path, path, old_len);
    memcpy(new_path + old_len, EXTENSION, ext_len + 1);

    return new_path;
}

static status_t read_macro(FILE *f, HashNode **out) {
    status_t rc;

    int c;
    char ch;

    /*#*/
    if ((c = fgetc(f)) != '#') {
        ungetc(c, f);
        return STATUS_END_PARSE;
    }

    /* \t*/
    while (is_empty_c(c = fgetc(f)));
    ungetc(c, f);

    /*define*/
    static const char sample[] = "define";
    size_t r = 0;
    while (r < strlen(sample)) {
        c = fgetc(f);
        if (c == EOF) {
            return STATUS_ERR_PARSE;
        }

        ch = (char)c;
        if (ch != sample[r++]) {
            return STATUS_ERR_PARSE;
        }
    }
    if (!is_empty_c(c = fgetc(f))) return STATUS_ERR_PARSE;

    /* \t*/
    while (is_empty_c(c = fgetc(f)));
    ungetc(c, f);

    /*def_name*/
    size_t name_cap = 16;
    size_t name_len = 0;
    char *def_name = malloc(name_cap);
    if (!def_name) return STATUS_ERR_ALLOC;
    while (true) {
        c = fgetc(f);

        if (is_empty_c(c)) break;

        if (c == EOF || !is_def_name_char((char)c)) {
            free(def_name);
            return STATUS_ERR_PARSE;
        }

        char ch = (char)c;

        if (name_len + 1 >= name_cap) {
            name_cap *= 2;
            char *tmp = realloc(def_name, name_cap);
            if (!tmp) {
                free(def_name);
                return STATUS_ERR_ALLOC;
            }
            def_name = tmp;
        }

        def_name[name_len++] = ch;
    }
    def_name[name_len] = '\0';

    /* \t*/
    while (is_empty_c(c = fgetc(f)));
    ungetc(c, f);

    /*value*/
    size_t value_cap = 16;
    size_t value_len = 0;
    char *value = malloc(value_cap);
    if (!value) {
        free(def_name);
        return STATUS_ERR_ALLOC;
    }
    while (true) {
        c = fgetc(f);

        if (c == '\r') continue; 

        if (c == '\n') {
            if (value_len > 0 && value[value_len - 1] == '\\') {
                value_len--;
                continue;
            }

            break;
        }

        if (c == EOF) break;

        char ch = (char)c;

        if (value_len + 1 >= value_cap) {
            value_cap *= 2;
            char *tmp = realloc(value, value_cap);
            if (!tmp) {
                free(def_name);
                free(value);
                return STATUS_ERR_ALLOC;
            }
            value = tmp;
        }

        value[value_len++] = ch;
    }
    value[value_len] = '\0';

    HashNode *node = NULL;
    rc = HashNode_create(def_name, value, &node);
    free(def_name);
    free(value);
    if (rc) return rc;

    *out = node;

    return STATUS_OK;
}

status_t macro_replace(const char *path) {
    status_t rc;
    if (!path) return STATUS_ERR_NULL_ARG;

    HashTable *table = NULL;
    rc = HashTable_create(128, &table);
    if (rc) return rc; 

    FILE *f_in = fopen(path, "rb");
    if (!f_in) {
        HashTable_free(table);
        return STATUS_ERR_OPEN;
    }

    while (true) {
        HashNode *node = NULL;
        rc = read_macro(f_in, &node);

        if (rc) {
            if (rc == STATUS_END_PARSE) break;
            HashTable_free(table);
            fclose(f_in);
            return rc;
        }
        
        rc = HashTable_add_HashNode(table, node);
        if (rc == STATUS_NEED_RESIZE) {
            HashTable *new_table = NULL;

            rc = HashTable_resize(table, &new_table);

            if (rc) {
                HashTable_free(table);
                fclose(f_in);
                return rc;
            }

            HashTable_free(table);
            table = new_table;
        }
        else if (rc != STATUS_OK) {
            HashNode_free(node, false);
            HashTable_free(table);
            fclose(f_in);
            return rc;
        }
    }

    char *out_path = create_out_path(path);
    if (!out_path) {
        HashTable_free(table);
        fclose(f_in);
        return STATUS_ERR_ALLOC;
    }

    FILE *f_out = fopen(out_path, "wb");
    free(out_path);
    if (!f_out) {
        HashTable_free(table);
        fclose(f_in);
        return STATUS_ERR_ALLOC;
    }

    size_t cap = 16;
    size_t len = 0;
    char *buf = malloc(cap);
    if (!buf) {
        HashTable_free(table);
        fclose(f_in);
        fclose(f_out);
        return STATUS_ERR_ALLOC;
    }
    while (true) {
        int c = fgetc(f_in);

        if (c == EOF) {
            if (len != 0) {
                buf[len] = '\0';
                HashNode *node = NULL;
                rc = HashTable_find_HashNode(table, buf, &node);
                if (rc == STATUS_OK) {
                    fputs(node->value, f_out);
                } else if (rc == STATUS_ERR_NOT_FOUND) {
                    fputs(buf, f_out);
                } else {
                    HashTable_free(table);
                    fclose(f_in);
                    fclose(f_out);
                    free(buf);
                    return rc;
                }
            }
            break;
        }
        
        char ch = (char)c;

        if (!is_def_name_char(ch)) {
            if (len != 0) {
                buf[len] = '\0';
                HashNode *node = NULL;
                rc = HashTable_find_HashNode(table, buf, &node);
                if (rc == STATUS_OK) {
                    fputs(node->value, f_out);
                } else if (rc == STATUS_ERR_NOT_FOUND) {
                    fputs(buf, f_out);
                } else {
                    HashTable_free(table);
                    fclose(f_in);
                    fclose(f_out);
                    free(buf);
                    return rc;
                }
            }
            len = 0;
            fputc(c, f_out);
            continue;
        }

        if (len + 1 >= cap) {
            cap *= 2;
            char *tmp = realloc(buf, cap);
            if (!tmp) {
                HashTable_free(table);
                fclose(f_in);
                fclose(f_out);
                free(buf);
                return STATUS_ERR_ALLOC;
            }
            buf = tmp;
        }
        buf[len++] = ch;
    }

    fclose(f_in);
    fclose(f_out);

    return STATUS_OK;
}