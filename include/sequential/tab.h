#ifndef TAB_H
#define TAB_H

#include "../../include/sequential/tl.h"

typedef struct RoutingTable {
    unsigned short destination; // destination
    unsigned short next_hop;    // next-hop
    tl_type type_length;        // type-length
    struct RoutingTable *next;  // next edge
} RoutingTable;

RoutingTable *add_adjancency(unsigned short u, unsigned short v, tl_type tl);

void load_state(RoutingTable **Et, unsigned short t);
void load_adj(const char *path, RoutingTable **tab);
void print_table(RoutingTable **table, char *name);
void clear_table(RoutingTable **table);

#endif
