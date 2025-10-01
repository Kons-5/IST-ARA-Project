#include "../../include/distributed/cal.h"
#include <stdlib.h>
#include <string.h>

typedef struct Calendar {
    event_type *a;
    size_t n;
    size_t cap;
} cal_type;

static int ev_lt(const event_type *x, const event_type *y) {
    if (x->time < y->time)
        return -1;
    if (x->time > y->time)
        return 1;
    if (x->seq < y->seq)
        return -1;
    if (x->seq > y->seq)
        return 1;
    return 0;
}

static int cal_reserve(cal_type *c, size_t need) {
    if (c->cap >= need)
        return 0;

    size_t new_cap = c->cap ? c->cap : 64;
    while (new_cap < need)
        new_cap <<= 1;

    event_type *na = (event_type *) realloc(c->a, new_cap * sizeof(event_type));
    if (!na)
        return -1;

    c->a = na;
    c->cap = new_cap;
    return 0;
}

static void sift_up(event_type *a, size_t i) {
    while (i > 0) {
        size_t p = (i - 1) / 2;
        if (ev_lt(&a[i], &a[p]) < 0) {
            event_type tmp = a[i];
            a[i] = a[p];
            a[p] = tmp;
            i = p;
        } else
            break;
    }
}

static void sift_down(event_type *a, size_t n, size_t i) {
    for (;;) {
        size_t l = 2 * i + 1, r = l + 1, m = i;
        if (l < n && ev_lt(&a[l], &a[m]) < 0)
            m = l;
        if (r < n && ev_lt(&a[r], &a[m]) < 0)
            m = r;
        if (m == i)
            break;

        event_type tmp = a[i];
        a[i] = a[m];
        a[m] = tmp;
        i = m;
    }
}

cal_type *cal_new(void) {
    cal_type *c = (cal_type *) calloc(1, sizeof(cal_type));
    if (!c)
        return NULL;

    c->a = NULL;
    c->n = 0;
    c->cap = 0;
    return c;
}

void cal_free(cal_type *c) {
    if (!c)
        return;

    free(c->a);
    free(c);
}

int cal_push(cal_type *c, event_type e) {
    if (!c)
        return -1;

    if (cal_reserve(c, c->n + 1) != 0)
        return -1;

    c->a[c->n] = e;
    sift_up(c->a, c->n);
    c->n += 1;
    return 0;
}

int cal_pop(cal_type *c, event_type *out) {
    if (!c || !out)
        return -1;

    if (c->n == 0)
        return -1;

    *out = c->a[0];
    c->n -= 1;
    if (c->n > 0) {
        c->a[0] = c->a[c->n];
        sift_down(c->a, c->n, 0);
    }

    return 0;
}
