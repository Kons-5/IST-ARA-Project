#ifndef CALENDAR_H
#define CALENDAR_H
#include "tl.h"

typedef struct Event {
  double time;
  tl_type adv;
  unsigned short to;
  unsigned short from;
  unsigned long long seq;
} event_type;

typedef struct Calendar cal_type;

cal_type* cal_new(void);
void cal_free(cal_type *c);
int cal_push(cal_type *c, event_type e);
int cal_pop (cal_type *c, event_type *out);

#endif
