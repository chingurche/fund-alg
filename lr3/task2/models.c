#include "models.h"
#include <stdlib.h>

// здесь было очень много динамической логики
// но я отказался - пусть будет просто статика

status_t newTime(int h, int min, Time *out) {
    if (out == NULL)            return STATUS_ERR_NULL_ARG;
    if   (h < 0 || h > 23)      return STATUS_ERR_RANGE;
    if (min < 0 || min > 59)    return STATUS_ERR_RANGE;

    out->hours = h;
    out->minutes = min;
    return STATUS_OK;
}

status_t newPresentation(
    int id, 
    Time start, 
    Time finish, 
    Presentation *out) {
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

int toMinutes(Time t) {
    return t.hours * 60 + t.minutes;
}

int comparePresentation(const void *a, const void *b) {
    const Presentation *p1 = (const Presentation *)a;
    const Presentation *p2 = (const Presentation *)b;
    
    int f1 = toMinutes(p1->finish_time);
    int f2 = toMinutes(p2->finish_time);
    
    if (f1 != f2) return f1 - f2;
    
    int s1 = toMinutes(p1->start_time);
    int s2 = toMinutes(p2->start_time);
    return s1 - s2;
}