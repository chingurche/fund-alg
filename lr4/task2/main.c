#include "cmd_interp.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc != 1) {
        fprintf(stderr, "usage: %s < script.\n", argv[0]);
        return 1;
    }
    
    interpret(stdin);
    
    return 0;
}