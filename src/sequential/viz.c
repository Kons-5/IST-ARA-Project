#include "../../include/sequential/tl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct RoutingTable {
    int destination;            // destination
    int next_hop;               // next-hop
    tl_type type_length;        // type-length
    struct RoutingTable *next;  // next edge
} RoutingTable;

static void run_cmd(const char *cmd) {
    int rc = system(cmd);
    if (rc == -1) { perror("system"); return; }
    if (WIFEXITED(rc)) {
        int code = WEXITSTATUS(rc);
        if (code != 0) fprintf(stderr, "`%s` exited with %d\n", cmd, code);
    } else if (WIFSIGNALED(rc)) {
        fprintf(stderr, "`%s` killed by signal %d\n", cmd, WTERMSIG(rc));
    }
}

static const char *edge_label(unsigned t) {
    switch (t) {
        case 1: return "customer";
        case 2: return "peer";
        case 3: return "provider";
        default: return "-";
    }
}

static const char *edge_color_two(unsigned a, unsigned b) {
    if (a == b) {
        switch (a) {
            case 1: return "forestgreen"; /* customer */
            case 2: return "dodgerblue"; /* peer */
            case 3: return "crimson"; /* provider */
            default: return "gray50";
        }
    }
    /* asymmetric: neutral color */
    return "gray40";
}

static unsigned find_reverse_type(RoutingTable **tab, unsigned v, unsigned u) {
    for (RoutingTable *e = tab[v]; e; e = e->next) {
        if ((unsigned)e->destination == u) {
            return (unsigned)e->type_length.type;
        }
    }
    return 0;
}

static void write_dot_bidirectional(FILE *out, RoutingTable **tab) {
    fprintf(out,
        "digraph G {\n"
        "  rankdir=TB;\n"
        "  bgcolor=\"white\";\n"
        "  nodesep=0.5; ranksep=1.0;\n"
        "  splines=true;\n"
        "  node [shape=circle, fontname=\"Sans\", style=filled, fillcolor=\"#f8f9fa\", penwidth=1.2];\n"
        "  edge [fontname=\"Sans\", penwidth=1.8, color=\"#777777\", arrowsize=0.8,\n"
        "        labeldistance=1.4, labelfontsize=10, labelangle=0];\n");

    for (unsigned u = 0; u < 65536u; ++u) {
        if (!tab[u]) continue;
        fprintf(out, "  \"%05u\";\n", u);
    }

    for (unsigned u = 0; u < 65536u; ++u) {
        for (RoutingTable *e = tab[u]; e; e = e->next) {
            unsigned v = (unsigned)e->destination;
            if (u >= v) continue;
            unsigned t_uv = (unsigned)e->type_length.type;
            unsigned t_vu = find_reverse_type(tab, v, u);
            if (t_uv == 2 || t_vu == 2) {
                // keep peers on the same vertical level
                fprintf(out, "  { rank=same; \"%05u\"; \"%05u\"; }\n", u, v);
            }
        }
    }

    for (unsigned u = 0; u < 65536u; ++u) {
        for (RoutingTable *e = tab[u]; e; e = e->next) {
            unsigned v = (unsigned)e->destination;
            if (u > v) continue;

            unsigned t_uv = (unsigned)e->type_length.type;
            unsigned t_vu = find_reverse_type(tab, v, u);
            if (t_uv == 0 && t_vu == 0) continue;

            const char *col = edge_color_two(t_uv, t_vu);
            const char *tlab = edge_label(t_uv);
            const char *hlab = edge_label(t_vu);

            int is_peer = (t_uv == 2 || t_vu == 2);

            fprintf(out,
                "  \"%05u\" -> \"%05u\" [dir=both, arrowhead=normal, arrowtail=normal, "
                "taillabel=\"%s\", headlabel=\"%s\", color=\"%s\"%s];\n",
                u, v, tlab, hlab, col, is_peer ? ", constraint=false" : "");
        }
    }

    fprintf(out, "}\n");
}

static int render_graph_png(RoutingTable **tab, const char *png_path) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, "dot -Tpng -o \"%s\"", png_path);

    FILE *dot = popen(cmd, "w");
    if (!dot) {
        fprintf(stderr, "Error: failed to run Graphviz 'dot'. Is it installed?\n");
        return -1;
    }
    write_dot_bidirectional(dot, tab);
    int rc = pclose(dot);
    if (rc != 0) {
        fprintf(stderr, "Error: graphviz 'dot' exited with code %d\n", rc);
        return -2;
    }
    return 0;
}

static void preview_png_in_terminal(const char *png_path) {
    const char *kitty = getenv("KITTY_WINDOW_ID");
    if (kitty) {
        char cmd[1024];
        snprintf(cmd, sizeof cmd, "kitty +kitten icat \"%s\"", png_path);
        run_cmd(cmd);
        return;
    }
    const char *term = getenv("TERM");
    if (term && strstr(term, "sixel")) {
        char cmd[1024];
        snprintf(cmd, sizeof cmd, "img2sixel \"%s\"", png_path);
        run_cmd(cmd);
        return;
    }
    printf("Graph image written to: %s\n", png_path);
}

void visualize_table(void **tab, const char *png_path) {
    RoutingTable **sigma = (RoutingTable **) tab;
    if (render_graph_png(sigma, png_path) == 0) {
        preview_png_in_terminal(png_path);
    }
}
