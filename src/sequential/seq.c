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

        for (RoutingTable *e = g_adj[v]; e != NULL; e = e->next) {
            unsigned short u = e->destination;

            // Relaxation of u from v
            tl_type extension = tl_extend(TL_SWAP(e->type_length), E_t[v]->type_length);
            if (tl_compare_stable(E_t[u]->type_length, extension) > 0) {
                E_t[u]->type_length = extension; // iff the extension is better
                E_t[u]->next_hop = v;

                // Enqueue neighbor u in the queue from the POV of u
                // (i.e.: if u sees v as a provider place it in the provider queue)
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
                        break;
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
    time_t t0 = time(NULL);

    // Iterate through all possible destinations
    for (unsigned t = 0; t <= 65535u; t++) {
        // printf("Destination %d\n", t);
        StableTypeLength(path, (unsigned short) t);
    }
    toggle.fn = NULL;

    // Print elapsed time and stats
    double secs = difftime(time(NULL), t0);
    printf("\nElapsed: %.2f minutes (%.0f s)\n\n", secs / 60.0, secs);

    PrintStats();

    // Free cached adjacency lists
    free_cached_adj();

    return;
}

void OptimalTypeLength(const char *path, unsigned short t) {
    // Read and populate the information from the file
    if (!g_adj_loaded) {
        load_adj(path, g_adj);
        g_adj_loaded = true;
    }

    if (!g_adj[t]) {
        return;
    }

    RoutingTable *O_t[65536] = {0};
    load_state(O_t, t);

    // Init queues for the three AS types
    Queue *customerQ = q_create();
    Queue *providerQ = q_create();
    Queue *peerQ = q_create();

    // Init destination with small epsilon
    O_t[t]->type_length.type = TL_CUSTOMER;
    O_t[t]->type_length.len = 0;
    q_push(customerQ, t);

    // Inverted BFS with our routing algebra
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

        for (RoutingTable *e = g_adj[v]; e != NULL; e = e->next) {
            unsigned short u = e->destination;

            // Relaxation of u from v
            tl_type neigh = e->type_length;

            // If customer, pick the entry with smaller length
            // If peer or provider, pick the entry with smaller type
            // better type is always on the head, better length on the tail
            tl_type base = O_t[v]->type_length;
            if (O_t[v]->next && neigh.type == TL_CUSTOMER) {
                base = O_t[v]->next->type_length;
            }

            tl_type extension = tl_extend(TL_SWAP(e->type_length), base);

            tl_type h = O_t[u]->type_length;
            tl_type n = O_t[u]->next ? O_t[u]->next->type_length : h;

            if (extension.type != TL_INVALID) {
                // If extension has better type than head
                if (better_by_type(extension, h)) {
                    if (h.type != TL_INVALID && !better_by_len(extension, h)) {
                        if (!O_t[u]->next) {
                            O_t[u]->next = add_adjacency(O_t[u]->next_hop, t, O_t[u]->type_length);
                        } else {
                            O_t[u]->next->type_length = extension;
                            O_t[u]->next->next_hop = v;
                        }
                    }

                    O_t[u]->type_length = extension;
                    O_t[u]->next_hop = v;
                } else {
                    // If extension has better length than head
                    if (better_by_len(extension, h)) {
                        if (!O_t[u]->next) {
                            O_t[u]->next = add_adjacency(v, t, extension);
                        } else if (better_by_len(extension, n)) {
                            O_t[u]->next->type_length = extension;
                            O_t[u]->next->next_hop = v;
                        }
                    }
                    continue;
                }

                // Enqueue neighbor u in the queue from the POV of u
                // (i.e.: if u sees v as a provider place it in the provider queue)
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
