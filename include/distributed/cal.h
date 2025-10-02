#ifndef CALENDAR_H
#define CALENDAR_H
#include "tl.h"

typedef struct Event {
    double time;
    tl_type adv;
    unsigned short to;
    unsigned short from;
    unsigned long long seq;
} Event;

typedef struct Calendar Calendar;

Calendar *cal_new(void);
void cal_free(Calendar *c);
int cal_push(Calendar *c, Event e);
int cal_pop(Calendar *c, Event *out);

#endif
