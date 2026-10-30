/*
 * Round 5 (letter z): small functions of HGSIM.DLL between 0x100a773b and 0x100ae470.
 * Calls of functions that are not converted yet go through their original
 * address (SIM_FN); the others call the converted C functions by name.
 */
#ifndef HG_SIM_UNSORTED_100A_W5Z_H
#define HG_SIM_UNSORTED_100A_W5Z_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Returns the original 0x100a7c4c(dword 0x1014cf8c). */
uint32_t __cdecl Sim_100a773b(void);

/*
 * When the original 0x100282d4(dword 0x1014cf8c, key, name, 0) answers non-zero, calls Sim_1002828e(key, name)
 * and returns 1; otherwise returns 0.
 */
int __cdecl Sim_100a7758(int key, const void *name);

/* When the original 0x100ba7d5(a, b, 0) answers non-zero, calls the original 0x100bb2b0(a, b) and returns 1; otherwise returns 0. */
int __cdecl Sim_100a77a7(uint32_t a, uint32_t b);

/* Calls the original 0x100a9c25(obj), then returns the original 0x10060681(obj+0xc, a, b, c). */
uint32_t __cdecl Sim_100a9ccf(void *obj, uint32_t a, uint32_t b, uint32_t c);

/* Calls the original 0x1006123b(a, b, c, d, obj+0x3c), then clears bit 0x4000 and sets bit 1 of the dword at obj+0x94. */
void __cdecl Sim_100aa0da(void *obj, uint32_t a, uint32_t b, uint32_t c, uint32_t d);

/*
 * With blk = dword at obj+0x30 and t = dword at obj+0x28: calls the original 0x100a9ba7(t),
 * Sim_100098bb(blk, t+0x4c) and stores the dword at obj+0x9c at blk+0x10.
 */
void __cdecl Sim_100aa307(void *obj);

/* Sim_1008a37f(dword at obj+0x8c) when that is not 0; then the same for each node of the chain that starts at obj+4 (next at +8), each one recursively. */
void __cdecl Sim_100aa406(void *obj);

/* Stores the low word of v at +0x24 of the node at obj+0x8c when that is not 0, then does the same recursively for the chain that starts at obj+4 (next at +8). */
void __cdecl Sim_100aa541(void *obj, uint32_t v);

/*
 * r = original 0x100fe830(buf, 1, count, dword at f); Sim_1003c860(buf, r, dword at f+4);
 * the dword at f+4 grows by r; returns r.
 */
uint32_t __cdecl Sim_100ae470(void *f, void *buf, uint32_t count);

#endif
