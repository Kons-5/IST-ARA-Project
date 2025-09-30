#include "../../include/sequential/tab.h"
#include <stdio.h>
#include <stdlib.h>

static RoutingTable *add_adjancency(unsigned short u, unsigned short v, tl_type tl) {
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

    return entry;
}

void read_table(const char *path, unsigned short t, RoutingTable **tab, RoutingTable **Et) {
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

        RoutingTable *tab_entry = add_adjancency(v, v, tmp);
        if (tab_entry == NULL) {
            exit(1);
        }

        // if the routing table is empty, also initialize Et
        if (tab[u] == NULL) {
            tl_type tmp = (tl_type){
                .type = TL_INVALID,
                .len = 0u,
            };

            RoutingTable *stab_entry = add_adjancency(u, t, tmp);
            if (stab_entry == NULL) {
                exit(1);
            }

            tab[u] = tab_entry;
            Et[u] = stab_entry;
        } else {
            // insert new neighbor at the end
            RoutingTable *tmp = tab[u];
            RoutingTable *prev = NULL;
            while (tmp != NULL) {
                prev = tmp;
                tmp = tmp->next;
            }
            prev->next = tab_entry;
        }
    }

    fclose(fp);
}

void clear_table(RoutingTable **tab) {
    if (tab == NULL) {
        return;
    }

    for (size_t i = 0; i <= 65535u; ++i) {
        RoutingTable *list = tab[i];
        while (list != NULL) {
            RoutingTable *next = list->next;
            free(list);  // traverses to end
            list = next;
        }
        tab[i] = NULL;
    }
}

void print_table(RoutingTable **tab, char *name) {
    printf("\n---> Start of %s\n\n", name);
    for (unsigned short i = 0; i < 65535u; ++i) {
        RoutingTable *list = tab[i];
        if (list == NULL) {
            continue;
        }

        printf("AS %05u:", i);
        for (RoutingTable *p = list; p != NULL; p = p->next) {
            printf("\n(dest=%05u, {type=%hu, len=%hu}, hop=%05u)",
                   p->destination,
                   p->type_length.type,
                   p->type_length.len,
                   p->next_hop);
        }
        printf("\n\n");
    }
    printf("End of %s <---\n", name);
    printf("\n");
}
