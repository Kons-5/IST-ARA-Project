#include "../../include/sequential/seq.h"
#include "../../include/sequential/queue.h"
#include "../../include/sequential/stats.h"
#include "../../include/sequential/tab.h"
#include "../../include/sequential/tl.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

static struct {
    StableAllToggle fn;
} toggle;

static RoutingTable *g_adj[65536] = {0}; // file-scope cache
static bool g_adj_loaded = false;

static void free_cached_adj(void) {
    if (!g_adj_loaded) {
        return;
    }

    clear_table(g_adj);
    memset(g_adj, 0, sizeof(g_adj));
    g_adj_loaded = false;
}

static inline bool any_nonempty(const Queue *a, const Queue *b, const Queue *c) {
    return !q_is_empty(a) || !q_is_empty(b) || !q_is_empty(c);
}

void StableTypeLength(const char *path, unsigned short t) {
    // Read and populate the information from the file
    if (!g_adj_loaded) {
        load_adj(path, g_adj);
        g_adj_loaded = true;
    }

    if (!g_adj[t]) {
        return;
    }

    RoutingTable *E_t[65536] = {0};
    load_state(E_t, t);

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

        for (RoutingTable *e = g_adj[v]; e != NULL; e = e->next) {
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
        toggle.fn(g_adj, E_t, t);
    } else {
        // Print stable routing and elapsed time
        print_table(g_adj, E_t, "Stable Routing");
        free_cached_adj();
    }

    // Clean-up
    q_destroy(customerQ);
    q_destroy(providerQ);
    q_destroy(peerQ);
    clear_table(E_t);
    return;
}

void StableAll(const char *path) {
    StatsReset();
    toggle.fn = AccStats;

    // Iterate through all possible destinations
    time_t t0 = time(NULL);

    for (unsigned t = 0; t <= 65535u; t++) {
        // printf("Destination %d\n", t);
        StableTypeLength(path, (unsigned short) t);
    }
    toggle.fn = NULL;

    // Print elapsed time and stats
    double secs = difftime(time(NULL), t0);
    printf("\nElapsed: %.2f minutes (%.0f s)\n\n", secs / 60.0, secs);

    PrintStats(); // results

    // Free cached adjacency lists
    free_cached_adj();

    return;
}

void OptimalTypeLength(const char *path, unsigned short t) {
    // Read and populate the information from the file
    RoutingTable *O_t[65536] = {0};
    load_state(O_t, t);

    if (!g_adj_loaded) {
        load_adj(path, g_adj);
        g_adj_loaded = true;
    }

    if (!g_adj[t]) {
        clear_table(O_t);
        return;
    }

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

        for (RoutingTable *e = g_adj[v]; e != NULL; e = e->next) {
            unsigned short u = e->destination;

            // Relaxation of u from v
            // if u is not a customer then it prefers the path with best type, else, with the best length
            tl_type adv = e->type_length;

            // If customer pick the entry with smaller length
            // If peer or provider pick the entry with smaller type
            tl_type base = O_t[v]->type_length;

            if (O_t[v]->next) {
                tl_type head = O_t[v]->type_length;
                tl_type alt = O_t[v]->next->type_length;

                if (adv.type == TL_CUSTOMER) {
                    base = (alt.len < head.len) ? alt : head;
                } else {
                    if (alt.type < head.type)
                        base = alt;
                }
            }

            tl_type extension = tl_extend(TL_SWAP(e->type_length), base);
            int inc1 = tl_compare_reduction(O_t[u]->type_length, extension);
            int inc2 = 0;
            if (O_t[u]->next) {
                inc2 = tl_compare_reduction(O_t[u]->next->type_length, extension);
            }

            if (inc1 >= 0 || inc2 >= 0) {
                if (inc1 == 1 && O_t[u]->next == NULL) {
                    O_t[u]->type_length = extension;
                    O_t[u]->next_hop = v;
                } else if (inc1 == 2 && O_t[u]->next == NULL) {
                    O_t[u]->next = add_adjancency(v, t, extension);
                } else if (inc1 == 1 && inc2 == 1) {
                    if (O_t[u]->type_length.len < O_t[u]->next->type_length.len) {
                        O_t[u]->next->type_length = extension;
                        O_t[u]->next->next_hop = v;
                    } else {
                        O_t[u]->type_length = extension;
                        O_t[u]->next_hop = v;
                    }
                } else if (inc1 == 2 && inc2 >= 0) {
                    O_t[u]->next->type_length = extension;
                    O_t[u]->next->next_hop = v;
                } else if (inc1 >= 0 && inc2 == 2) {
                    O_t[u]->type_length = extension;
                    O_t[u]->next_hop = v;
                } else if (inc1 == 2 && inc2 == 2) {
                    printf("Afinal pode\n");
                }

                // Enqueue neighbor u in the queue from the POV of u
                if (!discovered[u]) {
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
        toggle.fn(g_adj, O_t, t);
    } else {
        // Print stable routing and elapsed time
        print_table(g_adj, O_t, "Optimal Routing");
        free_cached_adj();
    }

    // Clean-up
    q_destroy(customerQ);
    q_destroy(providerQ);
    q_destroy(peerQ);
    clear_table(O_t);
    free_cached_adj();
    return;
}

void OptimalAll(const char *path) {
    StatsReset();
    toggle.fn = AccStats;

    // Iterate through all possible destinations
    time_t t0 = time(NULL);

    for (unsigned t = 0; t <= 65535u; t++) {
        OptimalTypeLength(path, (unsigned short) t);
    }
    toggle.fn = NULL;

    // Print elapsed time and stats
    double secs = difftime(time(NULL), t0);
    printf("\nElapsed: %.2f minutes (%.0f s)\n\n", secs / 60.0, secs);

    PrintStats(); // results

    // Free cached adjacency lists
    free_cached_adj();

    return;
}
