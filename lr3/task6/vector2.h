#ifndef VECTOR2_H
#define VECTOR2_H

#include <math.h>

typedef struct {
    float x;
    float y;
} Vector2;

static inline float get_distance_between(Vector2 a, Vector2 b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}

#endif