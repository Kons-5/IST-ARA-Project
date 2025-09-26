#include "../../include/sequential/tl.h"

static tl_type tl_invalid(void) {
    tl_type r = {
        .type = TL_INVALID,
        .len = 0u,
    };
    return r;
}

static bool tl_is_invalid(tl_type x) {
    return x.type == TL_INVALID;
}

static int tl_compare(tl_type a, tl_type b) {
    if (a.type != b.type) {
        return (a.type < b.type) ? -1 : 1;
    }

    if (a.len != b.len) {
        return (a.len < b.len ) ? -1 : 1;
    }

    return 0; // a == b
}

tl_type tl_extend(link_type lt, tl_type adv) {
    if (tl_is_invalid(adv)) {
        return tl_invalid();
    }

    if (adv.len == 65535) {
        return tl_invalid();
    }

    if (lt == TL_PROVIDER || adv.type == TL_CUSTOMER) {
        tl_type r = {
            .type = lt,
            .len = adv.len + 1u,
        };
        return r;
    }

    return tl_invalid();
}

int tl_compare_stable(tl_type a, tl_type b) {
    if (tl_is_invalid(a) && tl_is_invalid(b)) return 0;
    if (tl_is_invalid(a)) return 1;     // b wins
    if (tl_is_invalid(b)) return -1;    // a wins

    return tl_compare(a, b);
}

int tl_compare_reduction(tl_type a, tl_type b) {
    if (tl_is_invalid(a) && tl_is_invalid(b)) return 0;
    if (tl_is_invalid(a)) return 1;     // b wins
    if (tl_is_invalid(b)) return -1;    // a wins

    if (a.type == TL_CUSTOMER && b.type == TL_PEER) {
        return (a.len > b.len) ? 2 : 1; // Non comparable
    }

    return tl_compare(a, b);
}
