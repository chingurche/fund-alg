#ifndef MYTOR_H
#define MYTOR_H

#include "../status.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define CHECK(rc) \
    do { \
        switch (rc) { \
            case STATUS_OK: \
                break; \
            case STATUS_ERR_OVERFLOW: \
                fprintf(stderr, "Строка может не поместиться в буфер.\n"); \
                return 1; \
            case STATUS_ERR_RANGE: \
                fprintf(stderr, "r лежит в диапазоне от 1 до 5 включительно.\n"); \
                return 1; \
            case STATUS_ERR_NULL_ARG: \
                fprintf(stderr, "Произошла внутренняя ошибка.\n"); \
                return 1; \
            default: \
                fprintf(stderr, "Произошла неизвестная ошибка.\n"); \
                return 1; \
        } \
    } while (0)


// int to base 2 pow r
static status_t inttor( const int value, const int r,
                        char * const out, const size_t size) {
    if (out == NULL)        return STATUS_ERR_NULL_ARG;
    if (r < 1 || r > 5)     return STATUS_ERR_RANGE;
    static const size_t INT_SIZ = 32;
    size_t targsize = (INT_SIZ + r - 1) / r + 2;
    if (size < targsize)    return STATUS_ERR_OVERFLOW;

    unsigned int n = (value < 0) ? -(unsigned int)value : (unsigned int)value;

    if (n == 0) {
        out[0] = '0';
        out[1] = '\0';
        return STATUS_OK;
    }

    int w = 0;
    
    if (value < 0)
        out[w++] = '-';

    bool started = false;
    unsigned int mask = (1u << r) - 1;

    int start = (int)(INT_SIZ-1) - ((int)(INT_SIZ-1) % r);
    for (int i = start; i >= 0; i -= r) {
        int digit = (n >> i) & mask; 
        
        if (digit != 0) 
            started = true;

        if (started) {
            if (digit < 10) {
                out[w++] = '0' + digit;
            } else {
                out[w++] = 'A' + (digit - 10);
            }
        }

    }

    out[w] = '\0';
    return STATUS_OK;
}

#endif