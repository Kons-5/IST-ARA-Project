#include "../../include/distributed/sim.h"
#include "../../include/distributed/cal.h"
#include "../../include/distributed/tab.h"
#include "../../include/distributed/rng.h"
#include "../../include/distributed/tl.h"

void SimuSimple(const char *path, unsigned short t, double d) {
    Calendar *calendar = cal_new();

    cal_free(calendar);
    return;
}

void SimuComplete(const char *path, unsigned short t, double d) {
    return;
}

void SimuCompleteAll(const char *path, double d) {
    return;
}
