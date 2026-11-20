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


static size_t bit_add(size_t a, size_t b) {
    while (b != 0) {
        size_t carry = a & b;
        a = a ^ b;
        b = carry << 1;
    }
    return a;
}

static size_t bit_sub(size_t a, size_t b) {
    return bit_add(a, bit_add(~b, 1));
}

static size_t bit_neg(size_t a) {
    return bit_add(~a, 1);
}

static char digit_to_char(int digit) {
    if (digit < 10) {
        return (char)bit_add((size_t)'0', (size_t)digit);
    } else {
        return (char)bit_add(bit_sub((size_t)'A', 10), (size_t)digit);
    }
}

static status_t inttor( const int value, const int r,
                        char * const out, const size_t size) {
    if (out == NULL)        return STATUS_ERR_NULL_ARG;
    if (r < 1 || r > 5)     return STATUS_ERR_RANGE;
    if (size == 0)          return STATUS_ERR_OVERFLOW;

    size_t n = (value < 0) ? bit_neg((size_t)value) : (size_t)value;

    char *p = out;
    size_t ptr_addr = (size_t)p; 
    size_t written = 0;
    
    size_t max_writable = bit_sub(size, 1);

    if (n == 0) {
        if (written >= max_writable) return STATUS_ERR_OVERFLOW;
        
        *p = '0';
        ptr_addr = bit_add((size_t)ptr_addr, 1);
        p = (char *)ptr_addr;
        written = bit_add(written, 1);
        
        *p = '\0';
        return STATUS_OK;
    }
    
    if (value < 0) {
        if (written >= max_writable) return STATUS_ERR_OVERFLOW;
        
        *p = '-';
        ptr_addr = bit_add((size_t)ptr_addr, 1);
        p = (char *)ptr_addr;
        written = bit_add(written, 1);
    }

    bool started = false;
    size_t mask = ~(~0u << r);

    int start;
    switch (r) {
        case 1: start = 31; break;
        case 2: start = 30; break;
        case 3: start = 30; break;
        case 4: start = 28; break;
        case 5: start = 30; break;
        default: return STATUS_ERR_RANGE; 
    }
    for (int i = start; i >= 0; i = (int)bit_sub((size_t)i, (size_t)r)) {
        int digit = (n >> i) & mask; 
        
        if (digit != 0) 
            started = true;

        if (started) {
            if (written >= max_writable) return STATUS_ERR_OVERFLOW;
            
            *p = digit_to_char(digit);
            ptr_addr = bit_add((size_t)ptr_addr, 1);
            p = (char *)ptr_addr;
            written = bit_add(written, 1);
        }
    }

    *p = '\0';
    return STATUS_OK;
}

#endif