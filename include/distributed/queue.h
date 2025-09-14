#ifndef QUEUE_H
#define QUEUE_H
#include "tl.h"

typedef struct {
  double time;
  tl_type adv;
  unsigned short to;
  unsigned short from;
  unsigned long long seq;
} Event;

typedef struct EventQ EventQ;

EventQ* q_new(void);
void q_free(EventQ *q);
int q_push(EventQ *q, Event e);
int q_pop(EventQ *q, Event *out);
unsigned short q_size(const EventQ *q);

#endif
