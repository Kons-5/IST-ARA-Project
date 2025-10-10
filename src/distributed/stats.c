#include "../../include/distributed/stats.h"
#include "../../include/distributed/tl.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

unsigned long TypeCount[5];
unsigned long TotalTypes = 0;
unsigned long AvgPaths = 0;
unsigned long LenHist[65536];
unsigned long TotalLengths = 0;
unsigned long MsgHist[65536];
unsigned long TotalMsgs = 0;

void StatsReset(void) {
    memset(TypeCount, 0, sizeof TypeCount);
    memset(LenHist, 0, sizeof LenHist);
    memset(MsgHist, 0, sizeof MsgHist);
    TotalTypes = 0;
    TotalLengths = 0;
    AvgPaths = 0;
    TotalMsgs = 0;
}

void AccStats(RoutingTable *adj[], unsigned long seq, RoutingTable *stl[], unsigned short t) {
    for (unsigned u = 0; u <= 65535u; u++) {
        if (!adj[u] || !stl[u] || u == t)
            continue;

        tl_type tl = stl[u]->type_length;
        if (stl[u]->next != NULL) {
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

    // Store message count
    MsgHist[t] = seq;
    TotalMsgs = TotalMsgs + MsgHist[t];
}

static double CalcPMF(unsigned long num, unsigned long den) {
    return (den == 0) ? 0.0 : (100.0 * (double) num / (double) den);
}

static int cmp_longs(const void *a, const void *b) {
    const unsigned long *x = (const unsigned long *) a;
    const unsigned long *y = (const unsigned long *) b;
    if (*x < *y)
        return -1;
    if (*x > *y)
        return 1;
    return 0;
}

static void PrintMsgsCCDF(int W) {
    // collect per-destination message counts (MsgHist[t] > 0)
    unsigned long dests = 0;
    for (unsigned t = 0; t <= 65535u; ++t) {
        if (MsgHist[t] > 0) {
            dests++;
        }
    }

    printf("Messages:\n");
    printf("%*s %*s %*s %*s\n", W, "Msgs", W, "PMF", W, "CCDF", W, "Count");

    if (dests == 0) {
        printf("\n");
        return;
    }

    unsigned long *vals = (unsigned long *) malloc(dests * sizeof *vals);
    if (!vals) {
        printf("\n");
        return;
    }

    unsigned long k = 0;
    for (unsigned t = 0; t <= 65535u; ++t) {
        if (MsgHist[t] > 0) {
            vals[k++] = MsgHist[t]; // exactly the messages when destination was t
        }
    }

    // sort ascending and run-length encode
    qsort(vals, dests, sizeof *vals, cmp_longs);

    unsigned long i = 0;
    unsigned long remaining = dests; // #destinations with msgs >= current value
    while (i < dests) {
        unsigned long v = vals[i];
        unsigned long j = i + 1;
        while (j < dests && vals[j] == v) {
            j++;
        }

        unsigned long count = j - i;
        double pmf = CalcPMF(count, dests);
        double ccdf = CalcPMF(remaining, dests);

        printf("%*lu %*.3f%% %*.3f%% %*lu\n", W, v, W, pmf, W, ccdf, W, count);

        if (remaining >= count) {
            remaining -= count;
        }
        i = j;
    }
    printf("\n");

    free(vals);
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
    printf("%*s %*s %*s\n", W, "Type", W, "PMF", W, "Count");
    printf("%*s %*.3f%% %*lu\n", W, "Customer", W, CalcPMF(TypeCount[1], TotalTypes), W, TypeCount[1]);
    printf("%*s %*.3f%% %*lu\n", W, "Peer", W, CalcPMF(TypeCount[2], TotalTypes), W, TypeCount[2]);
    printf("%*s %*.3f%% %*lu\n", W, "Provider", W, CalcPMF(TypeCount[3], TotalTypes), W, TypeCount[3]);
    printf("%*s %*.3f%% %*lu\n", W, "Invalid", W, CalcPMF(TypeCount[4], TotalTypes), W, TypeCount[4]);
    printf("\n");

    // Lengths
    printf("Lengths:\n");
    printf("%*s %*s %*s %*s\n", W, "Length", W, "PMF", W, "CCDF", W, "Count");
    unsigned long remaining = TotalLengths; // mass for X >= current L
    for (unsigned L = 1; L <= 65535u; L++) {
        if (LenHist[L] == 0) {
            continue;
        }
        double pmf = CalcPMF(LenHist[L], TotalLengths);
        double ccdf = (TotalLengths == 0) ? 0.0 : (100.0 * (double) remaining / (double) TotalLengths);

        printf("%*hu %*.3f%% %*.3f%% %*lu\n", W, (unsigned short) L, W, pmf, W, ccdf, W, LenHist[L]);

        if (remaining >= LenHist[L]) {
            remaining -= LenHist[L];
        }
    }
    printf("\n");

    // Messages CCDF across destinations t
    PrintMsgsCCDF(W);

    // Paths per node
    double avg = (TotalTypes == 0) ? 0.0 : ((double) AvgPaths / (double) TotalTypes);
    printf("Average number of paths per node: %.3f\n", avg);
    printf("\n");
}

void PrintStats(void) {
    PrintStatsColumns(20);
}
