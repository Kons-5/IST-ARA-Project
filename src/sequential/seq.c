#include "../../include/sequential/seq.h"
#include "../../include/sequential/tl.h"
#include "../../include/sequential/tab.h"
#include "../../include/sequential/queue.h"
#include <stdio.h>
#include <stdlib.h>

void StableTypeLength(const char *path, unsigned short t) {
    RoutingTable *tab[65536] = {0};
    RoutingTable *Et[65536] = {0};
    read_table(path, t, tab, Et);
    print_table(tab);

    /*
     3 Q
     t
     */

    clear_table(tab);
    clear_table(Et);

    return;
}

void OptimalTypeLength(const char *path, unsigned short t) {
    return;
}

void StableAll(const char *path) {
    for (unsigned short t = 0; t < 65535u; t++) {
        StableTypeLength(path, t);
    }
    return;
}
