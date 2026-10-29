#ifndef OVERUTILS_H
#define OVERUTILS_H

#include "utils.h"

#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

status_t int_to_roman(int n, char *buf, size_t size) {
    if (buf == NULL) return STATUS_ERR_NULL_ARG;
    if (size == 0)   return STATUS_ERR_OVERFLOW;

    static const struct {
        int value;
        const char *sym;
    } table[] = {
        {1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"},
        { 100, "C"}, { 90, "XC"}, { 50, "L"}, { 40, "XL"},
        {  10, "X"}, {  9, "IX"}, {  5, "V"}, {  4, "IV"},
        {   1, "I"}
    };

    buf[0] = '\0';

    if (n <= 0 || n > 3999) return STATUS_ERR_RANGE;

    size_t pos = 0;
    for (size_t i = 0; i < sizeof(table) / sizeof(table[0]); i++) {
        while (n >= table[i].value) {
            size_t len = strlen(table[i].sym);
            if (pos + len >= size) {
                buf[pos] = '\0';
                return STATUS_ERR_OVERFLOW;
            }
            memcpy(buf + pos, table[i].sym, len);
            pos += len;
            n -= table[i].value;
        }
    }
    buf[pos] = '\0';
    return STATUS_OK;
}

status_t uint_to_zeckendorf(unsigned int n, char *buf, size_t size) {
    if (buf == NULL) return STATUS_ERR_NULL_ARG;
    if (size == 0)   return STATUS_ERR_OVERFLOW;

    static unsigned int fib[46];
    fib[0] = 1u; fib[1] = 2u;
    for (int i = 2; i < 46; ++i) {
        fib[i] = fib[i - 1] + fib[i - 2];
    }

    buf[0] = '\0';

    unsigned char bits[46] = {0};
    int top = -1;

    for (int i = 45; i >= 0; i--) {
        if (fib[i] <= n) {
            bits[i] = 1;
            n -= fib[i];
            if (top < 0) top = i;
        }
    }

    size_t pos = 0;
    if (top < 0) {
        if (pos + 1 >= size) {
            buf[pos] = '\0';
            return STATUS_ERR_OVERFLOW;
        }
        buf[pos++] = '1';
        buf[pos] = '\0';
        return STATUS_OK;
    }

    for (int i = 0; i <= top; i++) {
        if (pos + 1 >= size) {
            buf[pos] = '\0';
            return STATUS_ERR_OVERFLOW;
        }
        buf[pos++] = bits[i] ? '1' : '0';
    }

    if (pos + 1 >= size) {
        buf[pos] = '\0';
        return STATUS_ERR_OVERFLOW;
    }
    buf[pos++] = '1';
    buf[pos] = '\0';
    return STATUS_OK;
}

status_t int_to_base(int v, int b, bool up, char *buf, size_t size) {
    if (buf == NULL) return STATUS_ERR_NULL_ARG;
    if (size == 0)   return STATUS_ERR_OVERFLOW;
    if (b < 2 || b > 36) return STATUS_ERR_INVALID;

    static const char digits_lower[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    static const char digits_upper[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char *digits = up ? digits_upper : digits_lower;

    buf[0] = '\0';

    unsigned int uv;
    bool neg = false;
    if (v < 0) {
        neg = true;
        uv = (unsigned int)(-(unsigned int)v);
    } else {
        uv = (unsigned int)v;
    }

    char tmp[34];
    size_t len = 0;

    do {
        tmp[len++] = digits[uv % (unsigned int)b];
        uv /= (unsigned int)b;
    } while (uv != 0);

    size_t need = len + (neg ? 1 : 0) + 1;
    if (need > size) return STATUS_ERR_OVERFLOW;

    size_t pos = 0;
    if (neg) buf[pos++] = '-';
    while (len > 0) {
        buf[pos++] = tmp[--len];
    }
    buf[pos] = '\0';
    return STATUS_OK;
}

status_t based_to_int(const char *s, int b, bool up, char *buf, size_t size) {
    if (buf == NULL) return STATUS_ERR_NULL_ARG;
    if (size == 0)   return STATUS_ERR_OVERFLOW;
    if (s == NULL || b < 2 || b > 36) return STATUS_ERR_INVALID;

    buf[0] = '\0';

    while (isspace((unsigned char)*s)) s++;

    bool neg = false;
    if (*s == '-') { neg = true; s++; }
    else if (*s == '+') { s++; }

    unsigned long long acc = 0;
    bool any = false;

    for (; *s; s++) {
        int d;
        unsigned char c = (unsigned char)*s;

        if (c >= '0' && c <= '9')             d = c - '0';
        else if (up && c >= 'A' && c <= 'Z')  d = c - 'A' + 10;
        else if (!up && c >= 'a' && c <= 'z') d = c - 'a' + 10;
        else break;

        if (d >= b) break;

        any = true;

        if (acc > (ULLONG_MAX - (unsigned long long)d) / (unsigned long long)b)
            return STATUS_ERR_RANGE;

        acc = acc * b + (unsigned)d;
    }

    if (!any) return STATUS_ERR_FORMAT;

    int value;
    if (neg) {
        if (acc > (unsigned long long)INT_MIN) return STATUS_ERR_RANGE;
        value = -(int)acc;
    } else {
        if (acc > (unsigned long long)INT_MAX) return STATUS_ERR_RANGE;
        value = (int)acc;
    }

    int n = snprintf(buf, size, "%d", value);
    if (n < 0) return STATUS_ERR_FORMAT;
    if ((size_t)n >= size) {
        buf[0] = '\0';
        return STATUS_ERR_OVERFLOW;
    }

    return STATUS_OK; 
}

status_t memdump_int32(const int32_t v, char *buf, size_t size) {
    if (buf == NULL) return STATUS_ERR_NULL_ARG;
    if (size == 0)   return STATUS_ERR_OVERFLOW;
    buf[0] = '\0';

    if (size < 36) return STATUS_ERR_OVERFLOW;

    const unsigned char *bytes = (const unsigned char *)&v;

    size_t pos = 0;
    for (size_t i = 0; i < sizeof v; i++) {
        if (i > 0) buf[pos++] = ' ';
        for (int bit = 7; bit >= 0; bit--) {
            buf[pos++] = ((bytes[i] >> bit) & 1u) ? '1' : '0';
        }
    }
    buf[pos] = '\0';
    return STATUS_OK;
}

status_t memdump_uint32(const uint32_t v, char *buf, size_t size) {
    if (buf == NULL) return STATUS_ERR_NULL_ARG;
    if (size == 0)   return STATUS_ERR_OVERFLOW;
    buf[0] = '\0';

    if (size < 36) return STATUS_ERR_OVERFLOW;

    const unsigned char *bytes = (const unsigned char *)&v;

    size_t pos = 0;
    for (size_t i = 0; i < sizeof v; i++) {
        if (i > 0) buf[pos++] = ' ';
        for (int bit = 7; bit >= 0; bit--) {
            buf[pos++] = ((bytes[i] >> bit) & 1u) ? '1' : '0';
        }
    }
    buf[pos] = '\0';
    return STATUS_OK;
}

status_t memdump_float(float v, char *buf, size_t size)
{
    if (buf == NULL) return STATUS_ERR_NULL_ARG;
    if (size == 0)   return STATUS_ERR_OVERFLOW;
    buf[0] = '\0';

    size_t need = sizeof v * 8 + (sizeof v - 1) + 1;
    if (size < need) return STATUS_ERR_OVERFLOW;

    const unsigned char *bytes = (const unsigned char *)&v;

    size_t pos = 0;
    for (size_t i = 0; i < sizeof v; i++) {
        if (i > 0) buf[pos++] = ' ';
        for (int bit = 7; bit >= 0; bit--) {
            buf[pos++] = ((bytes[i] >> bit) & 1u) ? '1' : '0';
        }
    }
    buf[pos] = '\0';
    return STATUS_OK;
}

status_t memdump_double(double v, char *buf, size_t size)
{
    if (buf == NULL) return STATUS_ERR_NULL_ARG;
    if (size == 0)   return STATUS_ERR_OVERFLOW;
    buf[0] = '\0';

    size_t need = sizeof v * 8 + (sizeof v - 1) + 1;
    if (size < need) return STATUS_ERR_OVERFLOW;

    const unsigned char *bytes = (const unsigned char *)&v;

    size_t pos = 0;
    for (size_t i = 0; i < sizeof v; i++) {
        if (i > 0) buf[pos++] = ' ';
        for (int bit = 7; bit >= 0; bit--) {
            buf[pos++] = ((bytes[i] >> bit) & 1u) ? '1' : '0';
        }
    }
    buf[pos] = '\0';
    return STATUS_OK;
}

#endif