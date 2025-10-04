#include "../../include/distributed/sim.h"
#include "../../include/distributed/cal.h"
#include "../../include/distributed/rng.h"
#include "../../include/distributed/tab.h"
#include "../../include/distributed/tl.h"
#include <float.h>

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
    Calendar *cal = cal_new();
    RoutingTable *adj[65536] = {0};
    RoutingTable *stl[65536] = {0};
    load_adj(path, adj);
    load_state(stl, t);

    // Initialize destination with small epsilon
    stl[t]->type_length.type = TL_CUSTOMER;
    stl[t]->type_length.len = 0;

    // Broadcast initial (1,0) messages from destination
    for (RoutingTable *e = adj[t]; e; e = e->next) {
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

        process_event(cal, adj, stl, out);
    }

    // Debug
    print_table(stl, "Stable Routing");

    // Clean-up
    cal_free(cal);
    clear_table(adj);
    clear_table(stl);
    return;
}

void SimuComplete(const char *path, unsigned short t, double d) {
    return;
}

void SimuCompleteAll(const char *path, double d) {
    return;
}
