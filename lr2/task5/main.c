#include "over.h"

int main(int argc, char *argv[]) {
    overfprintf(stdout, "%Ro - Romanian\n", (int)181);
    overfprintf(stdout, "%Zr - Zeckendorz\n", (unsigned int)181);
    overfprintf(stdout, "%Cv - From decimal to lowercase base\n", (int)181979, (int)16);
    overfprintf(stdout, "%CV - From decimal to uppercase base\n", (int)181979, (int)16);
    overfprintf(stdout, "%to - From lowercase base to decimal\n", "2c6db", (int)16);
    overfprintf(stdout, "%TO - From uppercase base to decimal\n", "2C6DB", (int)16);
    overfprintf(stdout, "%mi - Memory dump of int32_t\n", (int32_t)129);
    overfprintf(stdout, "%mu - Memory dump of uint32_t\n", (uint32_t)129);
    overfprintf(stdout, "%md - Memory dump of double\n", (double)94124.41921);
    overfprintf(stdout, "%mf - Memory dump of float\n", (double)56385.38564);
    return 0;
}