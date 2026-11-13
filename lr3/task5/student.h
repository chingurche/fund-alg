#ifndef STUDENT_H
#define STUDENT_H

#include "../status.h"

#define GRADECOUNT 5

typedef struct {
    unsigned id;
    char *first_name;
    char *last_name;
    char *group;
    unsigned *grade;
} Student;

status_t newStudent(
    unsigned id,
    char *first_name,
    char *last_name,
    char *group,
    unsigned *grade,
    Student **out
);
void freeStudent(Student *s);
int compare_id(const void *a, const void *b);
int compare_first_name(const void *a, const void *b);
int compare_last_name(const void *a, const void *b);
int compare_group(const void *a, const void *b);

#endif