#include "w6e.h"
#include "fns.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"
#include "../crt/hg_winheap.h"

#define SOUND_STATE G32(0x1015d0a4)

// FUNCTION: HGSIM 0x10039aca
int __cdecl Sound_Init(uint32_t voices, uint32_t window)
{
    uint32_t i;

    if (SOUND_STATE != 0)
        return -1;
    SOUND_STATE = (uint32_t)(uintptr_t)Hg_HeapAlloc(G32(0x1017b8f8), 1, 0x374);
    if (SOUND_STATE == 0)
        return -3;
    Mem_Fill((void *)(uintptr_t)SOUND_STATE, 0, 0x374);
    /* the voice count at +4 is at most 11 (unsigned compare) */
    if (voices <= 0xb)
        U32(SOUND_STATE, 4) = voices;
    else
        U32(SOUND_STATE, 4) = 0xb;
    Sim_1003a52b(0);
    W6E_CALL_1003a0e4(SIM_PTR(void, 0x109e369c), window);
    if (G32(0x109e369c) == 0) {
        Hg_HeapFree(G32(0x1017b8f8), 1, (void *)(uintptr_t)SOUND_STATE);
        SOUND_STATE = 0;
        return -4;
    }
    for (i = 0; i < U32(SOUND_STATE, 4); i++) {
        U32(SOUND_STATE, 0x54 + i * 4) = 0;
        U32(SOUND_STATE, 0x1d4 + i * 4) = 0xffffffffu;
        U32(SOUND_STATE, 0x214 + i * 4) = 0;
        U32(SOUND_STATE, 0x254 + i * 4) = 0;
    }
    return 1;
}
