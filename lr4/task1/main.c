#include "macro.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    status_t rc;

    if (argc != 2) {
        fprintf(stderr, "usage: %s file\n", argv[0]);
        return 1;
    }

    rc = macro_replace(argv[1]);
    if (rc != STATUS_OK) {
        fprintf(stderr, "macro_replace(char *path): ");
        switch (rc) {
            case STATUS_ERR_INVALID:  fprintf(stderr, "invalid value"); break;
            case STATUS_ERR_PARSE:    fprintf(stderr, "invalid syntax (%s)", argv[1]); break;
            case STATUS_ERR_ALLOC:    fprintf(stderr, "allocator fault"); break;
            case STATUS_ERR_NULL_ARG: fprintf(stderr, "internal fault"); break;
            default:                  fprintf(stderr, "unknown");break;
        }
        fprintf(stderr, "\n");
        return 1;
    }

    return 0;
}