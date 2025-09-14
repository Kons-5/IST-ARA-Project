#include "../../include/distributed/rng.h"
#include <stdlib.h>

void rng_seed(unsigned int seed) {
    srand(seed);
}

double rng_uniform(double a, double b) {
    if (b < a) {
        double tmp = a; a = b; b = tmp;
    }
    double u = (double)rand() / ((double)RAND_MAX + 1.0);
    return a + (b - a) * u;
}
