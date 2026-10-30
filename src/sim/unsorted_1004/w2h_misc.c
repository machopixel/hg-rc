#include "w2h.h"
#include "fns.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100429d5
uint32_t __cdecl Sim_100429d5(void)
{
    uint32_t rec;
    uint32_t offset;

    if (GS32(0x101655ac) == -1)
        return 0;
    rec = PTRTAB(G32(0x101655ac));
    offset = U32(rec, 8) * 0x6c;
    return (offset & 0xffffff00u) | (G32(0x109b7d54 + offset) == 0 ? 2 : 3);
}

// FUNCTION: HGSIM 0x10047e95
int32_t __cdecl Sim_10047e95(int32_t *p)
{
    int32_t delta = (int32_t)((uint32_t)p[1] - (uint32_t)p[0]);

    p[0] = (int32_t)((uint32_t)p[0] + (uint32_t)(delta >> (S16(p, 8) & 31)));
    return p[0];
}

// FUNCTION: HGSIM 0x10048bd6
void __cdecl Sim_10048bd6(uint8_t *p)
{
    int found = 0;
    int i;

    for (i = 0; i < 0xc; i += 2) {
        if (*p == U8(GBYTES(0x10144220), i)) {
            *p = U8(GBYTES(0x10144221), i);
            found = 1;
            break;
        }
    }
    if (!found) {
        if (*p >= 0x2b && *p <= 0x3c)
            *p = U8(GBYTES(0x101441dd), *p);
        else if (*p >= 0x5b && *p <= 0x5d)
            *p = (uint8_t)(*p + 0x20);
    }
}

// FUNCTION: HGSIM 0x1004bfa9
int __cdecl Sim_1004bfa9(int word)
{
    return (word >> 12) & 1;
}

// FUNCTION: HGSIM 0x1004c221
int __cdecl Sim_1004c221(int a, int b)
{
    uint32_t sum = G32(0x101116a6 + (uint32_t)a * 0xaa);

    sum += G32(0x10124eac + (uint32_t)Sim_1004bf79((unsigned)b) * 0x98);
    sum += G32(0x10124eac + (uint32_t)Sim_1004bf87(b) * 0x98);
    sum += G32(0x10124eac + (uint32_t)Sim_1004bf98(b) * 0x98);
    if (Sim_1004bfa9(b) != 0)
        sum += G32(0x10126cf4);
    return (int)sum;
}
