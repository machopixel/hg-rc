#include "w3o.h"
#include "../unsorted_1008/w3o.h"
#include "../util/util.h"
#include "../util/w3o.h"

#define RECORD(index) GBYTES(0x109eeb00u + (uint32_t)(index) * 0x30)

/* (random int) / (A * B) * C, the way the first three fields are made. */
static float Spread(int32_t r)
{
    return (float)((double)r / ((double)GFL(0x10111284) * GFL(0x1013abc4)) * GFL(0x10111288));
}

/* (random int) * C / A, the way the next three are made. */
static float Unit(int32_t r)
{
    return (float)((double)r * GFL(0x1011128c) / GFL(0x10111284));
}

// FUNCTION: HGSIM 0x10025dc3
void __cdecl Sim_10025dc3(void)
{
}

// FUNCTION: HGSIM 0x10028c6f
void __cdecl Sim_10028c6f(int index)
{
    uint8_t *rec = RECORD(index);
    uint32_t owner;

    if (U32(rec, 4) == 0)
        return;
    owner = U32(U32(rec, 4), 0x8c);
    FL(rec, 8) = Spread((int32_t)Sim_1009c969());
    FL(rec, 0x10) = Spread((int32_t)Sim_1009c969());
    FL(rec, 0xc) = Spread((int32_t)(Sim_1009c969() + 0x400));
    FL(rec, 0x14) = Unit((int32_t)Sim_1009c969());
    FL(rec, 0x18) = Unit((int32_t)Sim_1009c969());
    FL(rec, 0x1c) = Unit((int32_t)Sim_1009c969());
    if (owner != 0) {
        U32(rec, 0x24) = U32(owner, 0x44);
        FL(rec, 0x28) = (float)((double)FL(owner, 0x48) + FL(owner, 0x3c));
        U32(rec, 0x2c) = U32(owner, 0x4c);
    }
}

// FUNCTION: HGSIM 0x1002977e
void __cdecl Sim_1002977e(int index, float x, float y, float z)
{
    uint8_t *rec = RECORD(index);
    double len1;
    float len1F, len2F, scale;
    double len2;

    if (U32(rec, 4) == 0)
        return;
    len1 = Sim_10084a94(x, y, z);
    len1F = (float)len1;
    if (!(len1 < GFL(0x1011129c) || len1 > GFL(0x1011129c)))
        return;
    Sim_10028c6f(index);
    len2 = Sim_10084a94(FL(rec, 8), FL(rec, 0xc), FL(rec, 0x10));
    len2F = (float)len2;
    if (len2 > len1F) {
        scale = (float)((double)len1F * GFL(0x10111290) / len2F);
        FL(rec, 8) = (float)((double)FL(rec, 8) * scale);
        FL(rec, 0xc) = (float)((double)FL(rec, 0xc) * scale);
        FL(rec, 0x10) = (float)((double)FL(rec, 0x10) * scale);
    }
    FL(rec, 8) = (float)((double)FL(rec, 8) + x);
    FL(rec, 0xc) = (float)((double)FL(rec, 0xc) + y);
    FL(rec, 0x10) = (float)((double)FL(rec, 0x10) + z);
}
