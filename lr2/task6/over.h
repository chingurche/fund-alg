#ifndef OVER_H
#define OVER_H

#include "../overutils.h"

#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

typedef struct {
    FILE       *stream;
    const char *str;
    size_t      pos;
} scan_src_t; // как жаль что union нигде невозможно применить

int scan_src_getc(scan_src_t *s) {
    if (s->stream != NULL) return fgetc(s->stream);
    unsigned char c = (unsigned char)s->str[s->pos];
    if (c == '\0') return EOF;
    s->pos++;
    return c;
}

void scan_src_ungetc(scan_src_t *s, int c) {
    if (c == EOF) return;
    if (s->stream != NULL) ungetc(c, s->stream);
    else if (s->pos > 0) s->pos--;
}

status_t over_vscan(scan_src_t *src, const char *format, va_list ap) {
    status_t rc;
    const char *p = format;

    while (*p) {
        if (isspace((unsigned char)*p)) {
            int c;
            while ((c = scan_src_getc(src)) != EOF
                   && isspace((unsigned char)c));
            scan_src_ungetc(src, c);
            p++;
            continue;
        }

        if (*p != '%') {
            int c = scan_src_getc(src);
            if (c == EOF) return STATUS_ERR_PARCE;
            if (c != (unsigned char)*p) {
                scan_src_ungetc(src, c);
                return STATUS_ERR_PARCE;
            }
            p++;
            continue;
        }

        p++;

        if (*p == 'R' && *(p+1) == 'o') {
            p += 2;

            int c;
            while ((c = scan_src_getc(src)) != EOF
                   && isspace((unsigned char)c));
            scan_src_ungetc(src, c);

            char tmp[64]; size_t len = 0;
            while ((c = scan_src_getc(src)) != EOF
                   && len + 1 < sizeof tmp
                   && roman_char_to_val(c) >= 0)
                tmp[len++] = (char)c;
            scan_src_ungetc(src, c);

            if (len == 0) return STATUS_ERR_PARCE;
            tmp[len] = '\0';

            int *out = va_arg(ap, int *);
            rc = roman_to_int(tmp, out);
            if (rc) return STATUS_ERR_PARCE;
            continue;
        }

        if (*p == 'Z' && *(p+1) == 'r') {
            p += 2;

            int c;
            while ((c = scan_src_getc(src)) != EOF
                && isspace((unsigned char)c));
            scan_src_ungetc(src, c);

            char tmp[128];
            size_t len = 0;
            while ((c = scan_src_getc(src)) != EOF
                && len + 1 < sizeof tmp
                && (c == '0' || c == '1'))
                tmp[len++] = (char)c;
            scan_src_ungetc(src, c);

            if (len == 0) return STATUS_ERR_PARCE;
            tmp[len] = '\0';

            unsigned int *out = va_arg(ap, unsigned int *);
            rc = zeckendorf_to_uint(tmp, out);
            if (rc) return STATUS_ERR_PARCE;
            continue;
        }

        if (*p == 'C' && (*(p+1) == 'v' || *(p+1) == 'V')) { 
            p += 2;

            int c;
            while ((c = scan_src_getc(src)) != EOF
                && isspace((unsigned char)c));
            scan_src_ungetc(src, c);

            bool up = (*(p-1) == 'V');
            char tmp[128];
            size_t len = 0;
            while ((c = scan_src_getc(src)) != EOF
                && len + 1 < sizeof tmp
                && ((up && c >= 'A' && c <= 'Z')
                || (!up && c >= 'a' && c <= 'z')
                || (c >= '0' && c <= '9')))
                tmp[len++] = (char)c;
            scan_src_ungetc(src, c);

            if (len == 0) return STATUS_ERR_PARCE;
            tmp[len] = '\0';

            int *out = va_arg(ap, int *);
            int b    = va_arg(ap, int);
            char buf[64]; // не хочу копипастить для получаемого типа, лучше так наверное
            rc = based_to_int(tmp, b, up, buf, sizeof buf);
            if (rc) return STATUS_ERR_PARCE;
            rc = parse_int(buf, out);
            if (rc) return STATUS_ERR_PARCE;
            continue;
        }

        char spec[32]; int si = 0;
        spec[si++] = '%';
        while (*p && *p != '%') {
            if (isalpha((unsigned char)*p)
                && strchr("hlLjzt", *p) == NULL)
                break;
            if (si < (int)sizeof spec - 2) spec[si++] = *p;
            p++;
        }
        if (*p == '%') {   /* %% */
            int c = scan_src_getc(src);
            if (c != '%') {
                scan_src_ungetc(src, c);
                return STATUS_ERR_PARCE;
            }
            p++;
            continue;
        }
        if (*p == '\0') return STATUS_ERR_PARCE;
        char type = *p++;
        spec[si++] = type;
        spec[si] = '\0';

        if (type == 'c') {
            int c = scan_src_getc(src);
            if (c == EOF) return STATUS_ERR_PARCE;
            char *out = va_arg(ap, char *);
            *out = (char)c;
            continue;
        }

        int c;
        while ((c = scan_src_getc(src)) != EOF
               && isspace((unsigned char)c))
            ;
        if (c == EOF) return STATUS_ERR_PARCE;
        scan_src_ungetc(src, c);

        char tmp[256]; size_t len = 0;
        while ((c = scan_src_getc(src)) != EOF
               && len + 1 < sizeof tmp
               && !isspace((unsigned char)c))
            tmp[len++] = (char)c;
        scan_src_ungetc(src, c);

        if (len == 0) return STATUS_ERR_PARCE;
        tmp[len] = '\0';

        char spec_n[40];
        snprintf(spec_n, sizeof spec_n, "%s%%n", spec);
        int consumed = 0, matched = 0;

        switch (type) {
            case 'd': case 'i': {
                int *o = va_arg(ap, int *);
                matched = sscanf(tmp, spec_n, o, &consumed);
                break;
            }
            case 'u': case 'x': case 'X': case 'o': {
                unsigned *o = va_arg(ap, unsigned *);
                matched = sscanf(tmp, spec_n, o, &consumed);
                break;
            }
            case 's': {
                char *o = va_arg(ap, char *);
                matched = sscanf(tmp, spec_n, o, &consumed);
                break;
            }
            case 'f': case 'e': case 'g': case 'a': {
                if (strchr(spec, 'l') != NULL) {
                    double *o = va_arg(ap, double *);
                    matched = sscanf(tmp, spec_n, o, &consumed);
                } else {
                    float *o = va_arg(ap, float *);
                    matched = sscanf(tmp, spec_n, o, &consumed);
                }
                break;
            }
            default:
                return STATUS_ERR_PARCE;
        }

        for (size_t i = len; i > (size_t)consumed; i--)
            scan_src_ungetc(src, (unsigned char)tmp[i - 1]);

        if (matched != 1) return STATUS_ERR_PARCE;
    }
    return STATUS_OK;
}


status_t overfscanf(FILE *stream, const char *format, ...) {
    if (stream == NULL || format == NULL)
        return STATUS_ERR_NULL_ARG;
    va_list ap; va_start(ap, format);
    scan_src_t src = { .stream = stream, .str = NULL, .pos = 0 };
    status_t rc = over_vscan(&src, format, ap);
    va_end(ap);
    return rc;
}

status_t oversscanf(const char *str, const char *format, ...) {
    if (str == NULL || format == NULL)
        return STATUS_ERR_NULL_ARG;
    va_list ap; va_start(ap, format);
    scan_src_t src = { .stream = NULL, .str = str, .pos = 0 };
    status_t rc = over_vscan(&src, format, ap);
    va_end(ap);
    return rc;
}


#endif