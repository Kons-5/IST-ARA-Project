#ifndef TAB_H
#define TAB_H

#include "../../include/sequential/tl.h"

typedef struct RoutingTable {
    int destination;            // destination
    int next_hop;               // next-hop
    tl_type type_length;        // type-length
    struct RoutingTable *next;  // next edge
} RoutingTable;

void read_table(const char *path, unsigned short t, RoutingTable **tab, RoutingTable **Et);
void clear_table(RoutingTable **table);
void print_table(RoutingTable **table);

#endif
