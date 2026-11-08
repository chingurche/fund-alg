#include "models.h"
#include <stdlib.h>

// здесь было очень много динамической логики
// но я отказался - пусть будет просто статика

status_t init_mytime_t(int h, int min, mytime_t *out) {
    if (out == NULL)            return STATUS_ERR_NULL_ARG;
    if   (h < 0 || h > 23)      return STATUS_ERR_RANGE;
    if (min < 0 || min > 59)    return STATUS_ERR_RANGE;

    out->hours = h;
    out->minutes = min;
    return STATUS_OK;
}

status_t init_presentation_t(
    int id, 
    mytime_t start, 
    mytime_t finish, 
    presentation_t *out) {
    if (out == NULL)    return STATUS_ERR_NULL_ARG;
    if (id <= 0)        return STATUS_ERR_RANGE;
    if  (start.hours > finish.hours ||
        (start.hours == finish.hours &&
        start.minutes >= finish.minutes))
                        return STATUS_ERR_INVALID;

    out->id;
    out->start_time = start;
    out->finish_time = finish;
    return STATUS_OK;
}

int to_minutes(mytime_t t) {
    return t.hours * 60 + t.minutes;
}

int compare_presentations(const void *a, const void *b) {
    const presentation_t *p1 = (const presentation_t *)a;
    const presentation_t *p2 = (const presentation_t *)b;
    
    int f1 = to_minutes(p1->finish_time);
    int f2 = to_minutes(p2->finish_time);
    
    if (f1 != f2) return f1 - f2;
    
    int s1 = to_minutes(p1->start_time);
    int s2 = to_minutes(p2->start_time);
    return s1 - s2;
}