#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <float.h>
#include <math.h>

typedef enum {
    STATUS_OK = 0,
	STATUS_ERR_RANGE,
	STATUS_ERR_NULL_ARG,
    STATUS_ERR_ALLOC,
    STATUS_ERR_INVALID,
    STATUS_ERR_OPEN,
    STATUS_ERR_READ,
    STATUS_ERR_WRITE,
    STATUS_ERR_CLOSE,
    STATUS_ERR_NOT_FOUND,
    STATUS_ERR_EMPTY,
    STATUS_ERR_DUPLICATE,
    STATUS_ERR_FORMAT
} status_t;

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

#endif