#ifndef STRBLDR_H
#define STRBLDR_H

#include <string.h>
#include <stdlib.h>

typedef struct {
    char   *data;
    size_t  len;
    size_t  cap;
} sb_t;

static status_t sb_init(sb_t *sb) {
    sb->cap = 256;
    sb->len = 0;
    sb->data = malloc(sb->cap);
    if (!sb->data) return STATUS_ERR_ALLOC;
    sb->data[0] = '\0';
    return STATUS_OK;
}

static status_t sb_append(sb_t *sb, const char *s, size_t n) {
    if (sb->len + n + 1 > sb->cap) {
        size_t new_cap = sb->cap;
        while (new_cap < sb->len + n + 1) new_cap *= 2;
        char *tmp = realloc(sb->data, new_cap);
        if (!tmp) return STATUS_ERR_ALLOC;
        sb->data = tmp;
        sb->cap = new_cap;
    }
    memcpy(sb->data + sb->len, s, n);
    sb->len += n;
    sb->data[sb->len] = '\0';
    return STATUS_OK;
}

static void sb_free(sb_t *sb) {
    free(sb->data);
    sb->data = NULL;
    sb->len = sb->cap = 0;
}

#endif