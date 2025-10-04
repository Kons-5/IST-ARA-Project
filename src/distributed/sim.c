#include "../../include/distributed/sim.h"
#include "../../include/distributed/cal.h"
#include "../../include/distributed/rng.h"
#include "../../include/distributed/tab.h"
#include "../../include/distributed/tl.h"
#include <float.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

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

static void channel(Event event, double time, double d) {
    return;
}

static void schedule(Calendar *cal, Event event) {
    return;
}

static void process_event(Calendar *cal, RoutingTable **adj, RoutingTable **stl, Event event) {
    return;
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
            .from = t,
            .to = e->destination,
            .adv = stl[t]->type_length,
            .time = 0.0,
            .seq = 0u,
        };

        schedule(cal, event);
    }

    // Run the Discrete Event Simulator
    while (not_empty(cal)) {
        Event out = {0};
        if (cal_pop(cal, &out) != 0) {
            break;
        }

        process_event(cal, g_adj, stl, out);
    }

    // Debug
    print_table(g_adj, stl, "Stable Routing");

    // Clean-up
    cal_free(cal);
    clear_table(stl);
    free_cached_adj();
    return;
}

void SimuComplete(const char *path, unsigned short t, double d) {
    return;
}

void SimuCompleteAll(const char *path, double d) {
    return;
}
