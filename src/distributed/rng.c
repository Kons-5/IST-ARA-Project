#include "../../include/distributed/rng.h"
#include <stdlib.h>

void rng_seed(unsigned int seed) {
    srand(seed);
}

double rng_uniform01(void) {
    return (double)rand() / ((double)RAND_MAX + 1.0);
}

double rng_uniform(double a, double b) {
    if (b < a) {
        double tmp = a; a = b; b = tmp;
    }
    double u = rng_uniform01();
    return a + (b - a) * u;
}

double rng_delay(double d) {
    if (d < 0.0) d = 0.0;
    return 1.0 + rng_uniform(0.0, d);
}
