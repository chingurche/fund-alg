#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>

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
    STATUS_ERR_CLOSE,
    STATUS_ERR_PATH  
} status_t;

status_t parse_uint(const char *s, unsigned int *out)
{
    if (s == NULL || out == NULL)
        return STATUS_ERR_NULL_ARG;

    const char *p = s;
    if (*p == '-')
        return STATUS_ERR_FORMAT;

    char *end = NULL;
    unsigned long v = strtoul(s, &end, 10);

    if (end == s)
        return STATUS_ERR_FORMAT;

    if (*end != '\0')
        return STATUS_ERR_FORMAT;

    if (v > UINT_MAX)
        return STATUS_ERR_RANGE;

    *out = (unsigned int)v;
    return STATUS_OK;
}

#endif