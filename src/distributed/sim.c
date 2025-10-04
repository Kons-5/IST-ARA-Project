#include "../../include/distributed/sim.h"
#include "../../include/distributed/cal.h"
#include "../../include/distributed/rng.h"
#include "../../include/distributed/tab.h"
#include "../../include/distributed/tl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>

static RoutingTable *g_adj[65536] = {0}; // file-scope cache
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

static void process_event(Calendar *cal, RoutingTable **stl, Event event, double d) {
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

        process_event(cal, stl, out, d);
    }

    // Debug
    print_table(g_adj, stl, "Stable Routing");

    // Clean-up
    cal_free(cal);
    clear_table(stl);
    free_cached_adj();
}

void SimuComplete(const char *path, unsigned short t, double d) {
    return;
}

void SimuCompleteAll(const char *path, double d) {
    return;
}
