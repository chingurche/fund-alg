#ifndef STOP_STORAGE_H
#define STOP_STORAGE_H

#include "../status.h"
#include "vector2.h"
#include <stdlib.h>
#include <stdbool.h>

typedef enum {
    INITIAL = 0, 
    INTERMEDIATE, 
    FINAL
} StopEvent_t;

typedef struct {
    char *route_number;
    char *arrival_time;
    char *departed_time;
    StopEvent_t type;
} StopEvent;

typedef struct {
    Vector2 coordinates;
    StopEvent **event;
    size_t cap;
    size_t count;
} Stop;

/* Init */
status_t StopEvent_init(
    char* route_number, 
    char* arrival_time, 
    char* departed_time,
    StopEvent_t type,
    StopEvent **out
);
status_t Stop_init(Vector2 coord, Stop **out);

/* Frees */
void StopEvent_free(StopEvent *se);
void Stop_free(Stop *s);

/* Stop */
status_t Stop_add_StopEvent(Stop *s, StopEvent *se);
bool     Stop_resize_to_optimal(Stop *s);


#endif