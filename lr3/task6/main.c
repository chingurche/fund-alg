#include "../status.h"
#include "vector2.h"
#include "stop_storage.h"
#include "route_storage.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

static time_t parse_time_string(const char *str) {
    struct tm tm = {0};
    sscanf(str, "%d.%d.%d %d:%d:%d", 
           &tm.tm_mday, &tm.tm_mon, &tm.tm_year, 
           &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    return mktime(&tm);
}

static float calc_distance(Vector2 a, Vector2 b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}

static void print_route_info(Route *r) {
    if (!r) return;
    printf("\n=> Найдено ТС: %s\n", r->number);
    RouteStop *rs = r->stop_list;
    while (rs) {
        printf("   [%s - %s] (%.1f, %.1f)\n", 
               rs->arrival_time, rs->departed_time, 
               rs->coordinates.x, rs->coordinates.y);
        rs = rs->next;
    }
}
\
bool is_stop_file(const char *path) {
    if (path == NULL) return false;

    const char *ext = strrchr(path, '.');
    if (ext == NULL) return false;

    const char *slash = strrchr(path, '/');
    if (slash != NULL && ext < slash) {
        return false;
    }
    
    return strcmp(ext, ".stop") == 0;
}

status_t parse_stop_storage(char *path, Stop **out) {
    if (path == NULL || out == NULL)
        return STATUS_ERR_NULL_ARG;
    status_t rc;

    if (!is_stop_file(path)) return STATUS_ERR_PATH;

    FILE *f = fopen(path, "r");
    if (!f) return STATUS_ERR_OPEN;

    char line[BUFSIZ];
    if (fgets(line, sizeof(line), f) == NULL) {
        fclose(f);
        return STATUS_ERR_READ;
    }
    float x, y;
    int n = sscanf(line, "%f,%f;\n", &x, &y);
    if (n != 2) {
        fclose(f);
        return STATUS_ERR_PARSE;
    }
    Vector2 coord = { x, y };
    
    Stop *stop = NULL;
    rc = Stop_init(coord, &stop);
    if (rc) {
        fclose(f);
        return rc;
    }
    while (fgets(line, sizeof(line), f) != NULL) {
        char num[16], a_time[20], d_time[20]; int type;
        int n = sscanf(
            line, "%15[^,],%19[^,],%19[^,],%d;", 
            num, a_time, d_time, &type
        );
        if (n != 4) {
            fclose(f);
            Stop_free(stop);
            return STATUS_ERR_PARSE;
        }
        StopEvent *event = NULL;
        rc = StopEvent_init(num, a_time, d_time, (StopEvent_t)type, &event);
        if (rc) {
            fclose(f);
            Stop_free(stop);
            return rc;
        }
        rc = Stop_add_StopEvent(stop, event);
        if (rc) {
            fclose(f);
            Stop_free(stop);
            return rc;
        }
    }

    if (fclose(f) != 0) return STATUS_ERR_CLOSE;

    *out = stop;
    return STATUS_OK;
}

status_t stop_to_route_storage(Stop **stop, size_t stop_count, Route **out) {
    if (!stop || !out) return STATUS_ERR_NULL_ARG;

    Route *head_route = NULL;

    for (size_t i = 0; i < stop_count; i++) {
        if (!stop[i]) continue;

        for (size_t j = 0; j < stop[i]->count; j++) {
            StopEvent *ev = stop[i]->event[j];
            if (!ev) continue;

            Route *route = find_or_create_route(&head_route, ev->route_number);
            if (!route) {
                Route_free_list(head_route);
                return STATUS_ERR_ALLOC;
            }

            RouteStop *rs = malloc(sizeof(RouteStop));
            if (!rs) {
                Route_free_list(head_route);
                return STATUS_ERR_ALLOC;
            }

            rs->coordinates = stop[i]->coordinates;
            rs->arrival_time = strdup(ev->arrival_time);
            rs->departed_time = strdup(ev->departed_time);

            if (!rs->arrival_time || !rs->departed_time) {
                free(rs->arrival_time);
                free(rs->departed_time);
                free(rs);
                Route_free_list(head_route);
                return STATUS_ERR_ALLOC;
            }

            rs->next = route->stop_list;
            route->stop_list = rs;
        }
    }

    Route *cur_route = head_route;
    while (cur_route) {
        sort_route_stops(&(cur_route->stop_list));
        cur_route = cur_route->next;
    }

    *out = head_route;
    return STATUS_OK;
}

status_t find_max_min_route_count();
status_t find_max_min_route_length();
status_t find_max_min_route_length();

void read_console(char *buf, size_t size) {
    if (fgets(buf, size, stdin) != NULL) {
        buf[strcspn(buf, "\n")] = 0;
    } else {
        buf[0] = '\0';
    }
}

void handle_dialog(Route *root) {
    char line[BUFSIZ];
    while (true) {
        printf(
            "Поиск транспортного средства...\n"
            "(1) которое проехало больше всех маршрутов\n"
            "(2) которое проехало меньше всех маршрутов\n"
            "(3) которое проехало самый длинный путь\n"
            "(4) которое проехало самый короткий путь\n"
            "(5) с самым длинным маршрутом\n"
            "(6) с самым коротким маршрутом\n"
            "(7) с самой долгой остановкой на остановочном пункте\n"
            "(8) с самой короткой остановкой на остановочном пункте\n"
            "(9) с самым большим временем простоя (стоянки на своих остановках)\n"
            "(0) Выход\n"
        );

        read_console(line, sizeof(line));
        if (line[0] == '\0' || line[1] == '\n') continue;
        if (line[0] == '0') break;
        
        Route *best = NULL;
        float best_val = 0;
        time_t best_time = 0;

        switch(line[0]) {
            case '1':
                best_val = -1;
                for (Route *r = root; r; r = r->next) {
                    int count = 0;
                    for (RouteStop *rs = r->stop_list; rs; rs = rs->next) count++;
                    if (count > best_val) { best_val = count; best = r; }
                }
                printf("ТС проехало больше всех остановок (%d)\n", (int)best_val);
                print_route_info(best);
                break;

            case '2':
                best_val = 999999;
                for (Route *r = root; r; r = r->next) {
                    int count = 0;
                    for (RouteStop *rs = r->stop_list; rs; rs = rs->next) count++;
                    if (count < best_val && count > 0) { best_val = count; best = r; }
                }
                printf("ТС проехало меньше всех остановок (%d)\n", (int)best_val);
                print_route_info(best);
                break;

            case '3':
                best_val = -1;
                for (Route *r = root; r; r = r->next) {
                    float dist = 0;
                    for (RouteStop *rs = r->stop_list; rs && rs->next; rs = rs->next) {
                        dist += calc_distance(rs->coordinates, rs->next->coordinates);
                    }
                    if (dist > best_val) { best_val = dist; best = r; }
                }
                printf("Самый длинный путь: %.2f км\n", best_val);
                print_route_info(best);
                break;

            case '4':
                best_val = 99999999;
                for (Route *r = root; r; r = r->next) {
                    float dist = 0;
                    for (RouteStop *rs = r->stop_list; rs && rs->next; rs = rs->next) {
                        dist += calc_distance(rs->coordinates, rs->next->coordinates);
                    }
                    if (dist < best_val && dist > 0) { best_val = dist; best = r; }
                }
                printf("Самый короткий путь: %.2f км\n", best_val);
                print_route_info(best);
                break;

            case '5':
                best_time = -1;
                for (Route *r = root; r; r = r->next) {
                    if (!r->stop_list) continue;
                    time_t start = parse_time_string(r->stop_list->arrival_time);
                    
                    RouteStop *last = r->stop_list;
                    while (last->next) last = last->next;
                    time_t end = parse_time_string(last->departed_time);
                    
                    time_t duration = end - start;
                    if (duration > best_time) { best_time = duration; best = r; }
                }
                printf("Самый долгий маршрут (сек): %ld\n", (long)best_time);
                print_route_info(best);
                break;

            case '6':
                best_time = 9999999999;
                for (Route *r = root; r; r = r->next) {
                    if (!r->stop_list) continue;
                    time_t start = parse_time_string(r->stop_list->arrival_time);
                    
                    RouteStop *last = r->stop_list;
                    while (last->next) last = last->next;
                    time_t end = parse_time_string(last->departed_time);
                    
                    time_t duration = end - start;
                    if (duration < best_time && duration > 0) { best_time = duration; best = r; }
                }
                printf("Самый короткий маршрут (сек): %ld\n", (long)best_time);
                print_route_info(best);
                break;

            case '7':
                best_time = -1;
                for (Route *r = root; r; r = r->next) {
                    for (RouteStop *rs = r->stop_list; rs; rs = rs->next) {
                        time_t arr = parse_time_string(rs->arrival_time);
                        time_t dep = parse_time_string(rs->departed_time);
                        time_t diff = dep - arr;
                        if (diff > best_time) { best_time = diff; best = r; }
                    }
                }
                printf("Самая долгая стоянка на пункте (сек): %ld\n", (long)best_time);
                print_route_info(best);
                break;

            case '8':
                best_time = 9999999999;
                for (Route *r = root; r; r = r->next) {
                    for (RouteStop *rs = r->stop_list; rs; rs = rs->next) {
                        time_t arr = parse_time_string(rs->arrival_time);
                        time_t dep = parse_time_string(rs->departed_time);
                        time_t diff = dep - arr;
                        if (diff < best_time && diff >= 0) { best_time = diff; best = r; }
                    }
                }
                printf("Самая короткая стоянка на пункте (сек): %ld\n", (long)best_time);
                print_route_info(best);
                break;

            case '9':
                best_time = -1;
                for (Route *r = root; r; r = r->next) {
                    time_t total_wait = 0;
                    for (RouteStop *rs = r->stop_list; rs; rs = rs->next) {
                        time_t arr = parse_time_string(rs->arrival_time);
                        time_t dep = parse_time_string(rs->departed_time);
                        total_wait += (dep - arr);
                    }
                    if (total_wait > best_time) { best_time = total_wait; best = r; }
                }
                printf("Самое большое суммарное время простоя (сек): %ld\n", (long)best_time);
                print_route_info(best);
                break;

            default:
                printf("Такой опции нет.\n");
                break;
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    status_t rc;

    size_t path_count = argc - 1;
    char *path[path_count];
    for (size_t i = 1; i < argc; i++)
        path[i-1] = argv[i];

    size_t stop_count = path_count;
    Stop *stop[stop_count];
    for (size_t i = 0; i < path_count; i++) {
        rc = parse_stop_storage(path[i], &stop[i]);
        if (rc) {
            fprintf(stderr, "Произошла ошибка при парсинге из '%s': ", path[i]);
            switch (rc) {
                case STATUS_ERR_PATH:       fprintf(stderr, MSG_PATH); break;
                case STATUS_ERR_PARSE:      fprintf(stderr, MSG_PARSE); break;
                case STATUS_ERR_INVALID:    fprintf(stderr, MSG_INVALID); break;
                case STATUS_ERR_ALLOC:      fprintf(stderr, MSG_ALLOC); break;
                case STATUS_ERR_OPEN:       fprintf(stderr, MSG_OPEN); break;
                case STATUS_ERR_READ:       fprintf(stderr, MSG_READ); break;
                case STATUS_ERR_CLOSE:      fprintf(stderr, MSG_CLOSE); break;
                case STATUS_ERR_NULL_ARG:   fprintf(stderr, MSG_NULL_ARG); break;
                default:                    fprintf(stderr, MSG_UNKNOWN); break;
            }
            return 1;
        }
    }

    Route *root = NULL;
    rc = stop_to_route_storage(stop, stop_count, &root);
    for (size_t i = 0; i < path_count; i++) {
        Stop_free(stop[i]);
    }
    if (rc) {
        fprintf(stderr, "Произошла ошибка при транспортации хранилища: ");
        switch (rc) {
            case STATUS_ERR_ALLOC:      fprintf(stderr, MSG_ALLOC); break;
            case STATUS_ERR_NULL_ARG:   fprintf(stderr, MSG_NULL_ARG); break;
            default:                    fprintf(stderr, MSG_UNKNOWN); break;
        }
        return 1;
    }

    handle_dialog(root);

    Route_free_list(root);

    return 0;
}