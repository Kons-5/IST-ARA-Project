#include "../../include/distributed/cal.h"
#include <stdlib.h>

#ifndef CAL_MAX_EVENTS
#    define CAL_MAX_EVENTS 65536
#endif

struct Calendar {
    event_type *a;
    size_t cap;
    size_t n;
};

static int lt_event(const event_type *a, const event_type *b) {
    return (a->time < b->time) || (a->time == b->time && a->seq < b->seq);
}

static void swap_event(event_type *x, event_type *y) {
    event_type temp = *x;
    *x = *y;
    *y = temp;
}

static void sift_up(event_type *a, size_t i) {
    if (i == 0) {
        return;
    }

    size_t p = (i - 1) >> 1;
    if (lt_event(&a[i], &a[p])) {
        swap_event(&a[i], &a[p]);
        sift_up(a, p);
    }
}

static void sift_down(event_type *a, size_t n, size_t i) {
    size_t left = (i << 1) + 1;
    size_t right = left + 1;

    if (left >= n) {
        return;
    }

    size_t m = left;
    if (right < n && lt_event(&a[right], &a[left])) {
        m = right;
    }

    if (lt_event(&a[m], &a[i])) {
        swap_event(&a[i], &a[m]);
        sift_down(a, n, m);
    }
}

cal_type *cal_new(void) {
    cal_type *c = (cal_type *) calloc(1, sizeof(cal_type));
    if (!c) {
        return NULL;
    }

    c->cap = (size_t) CAL_MAX_EVENTS;
    c->a = (event_type *) malloc(c->cap * sizeof(event_type));
    if (!c->a) {
        free(c);
        return NULL;
    }

    c->n = 0;
    return c;
}

void cal_free(cal_type *c) {
    if (!c) {
        return;
    }
    free(c->a);
    free(c);
}

int cal_push(cal_type *c, event_type e) {
    if (!c) {
        return -1;
    }
    if (c->n == c->cap) {
        return -1;  // heap is full
    }

    c->a[c->n] = e;
    sift_up(c->a, c->n);
    c->n++;
    return 0;
}

int cal_pop(cal_type *c, event_type *out) {
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
