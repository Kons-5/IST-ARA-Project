#include "../../include/sequential/seq.h"
#include "../../include/sequential/tl.h"
#include "../../include/sequential/tab.h"
#include "../../include/sequential/queue.h"

static bool any_nonempty(const Queue *a, const Queue *b, const Queue *c) {
    return !q_is_empty(a) || !q_is_empty(b) || !q_is_empty(c);
}

void StableTypeLength(const char *path, unsigned short t) {
    // Read and populate the information from the file
    RoutingTable *tab[65536] = {0};
    RoutingTable *Et[65536] = {0};
    read_table(path, t, tab, Et);
    print_table(tab);

    // Init queues for the three attribute types
    Queue *customerQ = q_create();
    Queue *peerQ = q_create();
    Queue *providerQ = q_create();

    // Init destination with small epsilon
    Et[t]->type_length.type = TL_CUSTOMER;
    Et[t]->type_length.len = 0;
    q_enqueue(customerQ, t);

    // BFS with our routing algebra
    bool discovered[65536] = {false};
    while (any_nonempty(customerQ, peerQ, providerQ)) {
      int u;

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

      for (RoutingTable *e = tab[u]; e; e = e->next) {
        int v = e->destination;

        // relaxation

        if (!discovered[v]) {
          switch (e->type_length.type) {
            case TL_CUSTOMER:
              q_enqueue(providerQ, v); // swap attribute
              break;

            case TL_PEER:
              q_enqueue(peerQ, v); // keep attribute
              break;

            case TL_PROVIDER:
              q_enqueue(customerQ, v); // swap attribute
              break;

            default:
              continue;
          }
        }
      }
    }

    // Clean-up
    clear_table(tab);
    clear_table(Et);
    return;
}

void OptimalTypeLength(const char *path, unsigned short t) {
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
