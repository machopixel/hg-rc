#include "w3o.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_1009/w3o.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x10008ed0
void __cdecl Sim_10008ed0(void *obj, float value)
{
    uint32_t target = U32(obj, 0x30);

    if (target != 0)
        U16(target, 0x14) = (uint16_t)Render_Ftol(value);
}

// FUNCTION: HGSIM 0x1000946e
float __cdecl Sim_1000946e(void *obj, uint32_t *x, uint32_t *y, uint32_t *z)
{
    if (obj == 0)
        return GFL(0x10111018);
    if (x != 0)
        *(u32a *)x = U32(obj, 0x44);
    if (y != 0)
        *(u32a *)y = U32(obj, 0x48);
    if (z != 0)
        *(u32a *)z = U32(obj, 0x4c);
    return FL(obj, 0x50);
}

// FUNCTION: HGSIM 0x10009ca8
double __cdecl Sim_10009ca8(void *obj, float x, float y, float z)
{
    float dx = (float)((double)FL(obj, 0x44) - x);
    float dy = (float)((double)FL(obj, 0x48) - y);
    float dz = (float)((double)FL(obj, 0x4c) - z);
    float r = FL(obj, 0x50);

    if (dx > r || -r > dx || dy > r || -r > dy || dz > r || -r > dz)
        return GFL(0x1011101c);
    {
        double left = (double)r * r - (((double)dz * dz + (double)dx * dx) + (double)dy * dy);

        if (!(left > GFL(0x10111018)))
            return GFL(0x1011101c);
    }
    return Sim_10084a94(dx, dy, dz);
}

/*
 * The two ray tests are the same code on different sphere fields: the
 * centre/radius offsets are the only difference.
 */
static float RayHitSphere(void *obj, void *ray, uint32_t cx, uint32_t cy, uint32_t cz, uint32_t cr)
{
    float r = FL(obj, cr);
    float dx, dy, dzf, c, b, q, t;
    double dzu;

    if (!(r >= GFL(0x10111020)))
        return GFL(0x1011101c);
    dx = (float)((double)FL(obj, cx) - FL(ray, 0));
    dy = (float)((double)FL(obj, cy) - FL(ray, 4));
    dzu = (double)FL(obj, cz) - FL(ray, 8);
    dzf = (float)dzu;
    c = (float)(((dzu * dzf + (double)dx * dx) + (double)dy * dy) - (double)r * r);
    if (Bits_OfFloat(c) & 0x80000000u)
        return GFL(0x10111024);
    b = (float)(((double)dx * FL(ray, 0x18) + (double)dy * FL(ray, 0x1c)) + (double)dzf * FL(ray, 0x20));
    if (Bits_OfFloat(b) & 0x80000000u)
        return GFL(0x1011101c);
    {
        double quotient = (double)b / FL(ray, 0x30);
        float quotientF = (float)quotient;

        b = quotientF;
        q = (float)(quotient * quotientF - c);
    }
    if (Bits_OfFloat(q) & 0x80000000u)
        return GFL(0x1011101c);
    t = (float)((double)b - r);
    if (Bits_OfFloat(t) & 0x80000000u)
        return GFL(0x10111018);
    if (!(Sim_1009cbde(ray) > t))
        return Float_OfBits(0x7f7fc99eu);
    return t;
}

// FUNCTION: HGSIM 0x10009d7e
float __cdecl Sim_10009d7e(void *obj, void *ray)
{
    return RayHitSphere(obj, ray, 0x60, 0x64, 0x68, 0x6c);
}

// FUNCTION: HGSIM 0x10009eb3
float __cdecl Sim_10009eb3(void *obj, void *ray)
{
    return RayHitSphere(obj, ray, 0x44, 0x48, 0x4c, 0x50);
}
