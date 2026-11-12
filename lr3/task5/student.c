#include "student.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

bool is_empty(char *s) {
    return *s == '\0';
}

bool is_all_alpha(char *s) {
    while(*s != '\0')
        if (!isalpha(*(s++)))
            return false;
    return true;
}

status_t newStudent(
    unsigned id,
    char *first_name,
    char *last_name,
    char *group,
    unsigned *grade,
    Student **out) 
{
    if (!first_name || !last_name ||
        !group || !grade || !out)
        return STATUS_ERR_NULL_ARG;
    if (is_empty(first_name) ||
        is_empty(first_name) ||
        is_empty(group) ||
        !is_all_alpha(first_name) ||
        !is_all_alpha(last_name))
        return STATUS_ERR_INVALID;

    Student *s = malloc(sizeof *s);
    if (!s) return STATUS_ERR_ALLOC;

    s->id = id;
    s->first_name = strdup(first_name);
    s->last_name  = strdup(last_name);
    s->group      = strdup(group);
    if (!s->first_name || !s->last_name || !s->group) {
        free(s->first_name);
        free(s->last_name);
        free(s->group);
        return STATUS_ERR_ALLOC;
    }
    s->grade = grade;

    *out = s;
    return STATUS_OK;
}

void freeStudent(Student *s) {
    if (!s) return;
    s->id = 0;
    free(s->first_name);
    free(s->last_name);
    free(s->group);
    free(s->grade);
    free(s);
}

int compare_id(const void *a, const void *b) {
    const Student *s1 = *(const Student **)a;
    const Student *s2 = *(const Student **)b;
    return s1->id - s2->id;
}

int compare_first_name(const void *a, const void *b) {
    const Student *s1 = *(const Student **)a;
    const Student *s2 = *(const Student **)b;
    return strcmp(s1->first_name, s2->first_name);
}

int compare_last_name(const void *a, const void *b) {
    const Student *s1 = *(const Student **)a;
    const Student *s2 = *(const Student **)b;
    return strcmp(s1->last_name, s2->last_name);
}

int compare_group(const void *a, const void *b) {
    const Student *s1 = *(const Student **)a;
    const Student *s2 = *(const Student **)b;
    return strcmp(s1->group, s2->group);
}