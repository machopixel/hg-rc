#include "w6g.h"
#include "w2e.h"
#include "../util/w3o.h"
#include "../unsorted_1002/w5b.h"
#include "../unsorted_100c/w5x.h"
#include "../unsorted_100f/w2e.h"

// FUNCTION: HGSIM 0x100a2823
uint32_t __cdecl Sim_100a2823(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4,
                              uint32_t colour)
{
    uint32_t v[5 * 9];      /* five vertices of 0x24 bytes; the fields not written stay as they are */
    int xy[10];
    int i;

    xy[0] = x0; xy[1] = y0; xy[2] = x1; xy[3] = y1; xy[4] = x2;
    xy[5] = y2; xy[6] = x3; xy[7] = y3; xy[8] = x4; xy[9] = y4;
    for (i = 0; i < 5; i++) {
        v[i * 9 + 0] = Bits_OfFloat((float)xy[2 * i]);
        v[i * 9 + 1] = Bits_OfFloat((float)xy[2 * i + 1]);
        v[i * 9 + 4] = colour;
        v[i * 9 + 6] = 0xff000000u;
    }
    return (*SIM_PTR(W6gDrawFn, 0x1014cb4c))(5, v, colour);
}

// FUNCTION: HGSIM 0x100a5336
int __cdecl Sim_100a5336(uint32_t a, uint32_t b)
{
    int i;

    (void)a;
    W6G_CALL_100d1f90(b);
    W6G_CALL_10087d83(b);
    for (i = 0; SIM_PTR(u32a, 0x1036f2a0)[i] != 0; i++)
        W6G_CALL_100a4f03(SIM_PTR(u32a, 0x1036f2a0)[i]);
    return 0;
}

// FUNCTION: HGSIM 0x100a7880
uint32_t __cdecl Sim_100a7880(const char *name)
{
    Sim_10027df4();
    if (G32(0x1017d158) != 0)
        Sim_100c6e87((void *)(uintptr_t)G32(0x1017d158));
    Str_Copy((char *)GBYTES(0x101c6520), name);
    G32(0x1017d158) = W6G_CALL_100c6e27(name);
    return G32(0x1017d158);
}

// FUNCTION: HGSIM 0x100a7f8c
int __cdecl Sim_100a7f8c(int index, const char *key, uint32_t slot)
{
    uint32_t rec = (uint32_t)index * 0x14au;
    int16_t entry;
    uint32_t values, names;
    int32_t v;

    if ((slot & 0xffff) == 0)
        return -1;
    entry = (int16_t)Sim_100a7da3(index, key);
    if (entry < 0)
        return -1;
    if (G32(0x101c660a + rec + (uint32_t)entry * 8) == 0 && W6G_CALL_100a7e55(index) == -1)
        return -1;
    values = G32(0x101c660a + rec + (uint32_t)entry * 8) + 0x16;
    names = G32(0x101c65e6 + rec) + 0xe;
    v = (int32_t)(U32(values, (slot & 0xffff) * 8 + 4) - U16(names, (uint32_t)entry * 0x18 + 0x14));
    return v > 0 ? v : 0;
}
