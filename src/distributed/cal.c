#include "../../include/distributed/cal.h"
#include <stdlib.h>

#ifndef CAL_MAX_EVENTS
#    define CAL_MAX_EVENTS 65536
#endif

struct Calendar {
    Event *a;   // list of time indexed events
    size_t cap; // maximum number of elements
    size_t n;   // current number of elements
};

static int lt_event(const Event *a, const Event *b) {
    return (a->time < b->time) || (a->time == b->time && a->seq < b->seq);
}

static void swap_event(Event *x, Event *y) {
    Event temp = *x;
    *x = *y;
    *y = temp;
}

// Restore heap property bottom -> top after inserting at index i.
// Min-heap invariant: for every node j>0, a[parent(j)] <= a[j].
static void sift_up(Event *a, size_t i) {
    if (i == 0) {
        // i is the root; nothing to fix
        return;
    }

    // Parent index in 0-based heap: floor((i-1)/2)
    size_t p = (i - 1) >> 1;

    // If a[i] < a[p], the edge (p -> i) violates the min-heap property.
    // Swap and continue bubbling up from the parent.
    if (lt_event(&a[i], &a[p])) {
        swap_event(&a[i], &a[p]);
        sift_up(a, p);
    }
}

// Restore heap property top -> down after removing the root and
// moving the last element to index i (usually i == 0).
static void sift_down(Event *a, size_t n, size_t i) {
    // Children in 0-based heap
    size_t left = (i << 1) + 1; // 2*i + 1
    size_t right = left + 1;    // 2*i + 2

    // If there is no left child, i is a leaf; heap is already valid below.
    if (left >= n) {
        return;
    }

    // m = index of the smaller child (prefer the smaller one to preserve heap)
    size_t m = left;
    if (right < n && lt_event(&a[right], &a[left])) {
        m = right;
    }

    // If the smaller child is smaller than a[i], swap and keep sinking.
    if (lt_event(&a[m], &a[i])) {
        swap_event(&a[i], &a[m]);
        sift_down(a, n, m);
    }
}

static int cal_grow(Calendar *c) {
    if (!c) {
        return -1;
    }

    size_t new_cap = c->cap ? c->cap * 2 : CAL_MAX_EVENTS; // double size
    Event *na = (Event *) realloc(c->a, new_cap * sizeof(Event));
    if (!na) {
        return -1;
    }

    c->a = na;
    c->cap = new_cap;
    return 0;
}

Calendar *cal_new(void) {
    Calendar *c = (Calendar *) malloc(sizeof(Calendar));
    if (!c) {
        return NULL;
    }

    c->cap = (size_t) CAL_MAX_EVENTS;
    c->a = (Event *) malloc(c->cap * sizeof(Event));
    if (!c->a) {
        free(c);
        return NULL;
    }

    c->n = 0;
    return c;
}

void cal_free(Calendar *c) {
    if (!c) {
        return;
    }
    free(c->a);
    free(c);
}

int cal_push(Calendar *c, Event e) {
    if (!c) {
        return -1;
    }
    if (c->n == c->cap) {
        if (cal_grow(c) != 0) {
            return -1;
        }
    }

    c->a[c->n] = e;
    sift_up(c->a, c->n);
    c->n++;
    return 0;
}

int cal_pop(Calendar *c, Event *out) {
    if (!c || !out || c->n == 0) {
        return -1;
    }

    *out = c->a[0];
    c->n--;

    if (c->n) {
        c->a[0] = c->a[c->n];
        sift_down(c->a, c->n, 0);
    }
    return 0;
}

int not_empty(Calendar *c) {
    if (c->n == 0) {
        return 0;
    }
    return 1;
}
