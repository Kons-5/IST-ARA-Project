#include "../../include/sequential/stats.h"
#include "../../include/sequential/tl.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

unsigned long TypeCount[5];
unsigned long TotalTypes = 0;
unsigned long AvgPaths = 0;
unsigned long LenHist[65536];
unsigned long TotalLengths = 0;

void StatsReset(void) {
    memset(TypeCount, 0, sizeof TypeCount);
    memset(LenHist, 0, sizeof LenHist);
    TotalTypes = 0;
    TotalLengths = 0;
    AvgPaths = 0;
}

void AccStats(RoutingTable *adj[], RoutingTable *E_t[], unsigned short t) {
    for (unsigned u = 0; u <= 65535u; u++) {
        if (!adj[u] || !E_t[u] || u == t)
            continue;

        tl_type tl = E_t[u]->type_length;
        if (E_t[u]->next != NULL) {
            AvgPaths = AvgPaths + 1;
        }

        // Increment type
        TypeCount[tl.type]++;
        AvgPaths = AvgPaths + 1;
        TotalTypes = TotalTypes + 1;

        // Increment length
        LenHist[tl.len]++;
        TotalLengths = TotalLengths + 1;
    }
}

static double pmf_calc(unsigned long num, unsigned long den) {
    return (den == 0) ? 0.0 : (100.0 * (double) num / (double) den);
}

void PrintStatsColumns(int col_width) {
    if (col_width <= 0) {
        col_width = 10;
    }
    if (col_width > 10) {
        col_width = 10;
    }

    int W = col_width;

    // Types
    printf("Types:\n");
    printf("  %*s %*s %*s\n", W, "Type", W, "PMF", W, "Count");
    printf("  %*s %*.3f%% %*lu\n", W, "Customer", W, pmf_calc(TypeCount[1], TotalTypes), W, TypeCount[1]);
    printf("  %*s %*.3f%% %*lu\n", W, "Peer", W, pmf_calc(TypeCount[2], TotalTypes), W, TypeCount[2]);
    printf("  %*s %*.3f%% %*lu\n", W, "Provider", W, pmf_calc(TypeCount[3], TotalTypes), W, TypeCount[3]);
    printf("  %*s %*.3f%% %*lu\n", W, "Invalid", W, pmf_calc(TypeCount[4], TotalTypes), W, TypeCount[4]);
    printf("\n");

    // Lengths
    printf("Lengths:\n");
    printf("  %*s %*s %*s %*s\n", W, "Length", W, "PMF", W, "CCDF", W, "Count");
    unsigned long remaining = TotalLengths; /* mass for X >= current L */
    for (unsigned L = 1; L <= 65535u; L++) {
        if (LenHist[L] == 0) {
            continue;
        }
        double pmf = pmf_calc(LenHist[L], TotalLengths);
        double ccdf = (TotalLengths == 0) ? 0.0 : (100.0 * (double) remaining / (double) TotalLengths);

        printf("  %*hu %*.3f%% %*.3f%% %*lu\n", W, (unsigned short) L, W, pmf, W, ccdf, W, LenHist[L]);

        if (remaining >= LenHist[L]) {
            remaining -= LenHist[L];
        }
    }
    printf("\n");

    // Paths per node
    double avg = (TotalTypes == 0) ? 0.0 : ((double) AvgPaths / (double) TotalTypes);
    printf("Average number of paths per node: %.3f\n", avg);
    printf("\n");
}

void PrintStats(void) {
    PrintStatsColumns(16);
}
