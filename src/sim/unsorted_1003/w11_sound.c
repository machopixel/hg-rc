#include "w11.h"
#include "../simmem.h"
#include "../util/access.h"
#include "w7m.h"
#include "../unsorted_1008/w8w.h"

/* The DirectSound versions of 0x10039708 (pitch), 0x1003a548 (shutdown: Release and CoUninitialize), 0x10039931 (3D
   voice position) and 0x100397e7 (3D listener) are deleted with the DirectSound path (hgrc.cfg "dsound"; DECISIONS.md,
   "DirectSound dropped"): the game's sound is SDL3's (sim/sound/sdl_sound.c). */

typedef uint32_t(__stdcall *W11GetProcAddressFn)(uint32_t module, const char *name);

/* the names of the library functions, in the order of their slots 0x1015ea8c.. */
static const uint32_t g_w11NetNames[55] = {
    0x1013a794, 0x1013a7a4, 0x1013a7b8, 0x1013a7cc, 0x1013a7d8, 0x1013a7e4, 0x1013a7f4, 0x1013a808, 0x1013a81c,
    0x1013a828, 0x1013a838, 0x1013a840, 0x1013a84c, 0x1013a854, 0x1013a85c, 0x1013a870, 0x1013a880, 0x1013a890,
    0x1013a8a0, 0x1013a8b0, 0x1013a8c0, 0x1013a8d0, 0x1013a8e0, 0x1013a8f4, 0x1013a900, 0x1013a910, 0x1013a920,
    0x1013a930, 0x1013a944, 0x1013a95c, 0x1013a970, 0x1013a980, 0x1013a990, 0x1013a9a0, 0x1013a9ac, 0x1013a9bc,
    0x1013a9c8, 0x1013a9d4, 0x1013a9e4, 0x1013a9f8, 0x1013aa0c, 0x1013aa18, 0x1013aa24, 0x1013aa30, 0x1013aa40,
    0x1013aa50, 0x1013aa60, 0x1013aa6c, 0x1013aa7c, 0x1013aa8c, 0x1013aaa0, 0x1013aab4, 0x1013aac0, 0x1013aad4,
    0x1013aae4,
};

// FUNCTION: HGSIM 0x1003be30
void __cdecl Sim_1003be30(void)
{
    int k;

    for (k = 0; k < 55; k++)
        G32(0x1015ea8c + 4 * (uint32_t)k) =
            SIM_API(W11GetProcAddressFn, 0x10ac55dc)(G32(0x1015eb68), SIM_PTR(const char, g_w11NetNames[k]));
}
