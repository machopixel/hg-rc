#include "w5a.h"
#include "fns.h"
#include "w2j.h"
#include "../unsorted_1000/w5a.h"
#include "../unsorted_1002/w2i.h"
#include "../unsorted_1007/w2e.h"
#include "../util/access.h"
#include "w3p.h"

// FUNCTION: HGSIM 0x10040a37
void __cdecl Sim_10040a37(const uint8_t *p)
{
    int16_t kind = S16(p, 3);
    uint32_t byteValue = U8(p, 2);
    uint16_t word = U16(p, 5);

#ifndef NETG_MUTATE /* the checker's negative check only: no guard */
    if (g_w34NetOver != 0)          /* 1.2 (0x10034682): ignored once the game is over */
        return;
#endif

    if (kind != 0) {
        if (kind == 7)
            W5A_CALL_10016de7(byteValue, word);
    } else {
        W5A_CALL_10016632(byteValue);
    }
}

// FUNCTION: HGSIM 0x10041c4c
void __cdecl Sim_10041c4c(uint32_t unused, int idx)
{
    char text[0x50];

    if (GS32(0x1017b8c8) == idx)
        return;
    W5A_SPRINTF(text, SIM_PTR(char, 0x1013ae1c), (char *)(uintptr_t)(PTRTAB(idx) + 0x124),
                (char *)(uintptr_t)G32(0x101c49f8));
    Sim_1007df6c(text, (const char *)(uintptr_t)G32(0x109b98c0), 0x712, 0x50);
    G32(0x101655ac) = (uint32_t)idx & 0xff;
    if (G32(0x1015ec74) == 0)
        G32(0x1015ec74) = 1;
}

// FUNCTION: HGSIM 0x10044909
void __cdecl Sim_10044909(int flag)
{
    float before = *(const float *)(uintptr_t)(G32(0x1014cae0) + 0x18);
    float after;

    if (flag != 0) {
        G32(0x1013aecc) = 0x3f800000;
        G32(0x1013aed0) = 0x3f800000;
    }
    if (Sim_100448ff() == 0)
        U32(G32(0x1014cae0), 0x18) = G32(0x1013aecc);
    else
        U32(G32(0x1014cae0), 0x18) = G32(0x1013aed0);
    after = *(const float *)(uintptr_t)(G32(0x1014cae0) + 0x18);
    if (before < after || before > after) {
        G32(0x1013a014) = 1;
        Sim_1000ebfb(SIM_PTR(char, 0x1013aee8), 0xfffff63cu, 0, 5, 0x50);
    }
}

// FUNCTION: HGSIM 0x10044c7a
void __cdecl Sim_10044c7a(void)
{
    void *record;

    if (GS32(0x1013aed4) != 3) {
        G32(0x1013aed8) = G32(0x1013aed4);
        Sim_10044992((const uint32_t *)(uintptr_t)G32(0x1014cae0), SIM_PTR(uint32_t, 0x109b9a20));
        G32(0x1013aed0) = 0x40000000;
        Sim_10044909(0);
    }
    record = Sim_100258d7();
    if (record != 0) {
        Sim_100449fc((void *)(uintptr_t)G32(0x1014cae0), record);
    } else {
        Sim_100448cd(GS32(0x1013aed8));
        if (GS32(0x1013aed8) == 2)
            Sim_100449fc((void *)(uintptr_t)G32(0x1014cae0), SIM_PTR(void, 0x109b9a20));
    }
}

// FUNCTION: HGSIM 0x1004ed37
int __cdecl Sim_1004ed37(const uint8_t *p)
{
    int result = 2;
    int width;
    int height;

    if (p != 0 && GS32(0x1013b9a8) == 1) {
        G32(0x109b8be0) = (uint32_t)(uintptr_t)p;
        width = S32(p, 0xc) - S32(p, 4) + 1;
        height = S32(p, 0x10) - S32(p, 8) + 1;
        result = W5A_CALL_1004deb3(S32(p, 4), S32(p, 8), width, height, (uint32_t)(uintptr_t)SIM_PTR(void, 0x10002955));
        G32(0x1013b9a8) = 2;
    }
    return result;
}
