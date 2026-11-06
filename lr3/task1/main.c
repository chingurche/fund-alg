#include "mytor.h"

int main(int argc, char *argv[]) {
    status_t rc;

    char buf[64];
    
    rc = inttor(-131, 1, buf, sizeof(buf));
    CHECK(rc); printf("%s\n", buf);

    rc = inttor(21031231, 5, buf, sizeof(buf));
    CHECK(rc); printf("%s\n", buf);

    return 0;
 }