#include "../../include/distributed/sim.h"
#include "../../include/distributed/cal.h"
#include "../../include/distributed/rng.h"
#include "../../include/distributed/tab.h"
#include "../../include/distributed/tl.h"
#include "../../include/distributed/stats.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>

static struct {
    StableAllToggle fn;
} toggle;

static RoutingTable *g_adj[65536] = {0}; // file-scope cache
static RoutingTable *tab_stl[65536] = {0};
static bool g_adj_loaded = false;
static unsigned long g_seq = 0;
static unsigned short g_dst;

static void free_cached_adj(void) {
    if (!g_adj_loaded) {
        return;
    }

    clear_table(g_adj);
    memset(g_adj, 0, sizeof(g_adj));
    g_adj_loaded = false;
}

static void schedule(Calendar *cal, Event event, double d) {
    unsigned short v = event.from;
    unsigned short u = event.to;

    if (u == g_dst) {
        RoutingTable *e_vu = NULL;
        for (RoutingTable *e = g_adj[u]; e; e = e->next) {
            if (e->destination == v) {
                e_vu = e;
                break;
            }
        }

        if (e_vu && e_vu->time) {
            double prev_time = *e_vu->time;
            double arrival_time = event.time + 1.0 + rng_uniform(0.0, d);
            arrival_time = (arrival_time > prev_time + 0.01) ? arrival_time : prev_time + 0.01;

            *e_vu->time = arrival_time;
        }
        return;
    }

    RoutingTable *e_vu = NULL;
    for (RoutingTable *e = g_adj[u]; e; e = e->next) {
        if (e->destination == v) {
            e_vu = e;
            break;
        }
    }
    if (!e_vu || !e_vu->time) {
        return;
    }

    double prev_time = *e_vu->time;
    double current_time = event.time;
    double arrival_time = current_time + 1.0 + rng_uniform(0.0, d);
    arrival_time = (arrival_time > prev_time + 0.01) ? arrival_time : prev_time + 0.01;

    event.seq = g_seq++;
    event.time = arrival_time;
    *e_vu->time = arrival_time;
    cal_push(cal, event);
}

static void process_event_simple(Calendar *cal, RoutingTable **stl, Event event, double d) {
    unsigned short v = event.from;
    unsigned short u = event.to;

    RoutingTable *edge_uv = NULL;
    for (RoutingTable *e = g_adj[u]; e; e = e->next) {
        if (e->destination == v) {
            edge_uv = e;
            break;
        }
    }
    if (!edge_uv) {
        return;
    }

    tl_type proposed = tl_extend(edge_uv->type_length, event.adv);
    if (tl_compare(stl[u]->type_length, proposed) > 0) {
        stl[u]->type_length = proposed;
        stl[u]->next_hop = v;

        // Advertise to all in-neighbors of u
        for (RoutingTable *e = g_adj[u]; e; e = e->next) {
            // if (e->destination == v) {
            //     continue;
            // }

            Event next = {
                .time = event.time,
                .adv = stl[u]->type_length,
                .to = e->destination,
                .from = u,
            };

            schedule(cal, next, d);
        }
    }
}

static void process_event_complete(Calendar *cal, RoutingTable **stl, Event event, double d) {
    unsigned short v = event.from;
    unsigned short u = event.to;

    RoutingTable *edge_uv = NULL; // find the link
    for (RoutingTable *a = g_adj[u]; a; a = a->next) {
        if (a->destination == v) {
            edge_uv = a;
            break;
        }
    }

    RoutingTable *slot = NULL; // find stored tab_u[v]
    for (RoutingTable *b = tab_stl[u]; b; b = b->next) {
        if (b->destination == v) {
            slot = b;
            break;
        }
    }

    if (!edge_uv || !slot) {
        return;
    }

    // Current most prefered stable state
    tl_type old_best = stl[u]->type_length;
    unsigned short old_hop = stl[u]->next_hop;

    // Compute new entry of tab_u[v]
    tl_type entry = tl_extend(edge_uv->type_length, event.adv);

    if (u == 9910) {
        printf("\nProcessing event %lu (from %hu to %hu):\n", g_seq, v, u);
        printf("stl[u=%hu]: tl.type=%hu, tl.len=%hu\n", u, old_best.type, old_best.len);
        for (RoutingTable *p = tab_stl[u]; p; p = p->next) {
            printf("tab(u=%hu)[v=%hu]: tl.type=%hu, tl.len=%hu\n", u, p->destination, p->type_length.type, p->type_length.len);
        }
        printf("Event: tl.type=%hu, tl.len=%hu\n", event.adv.type, event.adv.len);
        printf("Link uv: tl.type=%hu, tl.len=%hu\n", edge_uv->type_length.type, edge_uv->type_length.len);
        printf("Event (ext) Link uv: tl.type=%hu, tl.len=%hu\n\n", entry.type, entry.len);
    }

    // if (tl_compare(slot->type_length, entry) == 0) {
    //     return; // No need to propagate nor update stl[u]
    // }

    // Update cached entry for neighbor v
    slot->type_length = entry; // new value of tab_u[v]

    // Scan tab_stl[u] to find the new best
    tl_type new_best = tl_invalid();
    unsigned short new_hop = 0;
    for (RoutingTable *p = tab_stl[u]; p; p = p->next) {
        tl_type c = p->type_length;
        if (tl_compare(new_best, c) >= 0) {
            new_best = c;
            new_hop = p->destination;
        }
    }

    stl[u]->type_length = new_best; // most prefered from tab_u[v]
    stl[u]->next_hop = new_hop;     // most prefered from tab_u[v]

    if (u == 9910) {
        printf("\nNEW stl[u=%hu]: tl.type=%hu, tl.len=%hu\n", u, old_best.type, old_best.len);
        for (RoutingTable *p = tab_stl[u]; p; p = p->next) {
            printf("NEW tab(u=%hu)[v=%hu]: tl.type=%hu, tl.len=%hu\n", u, p->destination, p->type_length.type, p->type_length.len);
        }
        printf("\n");
    }

    // Advertize to all in-neighbors
    if (tl_compare(old_best, new_best) != 0 || old_hop != new_hop) {
        for (RoutingTable *e = g_adj[u]; e; e = e->next) {
            // if (old_hop != new_hop && new_hop == e->destination) {
            //     continue; // don't propagate to original sender
            // }

            Event next = (Event){
                .time = event.time,
                .adv = stl[u]->type_length,
                .to = e->destination,
                .from = u,
            };

            schedule(cal, next, d);
        }
    }
}

void SimuSimple(const char *path, unsigned short t, double d) {
    // Read and populate the information from the file
    if (!g_adj_loaded) {
        load_adj(path, g_adj);
        g_adj_loaded = true;
    }

    if (!g_adj[t]) {
        return;
    }

    g_dst = t;

    Calendar *cal = cal_new();
    RoutingTable *stl[65536] = {0};
    load_state(stl, t);

    // Initialize destination with small epsilon
    stl[t]->type_length.type = TL_CUSTOMER;
    stl[t]->type_length.len = 0;

    // Broadcast initial (1,0) messages from destination
    for (RoutingTable *e = g_adj[t]; e; e = e->next) {
        Event event = (Event){
            .time = 0.0,
            .adv = stl[t]->type_length,
            .to = e->destination,
            .from = t,
        };

        schedule(cal, event, d);
    }

    // Run the Discrete Event Simulator
    while (not_empty(cal)) {
        Event out = {0};
        if (cal_pop(cal, &out) != 0) {
            break;
        }

        process_event_simple(cal, stl, out, d);
    }

    if (toggle.fn) {
        toggle.fn(g_adj, stl, t);
    } else {
        // Print stable routing and elapsed time
        printf("\nMessages exchanged: %zu\n\n", g_seq);
        print_table(g_adj, stl, "Stable Routing");
        free_cached_adj();
    }

    // Clean-up
    cal_free(cal);
    clear_table(stl);
}

void SimuComplete(const char *path, unsigned short t, double d) {
    // Read and populate the information from the file
    if (!g_adj_loaded) {
        load_adj(path, g_adj);
        g_adj_loaded = true;
    }

    if (!g_adj[t]) {
        return;
    }

    g_dst = t;

    // printf("Processing %d\n", t);
    clear_table(tab_stl);
    load_stl_tab(tab_stl, g_adj);

    Calendar *cal = cal_new();
    RoutingTable *stl[65536] = {0};
    load_state(stl, t);

    // Initialize destination with small epsilon
    stl[t]->type_length.type = TL_CUSTOMER;
    stl[t]->type_length.len = 0;

    // Broadcast initial (1,0) messages from destination
    for (RoutingTable *e = g_adj[t]; e; e = e->next) {
        Event event = (Event){
            .time = 0.0,
            .adv = stl[t]->type_length,
            .to = e->destination,
            .from = t,
        };

        // printf("Sending event from %hu to %hu...\n", t, e->destination);
        schedule(cal, event, d);
    }

    // Run the Discrete Event Simulator
    while (not_empty(cal)) {
        Event out = {0};
        if (cal_pop(cal, &out) != 0) {
            break;
        }

        // printf("seq: %llu\n", out.seq);
        // printf("Reading event from %hu to %hu...\n", out.from, out.to);
        process_event_complete(cal, stl, out, d);
    }
    // cal_print_all_seqs(cal);

    if (toggle.fn) {
        for (unsigned i = 0; i <= 65535u; i++) {
            for (RoutingTable *e = g_adj[i]; e; e = e->next) {
                if (i < e->destination && e->time) {
                    *e->time = -DBL_MAX;
                }
            }
        }
    } else {
        // Print stable routing and elapsed time
        printf("\nMessages exchanged: %zu\n\n", g_seq);
        // print_table(g_adj, stl, "Stable Routing");
        // AccStats(g_adj, stl, t);
        // PrintStats();
        free_cached_adj();
    }

    // Clean-up
    cal_free(cal);
    clear_table(stl);
    clear_table(tab_stl);
}

void SimuCompleteAll(const char *path, double d) {
    StatsReset();
    toggle.fn = AccStats;
    time_t t0 = time(NULL);

    // Iterate through all possible destinations
    for (unsigned t = 0; t <= 65535u; t++) {
        // printf("Destination %d\n", t);
        SimuComplete(path, (unsigned short) t, d);
    }
    toggle.fn = NULL;

    // Print elapsed time and stats
    double secs = difftime(time(NULL), t0);
    printf("\nElapsed: %.2f minutes (%.0f s)\n\n", secs / 60.0, secs);

    PrintStats();

    // Free cached adjacency lists
    free_cached_adj();
}
