#ifndef OVER_H
#define OVER_H

#include "../overutils.h"
#include "../strbldr.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

status_t over_vformat(sb_t *sb, const char *format, va_list ap) {
    status_t rc;
    const char *p = format;
    char tmp[512];

    while (*p) {
        if (*p != '%') {
            rc = sb_append(sb, p, 1);
            if (rc) return rc;
            p++;
            continue;
        }
        p++;

        if (*p == 'R' || *p == 'Z' || *p == 'C' || *p == 't' || *p == 'T' || *p == 'm') {
            char c1 = *p;
            p++;
            char c2 = *p;
            if (c2 == '\0') {
                char buf[4] = { '%', c1, '\0' };
                rc = sb_append(sb, buf, 2);
                if (rc) return rc;
                continue;
            }
            p++;

            if (c1 == 'R' && c2 == 'o') {
                int v = va_arg(ap, int);
                rc = int_to_roman(v, tmp, sizeof tmp);
                if (rc) return rc;
                size_t n = strlen(tmp);
                if (n > 0) {
                    rc = sb_append(sb, tmp, n);
                    if (rc) return rc;
                }
            } else if (c1 == 'Z' && c2 == 'r') {
                unsigned int u = va_arg(ap, unsigned int);
                rc = uint_to_zeckendorf(u, tmp, sizeof tmp);
                if (rc) return rc;
                size_t n = strlen(tmp);
                if (n > 0) {
                    rc = sb_append(sb, tmp, n);
                    if (rc) return rc;
                }
            } else if (c1 == 'C' && c2 == 'v') {
                int v = va_arg(ap, int);
                int b = va_arg(ap, int);
                rc = int_to_base(v, b, false, tmp, sizeof tmp);
                if (rc) return rc;
                size_t n = strlen(tmp);
                if (n > 0) { 
                    rc = sb_append(sb, tmp, (size_t)n); 
                    if (rc) return rc; 
                }
            } else if (c1 == 'C' && c2 == 'V') {
                int v = va_arg(ap, int);
                int b = va_arg(ap, int);
                rc = int_to_base(v, b, true, tmp, sizeof tmp);
                if (rc) return rc;
                size_t n = strlen(tmp);
                if (n > 0) { 
                    rc = sb_append(sb, tmp, (size_t)n); 
                    if (rc) return rc; 
                }
            } else if (c1 == 't' && c2 == 'o') {
                char *s = va_arg(ap, char *);
                int b = va_arg(ap, int);
                rc = based_to_int(s, b, false, tmp, sizeof tmp);
                if (rc) return rc;
                size_t n = strlen(tmp);
                if (n > 0) { 
                    rc = sb_append(sb, tmp, (size_t)n); 
                    if (rc) return rc; 
                }
            } else if (c1 == 'T' && c2 == 'O') {
                char *s = va_arg(ap, char *);
                int b = va_arg(ap, int);
                rc = based_to_int(s, b, true, tmp, sizeof tmp);
                if (rc) return rc;
                size_t n = strlen(tmp);
                if (n > 0) { 
                    rc = sb_append(sb, tmp, (size_t)n); 
                    if (rc) return rc; 
                }
            } else if (c1 == 'm' && c2 == 'i') {
                int32_t v = va_arg(ap, int32_t);
                rc = memdump_int32(v, tmp, sizeof tmp);
                if (rc) return rc;
                size_t n = strlen(tmp);
                if (n > 0) { 
                    rc = sb_append(sb, tmp, (size_t)n); 
                    if (rc) return rc; 
                }
            } else if (c1 == 'm' && c2 == 'u') {
                uint32_t v = va_arg(ap, uint32_t);
                rc = memdump_uint32(v, tmp, sizeof tmp);
                if (rc) return rc;
                size_t n = strlen(tmp);
                if (n > 0) { 
                    rc = sb_append(sb, tmp, (size_t)n); 
                    if (rc) return rc; 
                }
            } else if (c1 == 'm' && c2 == 'd') {
                double v = va_arg(ap, double);
                rc = memdump_double(v, tmp, sizeof tmp);
                if (rc) return rc;
                size_t n = strlen(tmp);
                if (n > 0) { 
                    rc = sb_append(sb, tmp, (size_t)n); 
                    if (rc) return rc; 
                }
            } else if (c1 == 'm' && c2 == 'f') {
                float v = (float)va_arg(ap, double);
                rc = memdump_float(v, tmp, sizeof tmp);
                if (rc) return rc;
                size_t n = strlen(tmp);
                if (n > 0) { 
                    rc = sb_append(sb, tmp, (size_t)n); 
                    if (rc) return rc; 
                }
            } else {
                char buf[8];
                int n = snprintf(buf, sizeof buf, "%%%c%c", c1, c2);
                if (n > 0) { 
                    rc = sb_append(sb, buf, (size_t)n); 
                    if (rc) return rc; 
                }
            }
            continue;
        }

        char spec[32];
        int si = 0;
        spec[si++] = '%';
        while (*p && !isalpha((unsigned char)*p) && *p != '%') {
            if (si < (int)sizeof spec - 2) spec[si++] = *p;
            p++;
        }
        if (*p == '%') {
            spec[si++] = '%'; spec[si] = '\0';
            rc = sb_append(sb, spec, (size_t)si); 
            if (rc) return rc;
            p++;
            continue;
        }
        if (*p == '\0') {
            spec[si] = '\0';
            rc = sb_append(sb, spec, (size_t)si); 
            if (rc) return rc;
            break;
        }
        char type = *p;
        spec[si++] = type;
        spec[si] = '\0';
        p++;

        int n = 0;
        switch (type) {
            case 'd': case 'i': {
                int v = va_arg(ap, int);
                n = snprintf(tmp, sizeof tmp, spec, v);
                break;
            }
            case 'u': case 'x': case 'X': case 'o': {
                unsigned v = va_arg(ap, unsigned);
                n = snprintf(tmp, sizeof tmp, spec, v);
                break;
            }
            case 'c': {
                int v = va_arg(ap, int);
                n = snprintf(tmp, sizeof tmp, spec, v);
                break;
            }
            case 's': {
                const char *v = va_arg(ap, const char *);
                n = snprintf(tmp, sizeof tmp, spec, v);
                break;
            }
            case 'f': case 'F': case 'g': case 'G':
            case 'e': case 'E': case 'a': case 'A': {
                double v = va_arg(ap, double);
                n = snprintf(tmp, sizeof tmp, spec, v);
                break;
            }
            default:
                n = snprintf(tmp, sizeof tmp, "%s", spec);
                break;
        }
        if (n > 0) {
            rc = sb_append(sb, tmp, (size_t)n);
            if (rc) return rc;
        }
    }
    return STATUS_OK;
}

status_t overfprintf(FILE *stream, const char *format, ...) {
    if (stream == NULL || format == NULL) return STATUS_ERR_NULL_ARG;

    va_list ap;
    va_start(ap, format);
    sb_t sb;
    status_t rc = sb_init(&sb);
    if (rc != STATUS_OK) { va_end(ap); return rc; }
    rc = over_vformat(&sb, format, ap);
    va_end(ap);

    if (rc == STATUS_OK && sb.len > 0) {
        if (fwrite(sb.data, 1, sb.len, stream) != sb.len)
            rc = STATUS_ERR_WRITE;
    }
    sb_free(&sb);
    return rc;
}

status_t oversprintf(char *buf, size_t cap, const char *format, ...) {
    if (buf == NULL || cap == 0 || format == NULL) return STATUS_ERR_NULL_ARG;

    va_list ap;
    va_start(ap, format);
    sb_t sb;
    status_t rc = sb_init(&sb);
    if (rc != STATUS_OK) { va_end(ap); return rc; }
    rc = over_vformat(&sb, format, ap);
    va_end(ap);

    if (rc == STATUS_OK) {
        size_t copy = (sb.len < cap - 1) ? sb.len : cap - 1;
        memcpy(buf, sb.data, copy);
        buf[copy] = '\0';
    }
    sb_free(&sb);
    return rc;
}


#endif