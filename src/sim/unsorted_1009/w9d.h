/*
 * Round 9, letter d: the radar pane title of HGSIM.DLL at 0x1009a137. Floating point: the pane centre is
 * made as (float)int and truncated back through Render_Ftol, bit-exact.
 *
 * Callees that are not converted (or patched elsewhere) are called through their original address:
 *   0x100ba7d5 (name, key, 0) -> shape (patched by another hook file; upper-cases name in place)
 *   0x100f0205 / 0x100e91dc (pane, shape, 0, x, y): the text blit (0x100f0205 is patched by another hook file)
 */
#ifndef HG_SIM_UNSORTED_1009_W9D_H
#define HG_SIM_UNSORTED_1009_W9D_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

typedef void *(__cdecl *W9dRadarShapeFn)(void *name, void *key, int zero);
typedef int(__cdecl *W9dBlitFn)(void *pane, void *shape, uint32_t zero, uint32_t x, uint32_t y);

#define W9D_RADAR_CALL_100ba7d5 SIM_FN(W9dRadarShapeFn, 0x100ba7d5)
#define W9D_CALL_100f0205 SIM_FN(W9dBlitFn, 0x100f0205)
#define W9D_CALL_100e91dc SIM_FN(W9dBlitFn, 0x100e91dc)

/* ---- w9d_radar.c ---- */
/*
 * Sim_1009aac9(obj), then unless [0x1015d1ac] is 4: the first 8 characters of the text *(obj[0x70]) (the end
 * mark goes to [len + 1], len at most 7) make a shape through 0x100ba7d5 with the key [0x1014cec0]; when there
 * is one it is blitted at the centre of the pane obj[0] (((+0xc - +4 + 1) >> 1, (+0x10 - +8 + 1) >> 1) through
 * float), Sim_100bb2b0(label, key) and Sim_1009a3b3(obj) follow. Always ends with Sim_1009a6f3(obj).
 */
void __cdecl Sim_1009a137(void *obj);

#endif
