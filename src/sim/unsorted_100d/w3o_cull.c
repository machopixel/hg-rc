#include "w3o.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x100d2a7b
uint32_t __cdecl Sim_100d2a7b(void *obj)
{
    float radius = FL(obj, 0x50);
    float dx = (float)((double)FL(obj, 0x44) - GFL(0x10185c48));
    float dy = (float)((double)FL(obj, 0x48) - GFL(0x10185c44));
    float dz = (float)((double)FL(obj, 0x4c) - GFL(0x10185c4c));
    float depth, w, u, k, limit;

    if (U16(obj, 0) & 0x1000)
        return 1;
    GFL(0x101c7aa0) = (float)(((double)dx * GFL(0x10185c38) + (double)dy * GFL(0x10185c3c)) + (double)dz * GFL(0x10185c40));
    depth = GFL(0x101c7aa0);
    if (!((double)depth + radius >= GFL(0x10185c50)))
        return 4;
    w = (float)(((double)dx * GFL(0x10185c20) + (double)dy * GFL(0x10185c24)) + (double)dz * GFL(0x10185c28));
    k = FL(G32(0x1014cae0), 0x18);
    if (!(Bits_OfFloat(w) & 0x80000000u))
        limit = (float)(((double)k * w + -(double)depth) / FL(G32(0x1014cae0), 0xa8));
    else
        limit = (float)(((double)-w * k + -(double)depth) / FL(G32(0x1014cae0), 0xa8));
    if (limit > radius)
        return 6;
    u = (float)(((double)dx * GFL(0x10185c2c) + (double)dy * GFL(0x10185c30)) + (double)dz * GFL(0x10185c34));
    k = FL(G32(0x1014cae0), 0xb0);
    if (!(Bits_OfFloat(w) & 0x80000000u))
        limit = (float)(((double)k * u + -(double)depth) / FL(G32(0x1014cae0), 0xac));
    else
        limit = (float)(((double)-u * k + -(double)depth) / FL(G32(0x1014cae0), 0xac));
    if (limit > radius)
        return 7;
    return 0;
}
