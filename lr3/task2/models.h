#ifndef MODELS_H
#define MODELS_H

#include "../status.h"

typedef struct {
    int hours;
    int minutes;
} Time;

typedef struct {
    int id;
    Time start_time;
    Time finish_time;
} Presentation;

status_t newTime(int h, int min, Time *out);
status_t newPresentation(
    int id, 
    Time start, 
    Time finish, 
    Presentation * out);
int toMinutes(Time t);
int comparePresentation(const void *a, const void *b);

#endif