/*
 * Round 5, letter a: functions of 0x100579ad..0x1005ae79 that call helpers which are not converted yet.
 * Those helpers are called through their address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1005_W5A_H
#define HG_SIM_UNSORTED_1005_W5A_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: (dword [0x1017d594], value); returns a dword. */
#define W5A_CALL_100b7834 SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t), 0x100b7834)
/* Not converted: the C library's strstr (string, substring). */
#define W5A_STRSTR SIM_FN(char *(__cdecl *)(const char *, const char *), 0x100fc970)

/*
 * Once only (the flag [0x101668a4] is set by the first call): copies the 0x7d dwords of a to 0x10143808
 * when a is not null and replaces the 0x7d dwords at 0x10165f60 by 0x100b7834([0x1017d594], b[i])
 * when b is not null.
 */
void __cdecl Sim_100579ad(const uint32_t *a, const uint32_t *b);
/*
 * Searches the tree of nodes (the children are the pointers at +4 and +8) for the first node, node first,
 * then the +4 subtree, then the +8 subtree, whose string at +0x90 contains key; returns it or NULL.
 */
void *__cdecl Sim_1005ae79(void *node, const char *key);

#endif
