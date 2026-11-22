#ifndef UNDO_H
#define UNDO_H

#include "../status.h"
#include "liver.h"
#include <stddef.h>

typedef enum {
    SET_LAST_NAME,
    SET_FIRST_NAME,
    SET_PATRONYMIC,
    SET_BIRTH_DATE,
    SET_GENDER,
    SET_SALARY,
    ADD_LIVER,
    REMOVE_LIVER,
    WRITE_FILE
} Event_t;

typedef struct Event Event;

struct Event {
    Event_t type;

    Liver *target;
    bool delete;

    union {
        struct { char *old; }           set_last_name;
        struct { char *old; }           set_first_name;
        struct { char *old; }           set_patronymic;
        struct { char *old; }           set_birth_date;
        struct { char old; }            set_gender;
        struct { float old; }           set_salary;
        struct { }                      add_liver;
        struct { Liver *predecessor; }  remove_liver;
    };
    Event *prev;
};

typedef struct {
    Event *last;
    size_t N;
    size_t count;
} EventList;

status_t Event_init(Event_t type, Event **out);
void Event_free(Event *e);

status_t EventList_init(EventList **out);
status_t EventList_add_Event(EventList *el, Event *e);
status_t EventList_undo(EventList *el, Liver **root);
void     EventList_free(EventList *el);


#endif