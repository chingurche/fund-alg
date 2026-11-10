#include "mystring.h"
#include <stdlib.h>

status_t newString(char *s, String **out) {
    if (!s || !out) return STATUS_ERR_NULL_ARG;

    String *str = malloc(sizeof *str);
    if (!str) return STATUS_ERR_ALLOC;

    size_t sz = 0;
    char *c = s;
    while (*(c++) != '\0') sz++;
    str->length = sz + 1;

    str->data = malloc((sz + 1) * sizeof(char));
    if (!str->data && sz != 0) {
        free(str);
        return STATUS_ERR_ALLOC;
    }
    for (int i = 0; i < sz; i++) 
        str->data[i] = s[i];
    str->data[sz] = '\0';
    
    *out = str;
    return STATUS_OK;
}

void freeString(String *str) {
    free(str->data);
    str->length = 0;
    free(str);
}

status_t copyString(String *src, String *dst) {
    if (!src || !dst) return STATUS_ERR_NULL_ARG;

    char *tmp = malloc(src->length * sizeof *tmp);
    if (!tmp) return STATUS_ERR_ALLOC;
    
    for (int i = 0; i < src->length; i++)
        tmp[i] = src->data[i];

    free(dst->data);
    dst->data = tmp;
    dst->length = src->length;

    return STATUS_OK;
}

status_t copyNewString(String *src, String **out) {
    String *dst;
    status_t rc = newString(src->data, &dst);
    if (rc) return rc;

    *out = dst;
    return STATUS_OK;
}

status_t concateToFirstString(String *a, String *b) {
    if (!a || !b) return STATUS_ERR_NULL_ARG;

    int newlen = a->length + b->length - 1;
    char *tmp = malloc(newlen * sizeof *tmp);
    if (!tmp) return STATUS_ERR_ALLOC;
    
    for (int i = 0; i < a->length - 1; i++) {
        tmp[i] = a->data[i];
    }
    for (int i = a->length - 1, j = 0; i < newlen; i++, j++) {
        tmp[i] = b->data[j];
    }

    free(a->data);
    a->data = tmp;
    a->length = newlen;

    return STATUS_OK;
}

bool isEmptyString(String *str) {
    return !str || str->length <= 1;
}

int compareString(String *a, String *b) {
    if (a->length != b->length) {
        return a->length - b->length;
    }


    for (int i = 0; i < a->length; i++) {
        if (a->data[i] != b->data[i])
            return a->data[i] - b->data[i];
    }

    return 0;
}

bool equalString(String *a, String *b) {
    return compareString(a, b) == 0;
}