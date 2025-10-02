#include "../../include/sequential/seq.h"
#include "../../include/sequential/queue.h"
#include "../../include/sequential/tab.h"
#include "../../include/sequential/tl.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

static int TypeCount[4];
static int TotalPaths = 0;
static int LenHist[65536];

typedef void (*StableAllToggle)(RoutingTable *E_t[]);

static struct {
    StableAllToggle fn;
} toggle = {0};

static void StatsReset(void) {
    memset(TypeCount, 0, sizeof TypeCount);
    memset(LenHist, 0, sizeof LenHist);
    TotalPaths = 0;
}

static void AccStats(RoutingTable *E_t[]) {
    for (unsigned u = 0; u < 65536u; ++u) {
        if (!E_t[u]) {
            continue;
        }
        tl_type tl = E_t[u]->type_length;

        // Count type
        TypeCount[tl.type]++;

        // count length
        unsigned l = tl.len; // 0..65535
        LenHist[l]++;
    }
}

static bool any_nonempty(const Queue *a, const Queue *b, const Queue *c) {
    return !q_is_empty(a) || !q_is_empty(b) || !q_is_empty(c);
}

void StableTypeLength(const char *path, unsigned short t) {
    // Read and populate the information from the file
    RoutingTable *Table[65536] = {0};
    RoutingTable *E_t[65536] = {0};
    read_table(path, t, Table, E_t);
    // print_table(Table, "Routing Table");
    if (!Table[t]) {
        clear_table(Table);
        clear_table(E_t);
        return;
    }

    // Init queues for the three AS types
    Queue *customerQ = q_create();
    Queue *providerQ = q_create();
    Queue *peerQ = q_create();

    // Init destination with small epsilon
    E_t[t]->type_length.type = TL_CUSTOMER;
    E_t[t]->type_length.len = 0;
    q_push(customerQ, t);

    // Inverted BFS with our routing algebra
    bool discovered[65536] = {false};
    while (any_nonempty(customerQ, peerQ, providerQ)) {
        unsigned short v;

        // Dequeue the next node v from the highest-priority non-empty queue.
        // Priority encodes the best attribute: CUSTOMER < PEER < PROVIDER.
        if (!q_is_empty(customerQ)) {
            q_pop(customerQ, &v);
        } else if (!q_is_empty(peerQ)) {
            q_pop(peerQ, &v);
        } else {
            q_pop(providerQ, &v);
        }

        // Check if node v has been dequeued before
        if (discovered[v]) {
            continue;
        } else {
            discovered[v] = true;
        }

        for (RoutingTable *e = Table[v]; e != NULL; e = e->next) {
            unsigned short u = e->destination;

            // Relaxation of u from v
            tl_type extension = tl_extend(TL_SWAP(e->type_length), E_t[v]->type_length);
            if (tl_compare_stable(E_t[u]->type_length, extension) >= 0) {
                E_t[u]->type_length = extension; // iff the extension is better
                E_t[u]->next_hop = v;

                if (!discovered[u]) {
                    // Enqueue neighbor u in the queue from the POV of u
                    switch (TL_SWAP_ATTR(e->type_length.type)) {
                        case TL_CUSTOMER:
                            q_push(customerQ, u);
                            break;

                        case TL_PEER:
                            q_push(peerQ, u);
                            break;

                        case TL_PROVIDER:
                            q_push(providerQ, u);
                            break;

                        default:
                            continue;
                    }
                }
            }
        }
    }

    if (toggle.fn) {
        toggle.fn(E_t);
    } else {
        print_table(E_t, "Stable Routing");
    }

    // Clean-up
    q_destroy(customerQ);
    q_destroy(providerQ);
    q_destroy(peerQ);
    clear_table(Table);
    clear_table(E_t);
    return;
}

void StableAll(const char *path) {
    StatsReset();
    toggle.fn = AccStats;

    time_t t0 = time(NULL);

    printf("Types:\n");
    printf("  Customer: %d%%\n", TypeCount[TL_CUSTOMER]);
    printf("  Peer    : %d%%\n", TypeCount[TL_PEER]);
    printf("  Provider: %d%%\n", TypeCount[TL_PROVIDER]);

    for (unsigned short t = 0; t < 65535u; t++) {
        // printf("Destination %d\n", t);
        StableTypeLength(path, t);
    }
    toggle.fn = NULL;

    double secs = difftime(time(NULL), t0);
    printf("Elapsed: %.2f minutes (%.0f s)\n", secs / 60.0, secs);

    TotalPaths = TypeCount[1] + TypeCount[2] + TypeCount[3];
    printf("Types:\n");
    printf("  TotalPaths: %d\n", TotalPaths);
    printf("  Customer: %d\n", TypeCount[TL_CUSTOMER]);
    printf("  Peer    : %d\n", TypeCount[TL_PEER]);
    printf("  Provider: %d\n", TypeCount[TL_PROVIDER]);

    printf("Types:\n");
    printf("  Customer: %.3f%%\n", 100.0 * TypeCount[TL_CUSTOMER] / TotalPaths);
    printf("  Peer    : %.3f%%\n", 100.0 * TypeCount[TL_PEER] / TotalPaths);
    printf("  Provider: %.3f%%\n\n", 100.0 * TypeCount[TL_PROVIDER] / TotalPaths);
    return;
}

void OptimalTypeLength(const char *path, unsigned short t) {
    RoutingTable *Table[65536] = {0};
    RoutingTable *O_t[65536] = {0};
    read_table(path, t, Table, O_t);
    // print_table(Table, "Routing Table");

    // Init queues for the three AS types
    Queue *customerQ = q_create();
    Queue *providerQ = q_create();
    Queue *peerQ = q_create();

    // Init destination with small epsilon
    O_t[t]->type_length.type = TL_CUSTOMER;
    O_t[t]->type_length.len = 0;
    q_push(customerQ, t);

    // Inverted BFS with our routing algebra
    bool discovered[65536] = {false};
    while (any_nonempty(customerQ, peerQ, providerQ)) {
        unsigned short v;

        // Dequeue the next node v from the highest-priority non-empty queue.
        // Priority encodes the best attribute: CUSTOMER < PEER < PROVIDER.
        if (!q_is_empty(customerQ)) {
            q_pop(customerQ, &v);
        } else if (!q_is_empty(peerQ)) {
            q_pop(peerQ, &v);
        } else {
            q_pop(providerQ, &v);
        }

        // Check if node v has been dequeued before
        if (discovered[v]) {
            continue;
        } else {
            discovered[v] = true;
        }

        for (RoutingTable *e = Table[v]; e != NULL; e = e->next) {
            unsigned short u = e->destination;

            // Relaxation of u from v
            tl_type extension = tl_extend(TL_SWAP(e->type_length), O_t[v]->type_length);
            int inc = tl_compare_reduction(O_t[u]->type_length, extension);
            if (inc >= 0) {
                if (inc == 2) {
                    // incomparable with the first chosen path
                    if (O_t[u]->next == NULL) {
                        O_t[u]->next = add_adjancency(v, t, extension); // we have room for a second incomparable path
                    } else if (tl_compare_reduction(O_t[u]->next->type_length, extension) >= 0) {
                        O_t[u]->next->type_length = extension; // At most, a node keeps two paths,
                        O_t[u]->next->next_hop = v;            // one with better type and another with better length
                    } else {
                        continue;
                    }
                } else {
                    O_t[u]->type_length = extension;
                    O_t[u]->next_hop = v;
                }

                // Enqueue neighbor u in the queue from the POV of u
                if (!discovered[u] || inc == 2) {
                    switch (TL_SWAP_ATTR(e->type_length.type)) {
                        case TL_CUSTOMER:
                            q_push(customerQ, u);
                            break;

                        case TL_PEER:
                            q_push(peerQ, u);
                            break;

                        case TL_PROVIDER:
                            q_push(providerQ, u);
                            break;

                        default:
                            continue;
                    }
                }
            }
        }
    }

    print_table(O_t, "Optimal Routing");

    // Clean-up
    q_destroy(customerQ);
    q_destroy(providerQ);
    q_destroy(peerQ);
    clear_table(Table);
    clear_table(O_t);
    return;
}

void OptimalAll(const char *path) {
    for (unsigned short t = 0; t < 65535u; t++) {
        OptimalTypeLength(path, t);
    }
    return;
}
