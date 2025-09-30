#ifndef TL_H
#define TL_H

#include <stdbool.h>

typedef enum { TL_CUSTOMER = 1, TL_PEER = 2, TL_PROVIDER = 3, TL_INVALID = 4 } link_type;

typedef struct {
    link_type type;
    unsigned short len;
} tl_type;

int tl_compare_stable(tl_type a, tl_type b);
int tl_compare_reduction(tl_type a, tl_type b);
tl_type tl_extend(tl_type link_type_u_to_v, tl_type adv);

#endif
