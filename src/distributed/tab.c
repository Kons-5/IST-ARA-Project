#include "../../include/distributed/tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

RoutingTable *add_adjacency(unsigned short u, unsigned short v, tl_type tl) {
    RoutingTable *entry = (RoutingTable *) malloc(sizeof(RoutingTable));
    if (entry == NULL) {
        fprintf(stderr, "Error: out of memory for edge %hu->%hu\n", u, v);
        return NULL;
    }

    entry->next_hop = u;
    entry->destination = v;
    entry->type_length = (tl_type){
        .type = tl.type,
        .len = tl.len,
    };
    entry->next = NULL;
    entry->time = NULL;

    return entry;
}

void load_state(RoutingTable **Et, unsigned short t) {
    for (unsigned u = 0; u <= 65535u; u++) {
        tl_type tmp = (tl_type){
            .type = TL_INVALID,
            .len = 0u,
        };

        RoutingTable *entry = add_adjacency((unsigned short) u, t, tmp);
        if (entry == NULL) {
            exit(1);
        }

        Et[u] = entry;
    }
}

void load_adj(const char *path, RoutingTable **tab) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        exit(1);
    }

    unsigned short u, v, raw_type;
    while (fscanf(fp, "%hu %hu %hu\n", &u, &v, &raw_type) == 3) {
        tl_type tmp = (tl_type){
            .type = (link_type) raw_type,
            .len = 1u,
        };

        RoutingTable *tab_entry = add_adjacency(v, v, tmp);
        if (tab_entry == NULL) {
            exit(1);
        }

        if (tab[u] == NULL) {
            // If the routing table is empty
            tab[u] = tab_entry;
        } else {
            // Insert new neighbor at the end
            RoutingTable *tmp = tab[u];
            RoutingTable *prev = NULL;
            while (tmp != NULL) {
                prev = tmp;
                tmp = tmp->next;
            }
            prev->next = tab_entry;
        }
    }

    for (unsigned uu = 0; uu <= 65535u; uu++) {
        for (RoutingTable *e = tab[uu]; e; e = e->next) {
            if (e->time) {
                continue; // Already linked, skip
            }

            double *shared = (double *) malloc(sizeof(*shared));
            if (!shared) {
                exit(1);
            }
            *shared = -DBL_MAX;
            e->time = shared;

            // Find reverse edge vv->uu and point it to the same double
            unsigned short vv = e->destination;
            for (RoutingTable *r = tab[vv]; r; r = r->next) {
                if (r->destination == uu) {
                    r->time = shared;
                    break;
                }
            }
        }
    }

    fclose(fp);
}

void clear_table(RoutingTable **tab) {
    if (tab == NULL) {
        return;
    }

    for (unsigned i = 0; i <= 65535u; i++) {
        RoutingTable *list = tab[i];
        while (list != NULL) {
            RoutingTable *next = list->next;
            if (i < list->destination && list->time != NULL) {
                free(list->time);
            }
            free(list);
            list = next; // traverses to end
        }
        tab[i] = NULL;
    }
}

void print_table(RoutingTable **adj, RoutingTable **tab, char *name) {
    printf("\n---> Start of %s\n\n", name);
    for (unsigned i = 0; i <= 65535u; i++) {
        RoutingTable *list = tab[i];
        if (!adj[i] || list == NULL) {
            continue;
        }

        printf("AS %05hu:", i);
        for (RoutingTable *p = list; p != NULL; p = p->next) {
            printf("\n(dest=%05u, {type=%hu, len=%hu}, hop=%05u)", p->destination, p->type_length.type, p->type_length.len, p->next_hop);
        }
        printf("\n\n");
    }
    printf("End of %s <---\n", name);
    printf("\n");
}
