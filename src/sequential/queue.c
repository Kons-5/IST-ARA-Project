#include "../../include/sequential/queue.h"
#include <stdlib.h>
#include <stdio.h>

#ifndef Q_INITIAL_CAP
#    define Q_INITIAL_CAP 65536
#endif

struct Queue {
    unsigned short *a; // heap of indices
    size_t n;          // current number of elements
    size_t cap;        // capacity

    const unsigned short *lens; // lengths keyed by index
    size_t lens_n;              // number of lengths
};

static inline int less_idx(const Queue *q, unsigned short i, unsigned short j) {
    unsigned int li = (q->lens && i < q->lens_n) ? q->lens[i] : (unsigned int) -1;
    unsigned int lj = (q->lens && j < q->lens_n) ? q->lens[j] : (unsigned int) -1;
    if (li != lj) {
        return li < lj;
    }
    return i < j;
}

static inline void swap_idx(unsigned short *x, unsigned short *y) {
    unsigned short t = *x;
    *x = *y;
    *y = t;
}

// Restore heap property bottom -> top after inserting at index i.
// Min-heap invariant: for every node j>0, a[parent(j)] <= a[j].
static void sift_up(Queue *q, size_t i) {
    if (i == 0) {
        return;
    }

    // Parent index in 0-based heap: floor((i-1)/2)
    size_t p = (i - 1) >> 1;

    // Swap and continue bubbling up from the parent.
    if (less_idx(q, q->a[i], q->a[p])) {
        swap_idx(&q->a[i], &q->a[p]);
        sift_up(q, p);
    }
}

// Restore heap property top -> down after removing the root and
// moving the last element to index i (usually i == 0).
static void sift_down(Queue *q, size_t i) {
    size_t left = (i << 1) + 1; // 2*i + 1
    size_t right = left + 1;    // 2*i + 2

    // If there is no left child, i is a leaf; heap is already valid below
    if (left >= q->n)
        return;

    // m = index of the smaller child (prefer the smaller one to preserve heap)
    size_t m = left;
    if (right < q->n && less_idx(q, q->a[right], q->a[left])) {
        m = right;
    }

    // If the smaller child is smaller, swap and keep sinking
    if (less_idx(q, q->a[m], q->a[i])) {
        swap_idx(&q->a[i], &q->a[m]);
        sift_down(q, m);
    }
}

static int q_grow(Queue *q) {
    if (!q) {
        return -1;
    }

    size_t new_cap = q->cap ? 2 * q->cap : Q_INITIAL_CAP; // double size
    unsigned short *na = (unsigned short *) realloc(q->a, new_cap * sizeof(*na));
    if (!na) {
        return -1;
    }

    q->a = na;
    q->cap = new_cap;
    return 0;
}

Queue *q_create(void) {
    Queue *q = (Queue *) malloc(sizeof *q);
    if (!q) {
        return NULL;
    }

    q->a = NULL;
    q->lens = NULL;
    q->n = 0;
    q->cap = 0;
    q->lens_n = 0;
    return q;
}

void q_destroy(Queue *q) {
    if (!q) {
        return;
    }

    q_clear(q);
    free(q->a);
    free(q);
}

// bind lengths to indices (index -> length)
void q_bind_lengths(Queue *q, const unsigned short *lens, unsigned long long n) {
    if (!q) {
        return;
    }

    q->lens = lens;
    q->lens_n = n;
}

bool q_push(Queue *q, unsigned short value) {
    if (!q) {
        return false;
    }
    if (q->lens && value >= q->lens_n) {
        return false;
    }

    if (q->n == q->cap) {
        if (q_grow(q) != 0) {
            return false;
        }
    }

    q->a[q->n] = value;
    sift_up(q, q->n);
    q->n++;
    return true;
}

bool q_pop(Queue *q, unsigned short *out_value) {
    if (!q || q->n == 0) {
        return false;
    }

    if (out_value) {
        *out_value = q->a[0];
    }

    q->n--;
    if (q->n) {
        q->a[0] = q->a[q->n];
        sift_down(q, 0);
    }
    return true;
}

bool q_is_empty(const Queue *q) {
    return !q || q->n == 0;
}

void q_clear(Queue *q) {
    if (!q) {
        return;
    }
    q->n = 0;
}

void print(const Queue *q) {
    if (!q) {
        return;
    }

    printf("queue size: %llu\n", (unsigned long long) q->n);
    printf("[ ");
    for (size_t i = 0; i < q->n; ++i) {
        printf("%hu, ", q->a[i]);
    }
    printf("]\n\n");
}
