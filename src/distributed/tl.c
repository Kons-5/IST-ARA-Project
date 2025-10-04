#include "../../include/distributed/tl.h"

static tl_type tl_invalid(void) {
    tl_type r = {
        .type = TL_INVALID,
        .len = 0u,
    };
    return r;
}

bool tl_is_invalid(tl_type x) {
    return x.type == TL_INVALID;
}

int tl_compare(tl_type a, tl_type b) {
    if (tl_is_invalid(a) && tl_is_invalid(b)) {
        return -1; // both invalid
    } else if (tl_is_invalid(a)) {
        return 1; // b wins
    } else if (tl_is_invalid(b)) {
        return -1; // a wins
    }

    if (a.type != b.type) {
        return (a.type < b.type) ? -1 : 1;
    }

    if (a.len != b.len) {
        return (a.len < b.len) ? -1 : 1;
    }

    return 0; // a == b
}

tl_type tl_extend(tl_type lt, tl_type adv) {
    if (tl_is_invalid(lt) || tl_is_invalid(adv)) {
        return tl_invalid();
    }

    if (lt.type == TL_PROVIDER || adv.type == TL_CUSTOMER) {
        tl_type r = {
            .type = lt.type,
            .len = lt.len + adv.len,
        };
        return r;
    }

    return tl_invalid();
}
