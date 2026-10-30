#include "w6h.h"
#include "w2l.h"
#include "w5a.h"
#include "w5c.h"
#include "../unsorted_1000/w5b.h"
#include "../unsorted_1001/w3t.h"
#include "../unsorted_1001/w5c.h"
#include "../unsorted_1003/w3s.h"
#include "../unsorted_100f/w2e.h"
#include "../util/w3t.h"

/* The bits of a float (the originals store float results and pass them on as dwords). */
static uint32_t W6h_Bits(float f)
{
    union {
        float f;
        uint32_t u;
    } v;

    v.f = f;
    return v.u;
}

// FUNCTION: HGSIM 0x100c17b7
int __cdecl Sim_100c17b7(void *obj)
{
    float angle = Sim_10015266(obj);

    Sim_1000ae00(obj, W6h_Bits(angle));
    if (W6H_FABS_C((double)angle) > (double)GFL32(0x1012bb60))
        Sim_100c0379(obj, Sim_100144e9(obj));
    Sim_100c4898(obj);
    return Sim_100c1853(obj);
}

// FUNCTION: HGSIM 0x100c25af
int __cdecl Sim_100c25af(void *obj)
{
    int result = 0;

    if ((U32(obj, 0xf0) & 0xf00) == 0x200 && SIM_CALL_100c3550(obj, U32(obj, 0x5c)) == 0)
        result = (double)FL32(obj, 0xe0) - (double)GFL32(0x1012bb64) > (double)FL32(obj, 0x5c)
                 && S32(U32(obj, 0x28), 0x510) >= 0;
    return result;
}

// FUNCTION: HGSIM 0x100c2851
int __cdecl Sim_100c2851(void *block, uint32_t *out, uint32_t skip, int use_own)
{
    int32_t i;

    for (i = 0; i < GS32(0x1015dc64); i++) {
        uint32_t rec = PTRTAB(i);
        uint32_t radius;
        float distance;

        if (U32(rec, 4) == skip || rec == 0)
            continue;
        radius = use_own != 0 ? U32(rec, 0x246) : U32(U32(rec, 0x28), 0x53c);
        distance = (float)SIM_CALL_10009fe8(U32(rec, 0x58), U32(rec, 0x5c), U32(rec, 0x60), radius, block);
        if (!(distance >= FL32(block, 0x30))) {
            *out = U32(U32(rec, 0x48), 0x8c);
            return 1;
        }
    }
    return 0;
}

/* The common end of Sim_100c3372 and Sim_100c3419: the direction of the difference vector against obj[0x68]. */
static int W6h_TurnSide(void *obj, float dx, float dy, float dz)
{
    float a, b, c, d;
    double diff;
    float side;

    SIM_CALL_100cbc6b(W6h_Bits(dx), W6h_Bits(dy), W6h_Bits(dz), &a, &b, &c, &d);
    diff = (double)a - (double)FL32(obj, 0x68);
    side = (float)diff;
    /* the first compare is on the unrounded difference, the second on the stored float */
    if (diff > (double)GFL32(0x1012bba4))
        side = (float)((double)side - (double)GFL32(0x1012bba8));
    else if (!((double)side >= (double)GFL32(0x1012bbac)))
        side = (float)((double)side + (double)GFL32(0x1012bba8));
    return (W6h_Bits(side) & 0x80000000u) != 0 ? 1 : -1;
}

// FUNCTION: HGSIM 0x100c3372
int __cdecl Sim_100c3372(void *obj, void *other, float x, float y, float z)
{
    float dx = (float)((double)FL32(other, 0x44) - (double)x);
    float dy = (float)((double)FL32(other, 0x48) - (double)y);
    float dz = (float)((double)FL32(other, 0x4c) - (double)z);

    return W6h_TurnSide(obj, dx, dy, dz);
}

// FUNCTION: HGSIM 0x100c3419
int __cdecl Sim_100c3419(void *obj, float x1, float y1, float z1, float x2, float y2, float z2)
{
    float dx = (float)((double)x1 - (double)x2);
    float dy = (float)((double)y1 - (double)y2);
    float dz = (float)((double)z1 - (double)z2);

    return W6h_TurnSide(obj, dx, dy, dz);
}

// FUNCTION: HGSIM 0x100c417e
int __cdecl Sim_100c417e(void *rec, void *other, uint32_t key, W6hEntryFn fn)
{
    uint32_t *list = 0;
    int count;
    int result;
    int i;

    if (rec == 0 || other == 0)
        return -1;
    count = SIM_CALL_10033eb3(U32(rec, 0x58), U32(rec, 0x5c), U32(rec, 0x60), key, &list);
    if (count == 0 || list == 0)
        return Sim_10033d75(FL32(rec, 0x58), FL32(rec, 0x5c), FL32(rec, 0x60));
    result = -1;
    for (i = 0; i < count; i++)
        result = fn(result, list[i], rec, other);
    return result;
}

// FUNCTION: HGSIM 0x100c43e1
int __cdecl Sim_100c43e1(void *obj, uint32_t key, int mode)
{
    void *other;

    if ((U16(obj, 0x1aa) & 0x200) == 0)
        return -1;
    other = (void *)(uintptr_t)PTRTAB(U16(obj, 0x1aa) & 0xff);
    if (mode == 1)
        return Sim_100c417e(obj, other, key, SIM_FN(W6hEntryFn, 0x10002964));
    if (mode == 2)
        return Sim_100c417e(obj, other, key, SIM_FN(W6hEntryFn, 0x10002513));
    return -1;
}

// FUNCTION: HGSIM 0x100c6c39
void __cdecl Tar_ReadHeader(const char *header, void *out, uint32_t *gnu)
{
    U32(out, 8) = SIM_CALL_100c6b30(8, header + 0x64);
    U32(out, 0x24) = SIM_CALL_100c6b30(0xc, header + 0x88);
    U32(out, 0x1c) = SIM_CALL_100c6b30(0xc, header + 0x7c);
    if (Str_Compare(header + 0x101, SIM_PTR(const char, 0x10151ce8)) == 0) {
        *gnu = 1;
        U32(out, 0) = 0;
    } else {
        *gnu = 0;
        U32(out, 0x10) = SIM_CALL_100c6b30(8, header + 0x6c);
        U32(out, 0x14) = SIM_CALL_100c6b30(8, header + 0x74);
        U32(out, 0) = 0;
    }
}

// FUNCTION: HGSIM 0x100cb8ea
int __cdecl Sim_100cb8ea(void *obj)
{
    if ((U32(obj, 0x118) & 0xf00) == 0x100 && SIM_CALL_100cab7b(U32(obj, 4), U32(obj, 0x118) & 0xff, 0) >= 0)
        return 1;
    U32(obj, 0xf0) |= 0x1000;
    U32(obj, 0x118) |= 0x1000;
    return 0;
}
