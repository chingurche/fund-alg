#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <limits.h>

typedef enum {
    STATUS_OK = 0,
	STATUS_ERR_NULL_ARG,
    STATUS_ERR_ALLOC,
    STATUS_ERR_RANGE,
    STATUS_ERR_INVALID,
    STATUS_ERR_DIV_ZERO,
    STATUS_ERR_FORMAT,
    STATUS_ERR_OPEN,
    STATUS_ERR_READ,
    STATUS_ERR_WRITE,
    STATUS_ERR_CLOSE     
} status_t;

status_t parse_int(const char *s, int *out) {
    if (s == NULL || out == NULL)
        return STATUS_ERR_NULL_ARG;

    char *end = NULL;
    long v = strtol(s, &end, 10);

    if (end == s)
        return STATUS_ERR_FORMAT;

    if (*end != '\0')
        return STATUS_ERR_FORMAT;

    if (v < INT_MIN || v > INT_MAX) {
        return STATUS_ERR_RANGE;
    }

    *out = (int)v;
    return STATUS_OK;
}

status_t parse_double(const char *s, double *out) {
    if (s == NULL || out == NULL)
        return STATUS_ERR_NULL_ARG;

    char *end = NULL;
    double v = strtod(s, &end);

    if (end == s || *end != '\0')
        return STATUS_ERR_FORMAT;

    if (isnan(v) || isinf(v))
        return STATUS_ERR_RANGE;

    *out = v;
    return STATUS_OK;
}

status_t parse_in_base(const char *s, int base, size_t *out)
{
    if (s == NULL || out == NULL)     return STATUS_ERR_NULL_ARG;
    if (base < 2 || base > 36)        return STATUS_ERR_INVALID;

    size_t v = 0;
    for (const char *p = s; *p; ++p) {
        int d;
        if (*p >= '0' && *p <= '9')       d = *p - '0';
        else if (*p >= 'a' && *p <= 'z')  d = *p - 'a' + 10;
        else if (*p >= 'A' && *p <= 'Z')  d = *p - 'A' + 10;
        else                              return STATUS_ERR_FORMAT;

        if (d >= base)                    return STATUS_ERR_FORMAT;

        v = v * base + (size_t)d;
    }
    *out = v;
    return STATUS_OK;
}

#endif