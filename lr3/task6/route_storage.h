#ifndef ROUTE_STORAGE_H
#define ROUTE_STORAGE_H

#include "../status.h"
#include "vector2.h"

typedef struct RouteStop RouteStop;
typedef struct Route Route;

/* Structs */
struct RouteStop {
    Vector2 coordinates;
    char *arrival_time;
    char *departed_time;
    RouteStop *next;
};

struct Route {
    char *number;
    RouteStop *stop_list;
    Route *next;
};

/* Frees */
void Route_free_list(Route *head);
void RouteStop_free_list(RouteStop *head);

/* Route */
Route *find_or_create_route(Route **head, const char *number);
void sort_route_stops(RouteStop **head_ref);

#endif