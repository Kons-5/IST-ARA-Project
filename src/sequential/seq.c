#include "../../include/sequential/seq.h"
#include "../../include/sequential/queue.h"
#include "../../include/sequential/tab.h"
#include "../../include/sequential/tl.h"
#include <stdio.h>

static bool any_nonempty(const Queue *a, const Queue *b, const Queue *c) {
    return !q_is_empty(a) || !q_is_empty(b) || !q_is_empty(c);
}

void StableTypeLength(const char *path, unsigned short t) {
    // Read and populate the information from the file
    RoutingTable *Table[65536] = {0};
    RoutingTable *E_t[65536] = {0};
    read_table(path, t, Table, E_t);
    print_table(Table, "Routing Table");
    print_table(E_t, "Stable Routing");

    // Init queues for the three attribute types
    Queue *customerQ = q_create();
    Queue *providerQ = q_create();
    Queue *peerQ = q_create();

    // Init destination with small epsilon
    E_t[t]->type_length.type = TL_CUSTOMER;
    E_t[t]->type_length.len = 0;
    q_enqueue(customerQ, t);

    // BFS with our routing algebra
    bool discovered[65536] = {false};
    while (any_nonempty(customerQ, peerQ, providerQ)) {
        unsigned short u;

        if (!q_is_empty(customerQ)) {
            q_dequeue(customerQ, &u);
        } else if (!q_is_empty(peerQ)) {
            q_dequeue(peerQ, &u);
        } else {
            q_dequeue(providerQ, &u);
        }

        if (discovered[u]) {
            continue;
        } else {
            discovered[u] = true;
        }

        for (RoutingTable *e = Table[u]; e != NULL; e = e->next) {
            unsigned short v = e->destination;

            // Relaxation
            tl_type extension = tl_extend(e->type_length, E_t[v]->type_length);
            printf("Edge: %hu %hu\n", e->type_length.type, e->type_length.len);
            printf("Stable: %hu %hu\n", E_t[v]->type_length.type, E_t[v]->type_length.len);
            printf("Extension: %hu %hu\n\n", extension.type, extension.len);

            if (tl_compare_stable(E_t[u]->type_length, extension) >= 0) {
                E_t[u]->type_length = extension;  // iff the extension is better
                E_t[u]->next_hop = v;
            }

            if (!discovered[v]) {
                switch (e->type_length.type) {
                    case TL_CUSTOMER:
                        q_enqueue(providerQ, v);  // swap attribute
                        break;

                    case TL_PEER:
                        q_enqueue(peerQ, v);  // keep attribute
                        break;

                    case TL_PROVIDER:
                        q_enqueue(customerQ, v);  // swap attribute
                        break;

                    default:
                        continue;
                }
            }
        }
    }

    print_table(E_t, "Stable Routing");

    // Clean-up
    q_destroy(customerQ);
    q_destroy(providerQ);
    q_destroy(peerQ);
    clear_table(Table);
    clear_table(E_t);
    return;
}

void OptimalTypeLength(const char *path, unsigned short t) {
    RoutingTable *Table[65536] = {0};
    RoutingTable *O_t[65536] = {0};
    read_table(path, t, Table, O_t);
    print_table(Table, "Routing Table");

    /*
    3 Q
    t
    */

    clear_table(Table);
    clear_table(O_t);
    return;
}

void StableAll(const char *path) {
    for (unsigned short t = 0; t < 65535u; t++) {
        StableTypeLength(path, t);
    }
    return;
}

void OptimalAll(const char *path) {
    for (unsigned short t = 0; t < 65535u; t++) {
        OptimalTypeLength(path, t);
    }
    return;
}
