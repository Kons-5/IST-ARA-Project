#ifndef RNG_H
#define RNG_H

void rng_seed(unsigned int seed);
double rng_uniform01(void);
double rng_uniform(double a, double b);
double rng_delay(double d);

#endif
