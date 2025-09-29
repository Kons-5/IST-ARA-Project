#include "../../include/sequential/queue.h"
#include <stdlib.h>

typedef struct Node {
    int value;
    struct Node *next;
} Node;

struct Queue {
    Node *head;
    Node *tail;
    unsigned short size;
};

Queue *q_create(void) {
    Queue *q = (Queue*)malloc(sizeof *q);
    if (!q) return NULL;
    q->head = q->tail = NULL;
    q->size = 0;
    return q;
}

void q_destroy(Queue *q) {
    if (!q) return;
    q_clear(q);
    free(q);
}

bool q_enqueue(Queue *q, int value) {
    if (!q) {
      return false;
    }

    Node *n = (Node*) malloc(sizeof *n);
    if (!n) {
      return false;
    }

    n->value = value;
    n->next  = NULL;

    if (q->tail) {
      q->tail->next = n;
    } else {
      q->head = n;
    }

    q->tail = n;
    q->size++;
    return true;
}

bool q_dequeue(Queue *q, int *out_value) {
    if (!q || !q->head) {
      return false;
    }

    Node *n = q->head;
    if (out_value) {
      *out_value = n->value;
    }

    q->head = n->next;
    if (!q->head) {
      q->tail = NULL;
    }
    q->size--;

    free(n);
    return true;
}

bool q_front(const Queue *q, int *out_value) {
    if (!q || !q->head) {
      return false;
    }

    if (out_value) {
      *out_value = q->head->value;
    }

    return true;
}

bool q_is_empty(const Queue *q) {
    return !q || q->size == 0;
}

void q_clear(Queue *q) {
    if (!q) {
      return;
    }

    Node *n = q->head;
    while (n) {
        Node *next = n->next;
        free(n);
        n = next;
    }

    q->head = q->tail = NULL;
    q->size = 0;
}

unsigned short q_size(const Queue *q) {
    return q ? q->size : 0u;
}
