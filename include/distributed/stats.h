#ifndef STATS_H
#define STATS_H

#include "../../include/sequential/tab.h"

extern unsigned long TypeCount[5];
extern unsigned long TotalTypes;
extern unsigned long LenHist[65536];
extern unsigned long TotalLengths;

void AccStats(RoutingTable *adj[], unsigned long seq, RoutingTable *E_t[], unsigned short t);
void StatsReset(void);
void PrintStats(void);

typedef void (*StableAllToggle)(RoutingTable *adj[], unsigned long seq, RoutingTable *E_t[], unsigned short t);

#endif
