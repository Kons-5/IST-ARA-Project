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
        return (a.len < b.len) ? -1 : 1;
    }

    return 0;  // a == b
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

int tl_compare_stable(tl_type a, tl_type b) {
    if (tl_is_invalid(a) && tl_is_invalid(b)) {
        return -1;  // both invalid
    } else if (tl_is_invalid(a)) {
        return 1;  // b wins
    } else if (tl_is_invalid(b)) {
        return -1;  // a wins
    }

    return tl_compare(a, b);
}

int tl_compare_reduction(tl_type a, tl_type b) {
    if (tl_is_invalid(a) && tl_is_invalid(b))
        return -1;
    if (tl_is_invalid(a))
        return 1;  // b wins
    if (tl_is_invalid(b))
        return -1;  // a wins

    // Incomparability: opposite ordering in type vs len.
    // TODO: Check wether we keep both even when the AS has no customers.
    if (a.type != b.type && a.len != b.len) {
        bool a_better_type = (a.type < b.type);
        bool a_better_len  = (a.len  < b.len);

        if (a_better_type != a_better_len) {
            return 2;  // incomparable: one wins on type, the other on len
        }
    }
    return tl_compare(a, b);
}
