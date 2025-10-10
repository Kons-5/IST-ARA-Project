#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

typedef struct Queue Queue;

Queue *q_create(void);
void q_clear(Queue *q);
void q_destroy(Queue *q);
bool q_is_empty(const Queue *q);

bool q_push(Queue *q, unsigned short value);
bool q_pop(Queue *q, unsigned short *out_value);
void q_bind_lengths(Queue *q, const unsigned short *lens, unsigned long long n);

void print(const Queue *q);

#endif
