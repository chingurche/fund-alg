#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <errno.h>
#include <float.h>

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

status_t parse_eps(const char *s, double *out) // <stdlib.h> <errno.h> <float.h>
{
    if (s == NULL || out == NULL)
        return STATUS_ERR_NULL_ARG;

    char *end = NULL;
    errno = 0;
    double v = strtod(s, &end);

    if (end == s)         return STATUS_ERR_FORMAT;
    if (*end != '\0')     return STATUS_ERR_FORMAT;
    if (errno == ERANGE)  return STATUS_ERR_RANGE;

    if (v < DBL_EPSILON || v >= 1.0)
        return STATUS_ERR_RANGE;

    *out = v;
    return STATUS_OK;
}

#endif
