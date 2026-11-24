#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>

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


#endif