#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

typedef struct Queue Queue;

Queue *q_create(void);
void q_clear(Queue *q);
void q_destroy(Queue *q);

bool q_enqueue(Queue *q, unsigned short value);
bool q_dequeue(Queue *q, unsigned short *out_value);
bool q_front(const Queue *q, unsigned short *out_value);

bool q_is_empty(const Queue *q);
unsigned short q_size(const Queue *q);

#endif
