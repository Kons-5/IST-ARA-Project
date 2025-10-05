#ifndef TL_H
#define TL_H

#include <stdbool.h>

typedef enum {
    TL_CUSTOMER = 1,
    TL_PEER = 2,
    TL_PROVIDER = 3,
    TL_INVALID = 4,
} link_type;

typedef struct {
    link_type type;
    unsigned short len;
} tl_type;

int tl_compare_stable(tl_type a, tl_type b);
int better_by_type(tl_type a, tl_type b);
int better_by_len(tl_type a, tl_type b);
tl_type tl_extend(tl_type u_to_v, tl_type adv);

#define TL_SWAP_ATTR(a_) (((a_)) == TL_CUSTOMER ? TL_PROVIDER : ((a_)) == TL_PROVIDER ? TL_CUSTOMER : TL_PEER)
#define TL_SWAP(tl_) ((tl_type){TL_SWAP_ATTR((tl_).type), (tl_).len})

#endif
