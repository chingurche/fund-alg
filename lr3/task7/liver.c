#include "../status.h"
#include "liver.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static bool is_empty(char *s) {
    return *s == '\0';
}

static bool contains_only_alpha(char *s) {
    while(*s != '\0')
        if (!isalpha((unsigned char)*(s++)))
            return false;
    return true;
}

static bool date_is_valid(char* s) {
    if (!s) return false;
    if (strlen(s) != 10) return false;

    if (s[2] != '.' || s[5] != '.') return false;

    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;
        if (!isdigit((unsigned char)s[i])) return false;
    }

    int day   = (s[0] - '0') * 10 + (s[1] - '0');
    int month = (s[3] - '0') * 10 + (s[4] - '0');
    int year  = (s[6] - '0') * 1000 + (s[7] - '0') * 100 + 
                (s[8] - '0') * 10   + (s[9] - '0');

    if (month < 1 || month > 12) return false;
    if (year < 1) return false;

    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    bool is_leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (is_leap) {
        days_in_month[1] = 29;
    }

    if (day < 1 || day > days_in_month[month - 1]) return false;

    return true;
}

static bool gender_is_valid(char g) {
    return ((g == 'M') || (g == 'W'));
}

static bool salary_is_valid(float s) {
    return s > 0;
}

status_t Liver_set_last_name(Liver* l, char *name) {
    if (!l || !name) return STATUS_ERR_NULL_ARG;
    if (is_empty(name) || !contains_only_alpha(name))
        return STATUS_ERR_INVALID;

    char *tmp = strdup(name);
    if (!tmp) return STATUS_ERR_ALLOC;

    free(l->last_name);
    l->last_name = tmp;

    return STATUS_OK;
}

status_t Liver_set_first_name(Liver* l, char *name) {
    if (!l || !name) return STATUS_ERR_NULL_ARG;
    if (is_empty(name) || !contains_only_alpha(name))
        return STATUS_ERR_INVALID;

    char *tmp = strdup(name);
    if (!tmp) return STATUS_ERR_ALLOC;

    free(l->first_name);
    l->first_name = tmp;

    return STATUS_OK;
}

status_t Liver_set_patronymic(Liver* l, char *patr) {
    if (!l || !patr) return STATUS_ERR_NULL_ARG;
    if (!is_empty(patr) && !contains_only_alpha(patr))
        return STATUS_ERR_INVALID;

    char *tmp = strdup(patr);
    if (!tmp) return STATUS_ERR_ALLOC;

    free(l->patronymic);
    l->patronymic = tmp;

    return STATUS_OK;
}

status_t Liver_set_birth_date(Liver* l, char *date) {
    if (!l || !date) return STATUS_ERR_NULL_ARG;
    if (!date_is_valid(date)) return STATUS_ERR_INVALID;

    char *tmp = strdup(date);
    if (!tmp) return STATUS_ERR_ALLOC;

    free(l->birth_date);
    l->birth_date = tmp;

    return STATUS_OK;
}

status_t Liver_set_gender(Liver* l, char gender) {
    if (!l) return STATUS_ERR_NULL_ARG;
    if (!gender_is_valid(gender)) return STATUS_ERR_INVALID;

    l->gender = gender;

    return STATUS_OK;
}

status_t Liver_set_salary(Liver* l, float salary) {
    if (!l) return STATUS_ERR_NULL_ARG;
    if (!salary_is_valid(salary)) return STATUS_ERR_INVALID;

    l->salary = salary;

    return STATUS_OK;
}

status_t Liver_init(
    char *last_name,
    char *first_name,
    char *patronymic,
    char *birth_date,
    char gender,
    float salary,
    Liver **out) 
{
    status_t rc;
    if (!out || 
        !last_name || !first_name || 
        !patronymic || !birth_date) 
        return STATUS_ERR_NULL_ARG;
    
    Liver *liver = calloc(1, sizeof(*liver));
    if (!liver) return STATUS_ERR_ALLOC;

    rc = Liver_set_last_name(liver, last_name);
    if (rc) {
        Liver_free(liver, false);
        return rc;
    }
    
    rc = Liver_set_first_name(liver, first_name);
    if (rc) {
        Liver_free(liver, false);
        return rc;
    }

    rc = Liver_set_patronymic(liver, patronymic);
    if (rc) {
        Liver_free(liver, false);
        return rc;
    }

    rc = Liver_set_birth_date(liver, birth_date);
    if (rc) {
        Liver_free(liver, false);
        return rc;
    }

    rc = Liver_set_gender(liver, gender);
    if (rc) {
        Liver_free(liver, false);
        return rc;
    }

    rc = Liver_set_salary(liver, salary);
    if (rc) {
        Liver_free(liver, false);
        return rc;
    }

    *out = liver;

    return STATUS_OK;
}

void Liver_free(Liver* l, bool recursive) {
    if (!l) return;
    if (recursive) Liver_free(l->next, true);
    free(l->last_name);
    free(l->first_name);
    free(l->patronymic);
    free(l->birth_date);
    l->gender = 0;
    l->salary = 0;
    free(l);
}

static int Liver_compare(const void *a, const void *b) {
    const Liver *l1 = (const Liver *)a;
    const Liver *l2 = (const Liver *)b;

    int day1   = (l1->birth_date[0] - '0') * 10 + (l1->birth_date[1] - '0');
    int month1 = (l1->birth_date[3] - '0') * 10 + (l1->birth_date[4] - '0');
    int year1  = (l1->birth_date[6] - '0') * 1000 +
                 (l1->birth_date[7] - '0') * 100 +
                 (l1->birth_date[8] - '0') * 10 +
                 (l1->birth_date[9] - '0');

    int day2   = (l2->birth_date[0] - '0') * 10 + (l2->birth_date[1] - '0');
    int month2 = (l2->birth_date[3] - '0') * 10 + (l2->birth_date[4] - '0');
    int year2  = (l2->birth_date[6] - '0') * 1000 +
                 (l2->birth_date[7] - '0') * 100 +
                 (l2->birth_date[8] - '0') * 10 +
                 (l2->birth_date[9] - '0');

    if (year1 != year2) return (year1 > year2) - (year1 < year2);
    if (month1 != month2) return (month1 > month2) - (month1 < month2);
    return (day1 > day2) - (day1 < day2);
}

status_t LiverList_add(Liver **root, Liver *other) {
    if (!root || !other)
        return STATUS_ERR_NULL_ARG;

    Liver **cur = root;

    while (*cur && Liver_compare(*cur, other) <= 0) {
        cur = &(*cur)->next;
    }

    other->next = *cur;
    *cur = other;

    return STATUS_OK;
}

status_t LiverList_find(
    Liver *root, 
    FindOption opt, 
    char* str, 
    Liver **out) 
{
    if (!root || !str || !out)
        return STATUS_ERR_NULL_ARG;

    Liver *cur = root;
    while (cur) {
        bool found = false;
        switch (opt) {
            case BY_LAST_NAME:
                if (strcmp(cur->last_name, str) == 0)
                    found = true;
                break;
            case BY_FIRST_NAME:
                if (strcmp(cur->first_name, str) == 0)
                    found = true;
                break;
            case BY_PATRONYMIC:
                if (strcmp(cur->patronymic, str) == 0) 
                    found = true;
                break;
            case BY_BIRTH_DATE:
                if (strcmp(cur->birth_date, str) == 0) 
                    found = true;
                break;
            default: return STATUS_ERR_INVALID;
        }
        if (found) {
            *out = cur;
            return STATUS_OK;
        }
        cur = cur->next;
    }

    *out = NULL;

    return STATUS_ERR_NOT_FOUND;
}

/*status_t LiverList_find(
    Liver *root, 
    FindOption opt, 
    char* str, 
    Liver ***out, 
    size_t *out_size) 
{
    if (!root || !str || !out || !out_size)
        return STATUS_ERR_NULL_ARG;

    Liver *cur = root;
    int cap = 8;
    int count = 0;
    Liver **buf = malloc(cap * sizeof(*buf));
    if (!buf) return STATUS_ERR_ALLOC;
    while (cur) {
        bool found = false;
        switch (opt) {
            case BY_LAST_NAME:
                if (strcmp(cur->last_name, str))
                    found = true;
                break;
            case BY_FIRST_NAME:
                if (strcmp(cur->first_name, str))
                    found = true;
                break;
            case BY_PATRONYMIC:
                if (strcmp(cur->patronymic, str)) 
                    found = true;
                break;
            case BY_BIRTH_DATE:
                if (strcmp(cur->birth_date, str)) 
                    found = true;
                break;
        }
        if (found) {
            if (count + 1 >= cap) {
                cap *= 2;
                Liver **tmp = realloc(buf, cap * sizeof(*buf));
                if (!tmp) {
                    free(buf);
                    return STATUS_ERR_ALLOC;
                }
                buf = tmp;
            }
            buf[count++] = cur;
        }
        cur = cur->next;
    }
    
    if (count == 0) {
        free(buf);
        *out = NULL;
        *out_size = 0;
        return STATUS_ERR_NOT_FOUND;
    }

    
    Liver **tmp = realloc(count, cap * sizeof(*buf));
    if (tmp) buf = tmp;
    
    *out = buf;
    *out_size = count;

    return STATUS_OK;
}*/

status_t LiverList_remove(Liver **root, Liver *other) {
    if (!root || !other)
        return STATUS_ERR_NULL_ARG;

    Liver **cur = root;

    while (*cur && *cur != other)
        cur = &(*cur)->next;

    if (!*cur)
        return STATUS_ERR_NOT_FOUND;

    *cur = other->next;

    return STATUS_OK;
}