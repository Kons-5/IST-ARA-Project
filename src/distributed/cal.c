#include "../../include/distributed/cal.h"

cal_type *cal_new(void) {
    return (cal_type *) 0;
}

void cal_free(cal_type *c) {
    return;
}

int cal_push(cal_type *c, event_type e) {
    return 0;
}

int cal_pop(cal_type *c, event_type *out) {
    return 0;
}
