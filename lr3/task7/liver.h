#ifndef LIVER_H
#define LIVER_H

#include "../status.h"
#include <stdbool.h>

typedef struct Liver Liver;

struct Liver {
    char *last_name;
    char *first_name;
    char *patronymic;
    char *birth_date;
    char gender;
    float salary;
    Liver *next;
};

status_t Liver_set_last_name(Liver* l, char *name);
status_t Liver_set_first_name(Liver* l, char *name);
status_t Liver_set_patronymic(Liver* l, char *patr);
status_t Liver_set_birth_date(Liver* l, char *date);
status_t Liver_set_gender(Liver* l, char gender);
status_t Liver_set_salary(Liver* l, float salary);
status_t Liver_init(
    char *last_name,
    char *first_name,
    char *patronymic,
    char *birth_date,
    char gender,
    float salary,
    Liver **out
);
void     Liver_free(Liver* l, bool recursive);

/* LiverList*/
typedef enum {
    BY_LAST_NAME,
    BY_FIRST_NAME,
    BY_PATRONYMIC,
    BY_BIRTH_DATE,
} FindOption;

status_t LiverList_add(Liver **root, Liver *other);
status_t LiverList_find(
    Liver *root, 
    FindOption opt, 
    char* str, 
    Liver **out
);
status_t LiverList_remove(Liver **root, Liver *other);

#endif