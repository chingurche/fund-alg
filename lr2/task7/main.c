#include "../utils.h"

#include <stdio.h>
#include <stdarg.h>
#include <math.h>

typedef struct {
    double *coordv;
    int coordc;
} vector_t;

// хотел здесь сделать ввод параметров извне 
// но както это криво будет - закидывать все в параметры
/*typedef struct {
    double **elementv;
    int elementc;
} matrix_t;

typedef union {
    double p;
    matrix_t A;
} norm_params_t;

typedef enum {
    NORM_INF,
    NORM_LP,
    NORM_A
} norm_type_t;*/


typedef status_t (*normop_t)(vector_t, double *);

status_t inf_norm(vector_t v, double *out) {
    if (out == NULL)
        return STATUS_ERR_NULL_ARG;

    double max = 0;
    for (int i = 0; i < v.coordc; i++) {
        double abs = fabs(v.coordv[i]);
        if (abs > max) max = abs;
    }

    *out = max;

    return STATUS_OK;
}

status_t lp_norm(vector_t v, double *out) {
    static const double p = 2; 
    // при p = 2 то что мы обычно подразумеваем под длиной в евклидовом пространстве

    if (out == NULL)
        return STATUS_ERR_NULL_ARG;
    if (p < 1)
        return STATUS_ERR_RANGE;

    double sum = 0;
    for (int i = 0; i < v.coordc; i++) {
        sum += pow(fabs(v.coordv[i]), p);
    }

    *out = pow(sum, 1.0 / p);

    return STATUS_OK;
}

status_t a_norm(vector_t v, double *out) {
    static const double matrix[4][4] = { 
        { 4.0, 1.0, 0.0, 0.0 },
        { 1.0, 5.0, 1.0, 0.0 },
        { 0.0, 1.0, 6.0, 1.0 },
        { 0.0, 0.0, 1.0, 7.0 } 
    }; 
    // а вот тут возникает проблема с моим решением
    // матрица должна растягиваться с n(размерностью)
    // а тут фиксированная и нет проверки на полож опр

    if (out == NULL)
        return STATUS_ERR_NULL_ARG;

    double sum = 0;
    for (int i = 0; i < v.coordc; i++) {
        double y = 0;
        for (int j = 0; j < v.coordc; j++)
            y += matrix[i][j] * v.coordv[j];
        sum += y * v.coordv[i];
    }

    *out = sqrt(sum);
    
    return STATUS_OK;
}

status_t longest_vector(int n, vector_t ***out, int **out_count,
                        int vector_count, int norm_count, ...) {
    if (out == NULL || out_count == NULL)
        return STATUS_ERR_NULL_ARG;
    if (n < 1)
        return STATUS_ERR_RANGE;

    status_t rc;
    double eps = 1e-10;

    vector_t vector[vector_count];
    normop_t norm[norm_count];

    va_list ap;
    va_start(ap, norm_count);

    for (int i = 0, j = 0; i < vector_count || j < norm_count; ) {
        if (i != vector_count) vector[i++] = va_arg(ap, vector_t);
        else                     norm[j++] = va_arg(ap, normop_t);
    }

    va_end(ap);

    vector_t **tmpv = malloc(norm_count * sizeof(*tmpv));
    int *tmpc = calloc(norm_count, sizeof(*tmpc));
    if (tmpv == NULL || tmpc == NULL) {
        free(tmpv);
        free(tmpc);
        return STATUS_ERR_ALLOC;
    }
    for (int i = 0; i < norm_count; i++) {
        tmpv[i] = malloc(vector_count * sizeof(*tmpv[i]));
        if (tmpv[i] == NULL) {
            for (int k = 0; k < i; k++)
                free(tmpv[k]);
            free(tmpv);
            free(tmpc);
            return STATUS_ERR_ALLOC;
        }
    }

    for (int i = 0; i < norm_count; i++) {
        double max = 0; int count = 0;
        for (int j = 0; j < vector_count; j++) {
            double vector_norm;
            rc = norm[i](vector[j], &vector_norm);
            if (rc) {
                for (int k = 0; k < i; k++)
                    free(tmpv[k]);
                free(tmpv);
                free(tmpc);
                return rc;
            }
            if (vector_norm > max + eps) {
                max = vector_norm;
                count = 0;
                tmpv[i][count++] = vector[j];
            } else if (fabs(vector_norm - max) < eps) {
                tmpv[i][count++] = vector[j];
            }
        }
        tmpc[i] = count;
    }

    *out = tmpv;
    *out_count = tmpc;

    return rc;
}

int main(int argc, char *argv[]) {
    status_t rc;

    int vector_count = 6, norm_count = 3;
    vector_t vector[] = {
        { (double[]){ 0.9152, 1.1052, 0.4912, 4.9214 }, 4 },
        { (double[]){ 2.3847, 1.1293, 0.5571, 0.0000 }, 4 },
        { (double[]){ 2.0014, 0.2765, 0.8849, 1.7777 }, 4 },
        { (double[]){ 0.1234, 0.8765, 0.3210, 4.9214 }, 4 },
        { (double[]){ 1.2345, 2.3456, 3.4567, 4.5678 }, 4 },
        { (double[]){ 2.3456, 1.2345, 4.5678, 3.4567 }, 4 }
    };


    vector_t **longest;
    int *longest_count;
    rc = longest_vector(4, &longest, &longest_count, 
                        vector_count, norm_count,
                        vector[0], vector[1], vector[2],
                        vector[3], vector[4], vector[5],
                        inf_norm, lp_norm, a_norm);
    if (rc) {
        fprintf(stderr, "Произошла ошибка при выполнении функции.");
        for (int i = 0; i < norm_count; i++)
            free(longest[i]);
        free(longest);
        free(longest_count);
        return 1;
    }

    for (int i = 0; i < norm_count; i++) {
        printf("Функция вычисления нормы %d\n", i+1);
        for (int j = 0; j < longest_count[i]; j++) {
            for (int k = 0; k < longest[i][j].coordc; k++)
                printf("%.5f ", longest[i][j].coordv[k]);
            printf("\n"); 
        }
    }

    for (int i = 0; i < norm_count; i++)
        free(longest[i]);
    free(longest);
    free(longest_count);
    return 1;
}