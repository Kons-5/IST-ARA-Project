#include "../../include/sequential/stats.h"
#include <stdio.h>
#include <string.h>

unsigned long TypeCount[5];
unsigned long TotalTypes = 0;
unsigned long LenHist[65536];
unsigned long TotalLengths = 0;

void StatsReset(void) {
    memset(TypeCount, 0, sizeof TypeCount);
    memset(LenHist, 0, sizeof LenHist);
    TotalTypes = 0;
    TotalLengths = 0;
}

void AccStats(RoutingTable *adj[], RoutingTable *E_t[], unsigned short t) {
    for (unsigned u = 0; u <= 65535u; u++) {
        if (!adj[u] || !E_t[u] || u == t) {
            continue;
        }
        tl_type tl = E_t[u]->type_length;

        // Increment type
        TypeCount[tl.type]++;
        TotalTypes = TotalTypes + 1;

        // Increment length
        LenHist[tl.len]++;
        TotalLengths = TotalLengths + 1;
    }
}

void PrintStats(void) {
    // Type statistics
    printf("Types (percentage):\n");
    printf("  Customer: %.3f%%\n", 100.0 * (double) TypeCount[1] / (double) TotalTypes);
    printf("  Peer    : %.3f%%\n", 100.0 * (double) TypeCount[2] / (double) TotalTypes);
    printf("  Provider: %.3f%%\n", 100.0 * (double) TypeCount[3] / (double) TotalTypes);
    printf("  Invalid : %.3f%%\n", 100.0 * (double) TypeCount[4] / (double) TotalTypes);
    printf("\n");

    // Length statistics
    printf("Lengths (percentage):\n");
    for (unsigned L = 1; L <= 65535u; L++) {
        if (LenHist[L] == 0) {
            continue; // skip empty bins
        }

        double percentage = 100.0 * (double) LenHist[L] / (double) TotalLengths;
        printf("  Length %hu: %.3f%%\n", L, percentage);
    }
    printf("\n");
}
