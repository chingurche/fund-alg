#include "route_storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Route *find_or_create_route(Route **head, const char *number) {
    Route *cur = *head;
    
    while (cur) {
        if (strcmp(cur->number, number) == 0) return cur;
        cur = cur->next;
    }

    Route *new_route = malloc(sizeof(Route));
    if (!new_route) return NULL;

    new_route->number = strdup(number);
    if (!new_route->number) {
        free(new_route);
        return NULL;
    }
    new_route->stop_list = NULL;
    new_route->next = *head;
    *head = new_route;

    return new_route;
}

void RouteStop_free_list(RouteStop *head) {
    while (head) {
        RouteStop *next = head->next;
        free(head->arrival_time);
        free(head->departed_time);
        free(head);
        head = next;
    }
}

void Route_free_list(Route *head) {
    while (head) {
        Route *next = head->next;
        RouteStop_free_list(head->stop_list);
        free(head->number);
        free(head);
        head = next;
    }
}

static int compare_route_stops(const void *a, const void *b) {
    const RouteStop *rs1 = *(const RouteStop **)a;
    const RouteStop *rs2 = *(const RouteStop **)b;

    int d1, m1, y1, h1, min1, s1;
    int d2, m2, y2, h2, min2, s2;

    sscanf(rs1->arrival_time, "%d.%d.%d %d:%d:%d", &d1, &m1, &y1, &h1, &min1, &s1);
    sscanf(rs2->arrival_time, "%d.%d.%d %d:%d:%d", &d2, &m2, &y2, &h2, &min2, &s2);

    if (y1 != y2)     return y1 - y2;
    if (m1 != m2)     return m1 - m2;
    if (d1 != d2)     return d1 - d2;
    if (h1 != h2)     return h1 - h2;
    if (min1 != min2) return min1 - min2;
    return s1 - s2;
}

void sort_route_stops(RouteStop **head_ref) {
    if (!head_ref || !(*head_ref) || !(*head_ref)->next) return;

    size_t count = 0;
    RouteStop *cur = *head_ref;
    while (cur) { count++; cur = cur->next; }

    RouteStop **array = malloc(count * sizeof(RouteStop *));
    if (!array) return; 

    cur = *head_ref;
    for (size_t i = 0; i < count; i++) {
        array[i] = cur;
        cur = cur->next;
    }

    qsort(array, count, sizeof(RouteStop *), compare_route_stops);

    for (size_t i = 0; i < count - 1; i++) {
        array[i]->next = array[i + 1];
    }
    array[count - 1]->next = NULL;
    *head_ref = array[0];

    free(array);
}
