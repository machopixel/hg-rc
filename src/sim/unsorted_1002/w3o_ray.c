#include "w3o.h"
#include "w3q.h"
#include "w5c.h"
#include "w5x.h"
#include "../unsorted_1000/w3o.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_1009/w3o.h"
#include "../util/w3o.h"

/* The three original tests of cell objects, by the type dword at +0x34 (they are jump thunks). */
typedef int(__cdecl *CellTest)(void *node, float x, float y, float z, float *result);

// FUNCTION: HGSIM 0x1002a0ca
void __cdecl Sim_1002a0ca(void)
{
    uint32_t rec = PTRTAB(G32(0x1017b8c8));

    if (rec != 0) {
        uint32_t fn = U32(rec, 0x40);

        if (fn != 0 && U32(rec, 0x28) != 0)
            ((void(__cdecl *)(uint32_t))(uintptr_t)fn)(U32(rec, 0x28));
    }
}

// FUNCTION: HGSIM 0x1002d961
double __cdecl Sim_1002d961(float x, float y, float z)
{
    int n = 0;
    uint32_t bestBits = 0xff7fc99eu;    /* the nearest hit that is within the float at 0x101112c8 of y */
    uint32_t anyBits = 0xff7fc99eu;     /* the largest hit */
    uint32_t cell;

    G32(0x109e8230) = 0;
    Sim_100881dd(x, z);
    cell = G32(0x1036f2a0u + (uint32_t)n * 4);
    while (cell != 0) {
        uint32_t node;

        n++;
        for (node = U32(cell, 0x18); node != 0; node = U32(node, 0x18)) {
            CellTest test;
            float out;
            uint32_t type = U32(node, 0x34);

            /* the original calls them through the DLL's jump table (0x100035df, 0x10001e1f, 0x10002775) */
            if (type == 5)
                test = Sim_1002d3ca;
            else if (type == 8)
                test = (CellTest)Sim_1002d479;
            else if (type == 0)
                test = Sim_1002cde1;
            else
                continue;
            if (test((void *)(uintptr_t)node, x, y, z, &out) == 0)
                continue;
            G32(0x109e8230) = node;
            if (bestBits == 0xff7fc99eu && out > Float_OfBits(anyBits)) {
                anyBits = Bits_OfFloat(out);
                G32(0x1015dd4c) = G32(0x1015dd40);
                G32(0x1015dd50) = G32(0x1015dd44);
                G32(0x1015dd54) = G32(0x1015dd48);
            }
            if ((double)out + GFL(0x101112c8) > y && !((double)out - GFL(0x101112c8) >= y) && out > Float_OfBits(bestBits)) {
                bestBits = Bits_OfFloat(out);
                G32(0x1015dd4c) = G32(0x1015dd40);
                G32(0x1015dd50) = G32(0x1015dd44);
                G32(0x1015dd54) = G32(0x1015dd48);
            }
        }
        cell = G32(0x1036f2a0u + (uint32_t)n * 4);
    }
    if (Float_OfBits(bestBits) > GFL(0x101112cc))
        return Float_OfBits(bestBits);
    if (Float_OfBits(anyBits) > GFL(0x101112cc))
        return Float_OfBits(anyBits);
    return GFL(0x101112c4);
}

// FUNCTION: HGSIM 0x1002dc60
int __cdecl Sim_1002dc60(float x, float y, float z, void **found)
{
    *found = Sim_1002dcb3((void *)(uintptr_t)G32(0x1017c8cc), x, y, z);
    if (*found != 0 && Sim_1002dd6c(*found, x, y, z) != 0)
        return 1;
    return 0;
}

// FUNCTION: HGSIM 0x1002dcb3
void *__cdecl Sim_1002dcb3(void *first, float x, float y, float z)
{
    void *best = 0;
    int n = 0;
    float bestLen = Float_OfBits(0x7f7fc99eu);
    uint32_t cell;

    if (first == 0)
        return 0;
    Sim_100881dd(x, z);
    cell = G32(0x1036f2a0u);
    while (cell != 0) {
        uint32_t node;

        n++;
        for (node = U32(first, 0x10); node != 0; node = U32(node, 0x10)) {
            double len = Sim_10009ca8((void *)(uintptr_t)node, x, y, z);

            if (!(len >= bestLen)) {
                bestLen = (float)len;
                best = (void *)(uintptr_t)node;
            }
        }
        cell = G32(0x1036f2a0u + (uint32_t)n * 4);
    }
    return best;
}

// FUNCTION: HGSIM 0x1002dd6c
int __cdecl Sim_1002dd6c(void *obj, float x, float y, float z)
{
    uint32_t fn = G32(0x10138bc0u + U32(obj, 0x34) * 12);

    if (fn == 0)
        return 1;
    return ((int(__cdecl *)(void *, float, float, float))(uintptr_t)fn)(obj, x, y, z);
}

// FUNCTION: HGSIM 0x1002e6de
void __cdecl Sim_1002e6de(void *obj, float *vec, float t)
{
    if (t > GFL(0x101112c4))
        Sim_1009cd36(vec, t);
    if (Bits_OfFloat(t) == 0x41200000u) {
        GFL(0x1015dd40) = (float)((double)vec[0] - FL(obj, 0x44));
        GFL(0x1015dd44) = (float)((double)vec[1] - FL(obj, 0x48));
        GFL(0x1015dd48) = (float)((double)vec[2] - FL(obj, 0x4c));
    } else {
        GFL(0x1015dd40) = (float)((double)vec[3] - FL(obj, 0x44));
        GFL(0x1015dd44) = (float)((double)vec[4] - FL(obj, 0x48));
        GFL(0x1015dd48) = (float)((double)vec[5] - FL(obj, 0x4c));
    }
    Sim_10084c95(SIM_PTR(f32o, 0x1015dd40), SIM_PTR(f32o, 0x1015dd44), SIM_PTR(f32o, 0x1015dd48));
}
