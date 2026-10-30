#include "w3p.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1009cab0
void __cdecl Sim_1009cab0(float *box, float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
{
    box[0] = minX;
    box[1] = minY;
    box[2] = minZ;
    box[3] = maxX;
    box[4] = maxY;
    box[5] = maxZ;
    box[6] = (float)((double)maxX - (double)minX);
    box[7] = (float)((double)maxY - (double)minY);
    box[8] = (float)((double)maxZ - (double)minZ);
    U32(box, 0x30) = 0;
    U32(box, 0x2c) = 0;
    U32(box, 0x28) = 0;
    U32(box, 0x24) = 0;
    U32(box, 0x34) = 0;
}
