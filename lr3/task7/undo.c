#include "undo.h"
#include "../status.h"
#include <stdlib.h>

status_t Event_init(Event_t type, Event **out) {
    if (!out)
        return STATUS_ERR_NULL_ARG;

    *out = NULL;

    if (type < SET_LAST_NAME || type > REMOVE_LIVER)
        return STATUS_ERR_INVALID;

    Event *e = calloc(1, sizeof(*e));
    if (!e)
        return STATUS_ERR_ALLOC;

    e->type = type;

    *out = e;
    return STATUS_OK;
}

void Event_free(Event *e) {
    if (!e) return;

    switch (e->type) {
        case SET_LAST_NAME:
            free(e->set_last_name.old);
            break;

        case SET_FIRST_NAME:
            free(e->set_first_name.old);
            break;

        case SET_PATRONYMIC:
            free(e->set_patronymic.old);
            break;

        case SET_BIRTH_DATE:
            free(e->set_birth_date.old);
            break;

        case SET_GENDER:
        case SET_SALARY:
        case ADD_LIVER:
            break;

        case REMOVE_LIVER:
            if (e->delete && e->target) {
                Liver_free(e->target, false);
            }
            break;
    }
    free(e);
}

status_t EventList_init(EventList **out) {
    if (!out) return STATUS_ERR_NULL_ARG;

    EventList *el = calloc(1, sizeof(*el));
    if (!el) return STATUS_ERR_ALLOC;

    *out = el;
    return STATUS_OK;
}

static void EventList_drop_oldest(EventList *el) {
    if (!el || !el->last)
        return;

    Event *cur = el->last;

    if (!cur->prev) {
        el->last = NULL;
        Event_free(cur);
    } else {
        while (cur->prev->prev) {
            cur = cur->prev;
        }

        Event *oldest = cur->prev;
        cur->prev = NULL;

        Event_free(oldest);
    }

    el->N--;
}

static void EventList_clear(EventList *el) {
    if (!el) return;

    Event *cur = el->last;

    el->last = NULL;
    el->N = 0;
    el->count = 0;

    while (cur) {
        Event *prev = cur->prev;
        cur->prev = NULL;

        Event_free(cur);
        cur = prev;
    }
}

status_t EventList_add_Event(EventList *el, Event *e) {
    if (!el || !e) return STATUS_ERR_NULL_ARG;

    if (el->count == 0) EventList_clear(el);
    
    e->prev = el->last;
    el->last = e;
    el->N++;
    el->count++;

    size_t max_undo = el->count / 2;

    while (el->N > max_undo)
        EventList_drop_oldest(el);

    return STATUS_OK;
}

status_t EventList_undo(EventList *el, Liver **root) {
    if (!el || !root)
        return STATUS_ERR_NULL_ARG;

    if (!el->last)
        return STATUS_ERR_NOT_FOUND;

    Event *e = el->last;
    status_t rc = STATUS_OK;

    switch (e->type) {
        case SET_LAST_NAME:
            rc = Liver_set_last_name(
                e->target, e->set_last_name.old
            );
            break;

        case SET_FIRST_NAME:
            rc = Liver_set_first_name(
                e->target, e->set_first_name.old
            );
            break;

        case SET_PATRONYMIC:
            rc = Liver_set_patronymic(
                e->target, e->set_patronymic.old
            );
            break;

        case SET_BIRTH_DATE:
            rc = Liver_set_birth_date(
                e->target, e->set_birth_date.old
            );
            break;

        case SET_GENDER:
            rc = Liver_set_gender(
                e->target, e->set_gender.old
            );
            break;

        case SET_SALARY:
            rc = Liver_set_salary(
                e->target, e->set_salary.old
            );
            break;

        case ADD_LIVER: {
            Liver *target = e->target;
            
            if (!target)
                return STATUS_ERR_INVALID;

            rc = LiverList_remove(root, target);
            if (rc != STATUS_OK)
                return rc;

            target->next = NULL;
            Liver_free(target, false);

            e->target = NULL;

            break;
        }

        case REMOVE_LIVER: {
            Liver *target = e->target;
            Liver *pred = e->remove_liver.predecessor;

            if (!target || !e->delete)
                return STATUS_ERR_INVALID;

            if (!pred) {
                target->next = *root;
                *root = target;
            } else {
                target->next = pred->next;
                pred->next = target;
            }

            e->delete = false;
            e->target = NULL;

            break;
        }
    }

    if (rc != STATUS_OK)
        return rc;

    el->last = e->prev;
    e->prev = NULL;

    el->N--;
    el->count = 0;

    Event_free(e);

    return STATUS_OK;
}

void EventList_free(EventList *el) {
    if (!el) return;

    EventList_clear(el);
    free(el);
}