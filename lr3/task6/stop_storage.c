#include "stop_storage.h"
#include <stdio.h>
#include <string.h>

#define STARTCAP 16

bool is_valid_route_number(const char *s) {
    return strlen(s) != 0;
}

bool is_valid_datetime_format(const char *s) {
    if (!s) return false;
    if (strlen(s) != 19) return false;

    int d, m, y, h, min, sec;
    int consumed = 0;

    int n = sscanf(s, "%2d.%2d.%4d %2d:%2d:%2d%n",
                   &d, &m, &y, &h, &min, &sec, &consumed);

    if (n != 6) return false;

    if (s[2] != '.' || s[5] != '.' || s[10] != ' ' ||
        s[13] != ':' || s[16] != ':') return false;

    if (consumed != 19) return false;

    return true;
}

bool is_valid_type(StopEvent_t event) {
    switch (event) {
        case INITIAL:
        case INTERMEDIATE:
        case FINAL:
            return true;
        default:
            return false;
    }
}

status_t StopEvent_init(
    char* route_number, 
    char* arrival_time, 
    char* departed_time,
    StopEvent_t type,
    StopEvent **out)
{
    if (!route_number || !arrival_time || !departed_time || !out)
        return STATUS_ERR_NULL_ARG;
    if (!is_valid_route_number(route_number) ||
        !is_valid_datetime_format(arrival_time) ||
        !is_valid_datetime_format(departed_time) ||
        !is_valid_type(type))
        return STATUS_ERR_INVALID;

    StopEvent *event = malloc(sizeof(*event));
    if (!event) return STATUS_ERR_ALLOC;
    
    event->route_number = strdup(route_number);
    event->arrival_time = strdup(arrival_time);
    event->departed_time = strdup(departed_time);
    
    if (!event->route_number || !event->arrival_time || !event->departed_time) {
        free(event->route_number);
        free(event->arrival_time);
        free(event->departed_time);
        free(event);
        return STATUS_ERR_ALLOC;
    }

    event->type = type;

    *out = event;
    return STATUS_OK;
}

status_t Stop_init(Vector2 coord, Stop **out) {
    if (!out) return STATUS_ERR_NULL_ARG;

    Stop *stop = malloc(sizeof(*stop));
    if (!stop) return STATUS_ERR_ALLOC;

    stop->coordinates = coord;
    stop->event = malloc(STARTCAP * sizeof(stop->event));
    if (!stop->event) {
        free(stop);
        return STATUS_ERR_ALLOC;
    }
    stop->cap = STARTCAP;
    stop->count = 0;

    *out = stop;
    return STATUS_OK;
}

void StopEvent_free(StopEvent *se) {
    if (!se) return;
    free(se->route_number);
    free(se->arrival_time);
    free(se->departed_time);
    se->type = 0;
    free(se);
}

void Stop_free(Stop *s) {
    if (!s) return;
    s->coordinates = (Vector2){ 0, 0 };
    for (int i = 0; i < s->count; i++)
        StopEvent_free(s->event[i]);
    free(s->event);
    s->cap = 0;
    s->count = 0;
    free(s);
}

status_t Stop_add_StopEvent(Stop *s, StopEvent *se) {
    if (!s || !se) return STATUS_ERR_NULL_ARG;

    if (s->count + 1 > s->cap) {
        size_t newcap = s->cap * 2; 
        StopEvent **tmp = realloc(s->event, newcap * sizeof(*tmp));
        if (!tmp) return STATUS_ERR_ALLOC;
        s->event = tmp;
        s->cap = newcap;
    }

    s->event[s->count++] = se;
    return STATUS_OK;
}

bool Stop_resize_to_optimal(Stop *s) {
    StopEvent **tmp = realloc(s->event, s->count * sizeof(*tmp));
    if (!tmp) return false;
    s->event = tmp;
    s->cap = s->count;
    return true;
}