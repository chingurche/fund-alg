#ifndef MYSTRING_H
#define MYSTRING_H

#include "../status.h"
#include <stdbool.h>

typedef struct {
    char *data;
    int length;
} String;

status_t     newString(char *s, String **out);
void        freeString(String *str);
status_t    copyString(String *src, String *dst);
status_t copyNewString(String *src, String **out);
status_t concateString(String *a, String *b);
bool     isEmptyString(String *str);
/*                              */
int      compareString(String *a, String *b);
bool       equalString(String *a, String *b);
/*       ^^^^^^^^^^^^^          */
/* здесь без статус-кодов решил */

#endif