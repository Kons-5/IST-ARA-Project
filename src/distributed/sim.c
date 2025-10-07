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

    RoutingTable *edge_vu = NULL;
    for (RoutingTable *e = g_adj[u]; e; e = e->next) {
        if (e->destination == v) {
            edge_vu = e;
            break;
        }
    }
    if (!edge_vu) {
        return;
    }

    tl_type proposed = tl_extend(edge_vu->type_length, event.adv);
    if (tl_compare(stl[u]->type_length, proposed) >= 0) {
        stl[u]->type_length = proposed;
        stl[u]->next_hop = v;

        // Advertise to all in-neighbors of u
        for (RoutingTable *e = g_adj[u]; e; e = e->next) {
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

    RoutingTable *edge_vu = NULL;
    RoutingTable *slot = NULL;
    RoutingTable *a = g_adj[u];
    RoutingTable *b = tab_stl[u];
    while (a && b) {
        if (a->destination == v) {
            edge_vu = a;
            slot = b;
            break;
        }
        a = a->next;
        b = b->next;
    }
    if (!edge_vu || !slot) {
        return;
    }

    // compute new candidate via v
    tl_type new_cand = tl_extend(edge_vu->type_length, event.adv);
    if (!tl_is_invalid(slot->type_length) && tl_compare(slot->type_length, new_cand) == 0) {
        return;
    }

    // update cached candidate for neighbor v
    tl_type old_cand = slot->type_length;
    slot->type_length = new_cand;

    tl_type old_best = stl[u]->type_length;
    unsigned short old_hop = stl[u]->next_hop;
    int updated_is_best = (old_hop == v);

    // If updated neighbor is not the current best
    if (!updated_is_best) {
        if (tl_is_invalid(old_best) || tl_compare(new_cand, old_best) < 0) {
            stl[u]->type_length = new_cand;
            stl[u]->next_hop = v;

            // Advertise to all in-neighbors of u
            for (RoutingTable *e = g_adj[u]; e; e = e->next) {
                Event next = (Event){.time = event.time, .adv = stl[u]->type_length, .to = e->destination, .from = u};
                schedule(cal, next, d);
            }
        }
        return; // Nothing else to do
    }

    // If it improved or stayed equal, keep it and (if changed) advertise
    if (tl_compare(new_cand, old_cand) <= 0) {
        if (tl_compare(new_cand, old_best) != 0) {
            stl[u]->type_length = new_cand;
            // next_hop stays v
            for (RoutingTable *e = g_adj[u]; e; e = e->next) {
                Event next = (Event){.time = event.time, .adv = stl[u]->type_length, .to = e->destination, .from = u};
                schedule(cal, next, d);
            }
        }
        return;
    }

    // If it got worse rescan tab_stl[u] once to find the new best
    tl_type best = tl_invalid();
    unsigned short best_hop = old_hop; // replaced if we find better
    for (RoutingTable *p = tab_stl[u]; p; p = p->next) {
        tl_type c = p->type_length;
        if (!tl_is_invalid(c) && (tl_is_invalid(best) || tl_compare(c, best) < 0)) {
            best = c;
            best_hop = p->destination;
        }
    }

    if (tl_compare(best, old_best) != 0) {
        stl[u]->type_length = best;
        stl[u]->next_hop = best_hop;

        for (RoutingTable *e = g_adj[u]; e; e = e->next) {
            Event next = (Event){.time = event.time, .adv = stl[u]->type_length, .to = e->destination, .from = u};
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

    printf("Processing %d\n", t);
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

        schedule(cal, event, d);
    }

    // Run the Discrete Event Simulator
    while (not_empty(cal)) {
        Event out = {0};
        if (cal_pop(cal, &out) != 0) {
            break;
        }

        process_event_complete(cal, stl, out, d);
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
    clear_table(tab_stl);

    for (unsigned i = 0; i <= 65535u; i++) {
        for (RoutingTable *e = g_adj[i]; e; e = e->next) {
            if (i < e->destination && e->time) {
                *e->time = -DBL_MAX;
            }
        }
    }
}

void SimuCompleteAll(const char *path, double d) {
    StatsReset();
    toggle.fn = AccStats;
    time_t t0 = time(NULL);

    // Iterate through all possible destinations
    for (unsigned t = 0; t <= 65535u; t++) {
        printf("Destination %d\n", t);
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
