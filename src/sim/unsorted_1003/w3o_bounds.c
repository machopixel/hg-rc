#include "w3o.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x1003196f
int __cdecl Sim_1003196f(void *list, void *scene, float xLo, float xHi, float zLo, float zHi, uint32_t *yMin, uint32_t *yMax)
{
    int count = U16(list, 2);
    uint32_t first = (uint32_t)(uintptr_t)scene + 0x18;
    float minX = Float_OfBits(0x7f7fc99eu), minY = minX, minZ = minX;
    float maxX = Float_OfBits(0xff7fc99eu), maxY = maxX, maxZ = maxX;
    uint32_t minYBits = Bits_OfFloat(minY), maxYBits = Bits_OfFloat(maxY);
    int i;

    for (i = 0; i < count; i++) {
        uint32_t v = first + (uint32_t)U8(list, U32(list, 4) + (uint32_t)i) * 0x58;
        float x = FL(v, 0xc), y = FL(v, 0x10), z = FL(v, 0x14);

        if (!(x >= minX))
            minX = x;
        if (!(y >= minY)) {
            minY = y;
            minYBits = U32(v, 0x10);
        }
        if (!(z >= minZ))
            minZ = z;
        if (x > maxX)
            maxX = x;
        if (y > maxY) {
            maxY = y;
            maxYBits = U32(v, 0x10);
        }
        if (z > maxZ)
            maxZ = z;
    }
    if (minX > xHi || !(maxX >= xLo) || minZ > zHi || !(maxZ >= zLo))
        return 0;
    *(u32a *)yMin = minYBits;
    *(u32a *)yMax = maxYBits;
    return 1;
}
