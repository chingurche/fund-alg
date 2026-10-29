#include "over.h"

int main(int argc, char *argv[]) {
    status_t rc;

    FILE *f = fopen("test.txt", "r");
    if (!f) { fprintf(stderr, "Ошибка открытия файла"); return 1; }

    int a[2];
    overfscanf(f, "%Ro %Ro", &a[0], &a[1]);
    printf("%d - результат парса из файла\n", a[0] * a[1]);

    fclose(f);

    int b; unsigned int c;
    oversscanf("CLXXXI", "%Ro", &b);
    oversscanf("010011", "%Zr", &c);
    printf("%d - число Ro * Zr\n", b * (int)c);
    
    int d[2];
    oversscanf("69kd", "%Cv", &d[0], 36);
    oversscanf("1A2B", "%CV", &d[1], 14);
    printf("%d - число Cv * CV\n", d[0] * d[1]);

    return 0;
}