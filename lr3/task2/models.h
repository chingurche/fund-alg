#ifndef MODELS_H
#define MODELS_H

#include "../status.h"

typedef struct {
    int hours;
    int minutes;
} mytime_t;

typedef struct {
    int id;
    mytime_t start_time;
    mytime_t finish_time;
} presentation_t;

status_t init_mytime_t(int h, int min, mytime_t *out);
status_t init_presentation_t(
    int id, 
    mytime_t start, 
    mytime_t finish, 
    presentation_t * out);
int to_minutes(mytime_t t);
int compare_presentations(const void *a, const void *b);

#endif