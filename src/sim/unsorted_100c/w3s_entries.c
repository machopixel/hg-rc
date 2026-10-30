#include "w3s.h"
#include "../render/w3s_fp.h"
#include "../unsorted_1003/fns.h"
#include "../unsorted_1003/w3s.h"

// FUNCTION: HGSIM 0x100c3f0c
int __cdecl Sim_100c3f0c(const void *a, const void *b, const void *entry)
{
    float ax, ay, az, bx, by, bz;
    float dx, dy, dz, between, da, db;
    int side;

    if (a == 0 || b == 0 || entry == 0)
        return -1;
    ax = F32(a, 0x58);
    ay = F32(a, 0x5c);
    az = F32(a, 0x60);
    bx = F32(b, 0x58);
    by = F32(b, 0x5c);
    bz = F32(b, 0x60);
    dx = (float)((double)ax - (double)bx);
    dy = (float)((double)ay - (double)by);
    dz = (float)((double)az - (double)bz);
    between = (float)(((double)dx * (double)dx + (double)dy * (double)dy) + (double)dz * (double)dz);
    da = (float)Entry_DistSq(entry, ax, ay, az);
    db = (float)Entry_DistSq(entry, bx, by, bz);
    side = Sim_1003366e(entry, ax, ay, az, bx, by, bz);
    if (!(da >= between) && !(da >= db) && side == 1)
        return 1;
    return 0;
}

// FUNCTION: HGSIM 0x100c4265
int __cdecl Sim_100c4265(int a, int b, int unused, const void *obj)
{
    float da, db;
    const unsigned char *ea, *eb;

    (void)unused;
    if (a == -1)
        return b;
    ea = Sim_10033c42(a);
    eb = Sim_10033c42(b);
    da = (float)Entry_DistSq(ea, F32(obj, 0x58), F32(obj, 0x5c), F32(obj, 0x60));
    db = (float)Entry_DistSq(eb, F32(obj, 0x58), F32(obj, 0x5c), F32(obj, 0x60));
    return !(da >= db) ? a : b;
}

// FUNCTION: HGSIM 0x100c4323
int __cdecl Sim_100c4323(int a, int b, int unused, const void *obj)
{
    float da, db;
    const unsigned char *ea, *eb;

    (void)unused;
    if (a == -1)
        return b;
    ea = Sim_10033c42(a);
    eb = Sim_10033c42(b);
    da = (float)Entry_DistSq(ea, F32(obj, 0x58), F32(obj, 0x5c), F32(obj, 0x60));
    db = (float)Entry_DistSq(eb, F32(obj, 0x58), F32(obj, 0x5c), F32(obj, 0x60));
    return da > db ? a : b;
}
