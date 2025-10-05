#include "../../include/sequential/stats.h"
#include "../../include/sequential/tl.h"
#include <stdio.h>
#include <string.h>

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
        if (!adj[u] || !E_t[u] || u == t) {
            continue;
        }

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

void PrintStats(void) {
    printf("Numbers:\n");
    printf("  Customer: %d\n", (int) TypeCount[1]);
    printf("  Peer    : %d\n", (int) TypeCount[2]);
    printf("  Provider: %d\n", (int) TypeCount[3]);
    printf("  Invalid : %d\n", (int) TypeCount[4]);
    printf("\n");

    // Type statistics
    printf("Types (percentage):\n");
    printf("  Customer: %.3f%%\n", 100.0 * (double) TypeCount[1] / (double) TotalTypes);
    printf("  Peer    : %.3f%%\n", 100.0 * (double) TypeCount[2] / (double) TotalTypes);
    printf("  Provider: %.3f%%\n", 100.0 * (double) TypeCount[3] / (double) TotalTypes);
    printf("  Invalid : %.3f%%\n", 100.0 * (double) TypeCount[4] / (double) TotalTypes);
    printf("\n");

    // Length statistics
    printf("Lengths:\n");
    for (unsigned L = 1; L <= 65535u; L++) {
        if (LenHist[L] == 0) {
            continue; // skip empty bins
        }

        double percentage = 100.0 * (double) LenHist[L] / (double) TotalLengths;
        printf("  Length %hu: %.3f%% (%zu)\n", L, percentage, LenHist[L]);
    }
    printf("\n");

    // Number of paths per node
    printf("Average number of paths per node: %.3f\n", (double)AvgPaths / (double) TotalTypes);
    printf("\n");
}
