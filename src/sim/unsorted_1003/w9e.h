/*
 * Round 9, letter e: functions of HGSIM.DLL at 0x10034ec9 (the frame driver of the 3-D view). Helpers that
 * are not converted (or patched elsewhere) are called through their original address with the prototypes
 * below.
 */
#ifndef HG_SIM_UNSORTED_1003_W9E_H
#define HG_SIM_UNSORTED_1003_W9E_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Patched by hooks_view.c: the camera viewport (0x100d2306), the current camera (0x100d1f90) and the visible
 * cells (0x10087d83). */
#define W9E_CALL_100d2306 SIM_FN(void(__cdecl *)(void *), 0x100d2306)
#define W9E_CALL_100d1f90 SIM_FN(void(__cdecl *)(void *), 0x100d1f90)
#define W9E_CALL_10087d83 SIM_FN(void(__cdecl *)(void *), 0x10087d83)
/* Not converted: draws the sky for the camera; clears the view (view record, colour); draws a cell. */
#define W9E_CALL_100a0def SIM_FN(void(__cdecl *)(void *), 0x100a0def)
#define W9E_CALL_100eb070 SIM_FN(void(__cdecl *)(void *, uint32_t), 0x100eb070)
#define W9E_CALL_100357db SIM_FN(void(__cdecl *)(uint32_t), 0x100357db)
/* The driver's polygon routine (slot 0x1014cb4c): (count, vertices, colour). */
typedef void(__cdecl *W9eDriverPolyFn)(int count, void *vertices, uint32_t colour);
/* The address of a routine of the original DLL, as the original stores it in a function-pointer variable. */
#define W9E_ADDR(addr) ((uint32_t)(uintptr_t)(g_simBase + ((addr) - SIM_PREFERRED_BASE)))

/* ---- w9e_frame.c ---- */
/*
 * Renders one frame of the 3-D view through the camera at [0x1014cae0]: clears the frame buffer when
 * [0x1015e53c] asks (Mem_Fill of [0x109e3e20] with [0x10138c70], Sim_100d32e5 of that preset), re-applies the
 * viewport when 0x1013a014 is set, publishes the view rectangle [0x109e3e00] at 0x109e3dc4..0x109e3ddc (8-bit
 * display only), then either only clears the view (0x100eb070, when [0x1014cae8] with the 8-bit display) or
 * draws: Sim_100d255e, 0x100d1f90, the clear or the sky (0x100a0def), the light direction (Sim_100b53f0), the
 * far scene [0x1015e544] with the far plane 0x7f7fc99e and the far polygon routine 0x10001ab9 in 0x1014cb40,
 * Sim_100a4d8e, the near scene [0x1015e548] with the near plane 8.0 (0x41000000) and routine 0x10002ce3, the
 * cells (0x10087d83, then 0x100357db for every entry of 0x1036f2a0 up to the first 0) when [0x1015e52c], and
 * finally Sim_100071f6 and Sim_100d32e5(0). 0x1015e54c sums the polygon counts 0x1017d138 of the three scene
 * passes.
 */
void __cdecl Sim_10034ec9(void);

#endif
