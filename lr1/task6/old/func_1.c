#include <utils.h>

#include <stdbool.h>
#include <stdio.h>

typedef struct {
    double x, y;
} point_t;

static double cross(const point_t *a, const point_t *b, const point_t *c)
{
    return (b->x - a->x) * (c->y - a->y)
         - (b->y - a->y) * (c->x - a->x);
}

status_t is_convex(const point_t *pts, const size_t n, bool *out)
{
    if (pts == NULL || out == NULL)  return STATUS_ERR_NULL_ARG;
    if (n < 3)                       return STATUS_ERR_INVALID;

    *out = true;

    int sign = 0;

    for (size_t i = 0; i < n; ++i) {
        const point_t *a = &pts[i];
        const point_t *b = &pts[(i + 1) % n];
        const point_t *c = &pts[(i + 2) % n];

        double cr = cross(a, b, c);

        if (fabs(cr) <= 1e-10) // чтобы каждый раз не вводить эпсилон установлен такой
            continue;

        int s = (cr > 0.0) ? 1 : -1;

        if (sign == 0) {
            sign = s;
        } else if (s != sign) {
            *out = false;
            return STATUS_OK;
        }
    }

    return STATUS_OK;
}


int main(int argc, char *argv[]) {
    if (argc < 3 || argc % 2 != 1) {
        fprintf(stderr, "Формат запуска программы: [программа] [x₁] [y₁] [x₂] [y₂] ... [xₙ] [yₙ]\n");
		return 1;
    }

    const size_t p_count = (argc - 1) / 2;
    point_t points[p_count];
    for (size_t i = 0; i < p_count; i++) {
        status_t rc1 = parse_double(argv[1 + 2 * i], &points[i].x);
        status_t rc2 = parse_double(argv[2 + 2 * i], &points[i].y);

        if (rc1 != STATUS_OK || rc2 != STATUS_OK) {
            fprintf(stderr, "Переменные должны быть вещественного типа\n");
		    return 1;
        }
    }

    bool result;
    status_t rc = is_convex(points, p_count, &result);

    switch (rc) {
        case STATUS_OK:
            break;
        case STATUS_ERR_INVALID:
            fprintf(stderr, "Для работы программы необходимо как минимум 3 точки\n");
		    return 1;
        case STATUS_ERR_NULL_ARG:
            fprintf(stderr, "Внутренняя ошибка\n");
		    return 1;
        default:
            fprintf(stderr, "Неизвестная ошибка\n");
		    return 1;
    }

    if (result) {
        printf("Этот многоугольник является выпуклым\n");
    } else {
        printf("Этот многоугольник не является выпуклым\n");
    }
}