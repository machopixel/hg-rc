#include "w3s.h"
#include "fns.h"
#include "../render/w3s_fp.h"

#define ENTRY_COUNT GS32(0x109e4cc0)
#define ENTRY(i) (GBYTES(0x109e4cc4) + (uint32_t)(i) * 0x44u)

// FUNCTION: HGSIM 0x100335fd
double __cdecl Entry_DistSq(const void *entry, float x, float y, float z)
{
    float dx, dy, dz;

    if (entry == 0)
        return (double)GF32(0x1011131c);
    dx = (float)((double)F32(entry, 0x20) - (double)x);
    dy = (float)((double)F32(entry, 0x24) - (double)y);
    dz = (float)((double)F32(entry, 0x28) - (double)z);
    return ((double)dx * (double)dx + (double)dy * (double)dy) + (double)dz * (double)dz;
}

// FUNCTION: HGSIM 0x1003366e
int __cdecl Sim_1003366e(const void *entry, float ax, float ay, float az, float bx, float by, float bz)
{
    float px = F32(entry, 0x20), py = F32(entry, 0x24), pz = F32(entry, 0x28);
    float d1x = (float)((double)ax - (double)px);
    float d1y = (float)((double)ay - (double)py);
    float d1z = (float)((double)az - (double)pz);
    float d2x = (float)((double)bx - (double)px);
    float d2y = (float)((double)by - (double)py);
    float d2z = (float)((double)bz - (double)pz);
    float dot = (float)(((double)d1x * (double)d2x + (double)d1y * (double)d2y) + (double)d1z * (double)d2z);

    return !(dot >= GF32(0x1011131c)) ? 1 : 2;
}

// FUNCTION: HGSIM 0x10033d75
int __cdecl Sim_10033d75(float x, float y, float z)
{
    int32_t count = ENTRY_COUNT;
    int32_t best = 0;
    int32_t j;
    uint8_t *entry;
    float bestDist;

    if (count == 0)
        return -1;
    entry = GBYTES(0x109e4cc4);
    while (Sim_1003356a(entry) != 0 && best < count) {
        entry += 0x44;
        best++;
    }
    bestDist = (float)Entry_DistSq(entry, x, y, z);
    entry += 0x44;
    for (j = best; j < count; j++) {
        if (Sim_1003356a(entry) == 0) {
            double d = Entry_DistSq(entry, x, y, z);
            float f = (float)d;

            if (!(d >= (double)bestDist)) {
                bestDist = f;
                best = j;
            }
        }
        entry += 0x44;
    }
    return best;
}

// FUNCTION: HGSIM 0x10034036
int __cdecl Sim_10034036(const int32_t *list, int n, float x, float y, float z)
{
    const int32_t *cur = list;
    int32_t best;
    float bestDist;
    int i;

    if (list == 0 || n <= 0)
        return -1;
    if (n == 1)
        return list[0];
    best = list[0];
    bestDist = (float)Entry_DistSq(ENTRY(best), x, y, z);
    for (i = 1; i < n; i++) {
        double d = Entry_DistSq(ENTRY(*cur), x, y, z);
        float f = (float)d;

        if (!(d >= (double)bestDist)) {
            bestDist = f;
            best = *cur;
        }
        cur++;
    }
    return best;
}
