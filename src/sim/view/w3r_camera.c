#include "w3r.h"
#include "../util/w3r.h"
#include "../render/w2n.h"

// FUNCTION: HGSIM 0x100d2306
void __cdecl Sim_100d2306(uint8_t *cam)
{
    float fov = F32(cam, 0x18);
    float aspect = F32(cam, 0x44);
    int32_t left = S32(cam, 0x2c), right = S32(cam, 0x30), top = S32(cam, 0x34), bottom = S32(cam, 0x38);
    int32_t half_w, half_h, i1, i2;
    float scale_y, scale_x, g;

    S32(cam, 0x8c) = (int32_t)((uint32_t)left + (uint32_t)right + 1u) >> 1;
    S32(cam, 0x90) = (int32_t)((uint32_t)top + (uint32_t)bottom + 1u) >> 1;
    half_w = (int32_t)((uint32_t)right - (uint32_t)left + 1u) >> 1;
    if (half_w <= 1)
        half_w = 1;
    S32(cam, 0x84) = half_w;
    half_h = (int32_t)((uint32_t)bottom - (uint32_t)top + 1u) >> 1;
    if (half_h <= 1)
        half_h = 1;
    S32(cam, 0x88) = half_h;

    /* a NaN stays as it is in the first test and becomes 0.5 in the second */
    if (fov > GF32(0x1012bcb8))
        fov = 16.0f;
    if (!(fov >= GF32(0x1012bcbc)))
        fov = 0.5f;

    i1 = Render_Ftol((double)fov * (double)GF32(0x1012bcb8));
    g = (float)((double)fov * (double)aspect * (double)half_w / (double)half_h);
    F32(cam, 0xb0) = g;
    i2 = Render_Ftol((double)g * (double)GF32(0x1012bcb8));
    if (i1 > 0x31f)
        i1 = 0x31f;
    if (i2 > 0x31f)
        i2 = 0x31f;
    F32(cam, 0xa8) = (float)((double)fov * (double)SIM_PTR(f32a, 0x1036f860)[i1] + (double)SIM_PTR(f32a, 0x10370d20)[i1]);
    F32(cam, 0xac) = (float)((double)g * (double)SIM_PTR(f32a, 0x1036f860)[i2] + (double)SIM_PTR(f32a, 0x10370d20)[i2]);

    scale_x = (float)((double)half_w * (double)fov * (double)aspect);
    scale_y = (float)((double)half_w * (double)fov);
    F32(cam, 0x3c) = (float)((double)half_w * (double)GF32(0x1012bcbc) + (double)GF32(0x1012bcac));
    F32(cam, 0x94) = scale_y;
    F32(cam, 0x98) = scale_x;
    if (GS32(0x10152418) <= 0)
        GS32(0x10152418) = 1;
    F32(cam, 0xb8) = (float)((double)scale_y / ((double)GS32(0x10152418) * (double)GF32(0x1012bcc0)));
}

// FUNCTION: HGSIM 0x100d280d
int __cdecl Sim_100d280d(const uint8_t *obj)
{
    float x, y, z, r, dx, dy, dz, depth, far_, t, u, side, w;
    const uint8_t *cam;
    double d2, far2;

    if (GS32(0x1015ef0c) != 0 && (U16(obj, 2) & 0xf0) == 0xa0)
        return 1;
    x = F32(obj, 0x44);
    y = F32(obj, 0x48);
    z = F32(obj, 0x4c);
    r = F32(obj, 0x50);
    if ((int8_t)U8(obj, 0xa0) == 0x57)
        r = (float)((double)r * (double)GF32(0x1012bca4));
    dx = (float)((double)x - (double)GF32(0x10185c48));
    dy = (float)((double)y - (double)GF32(0x10185c44));
    dz = (float)((double)z - (double)GF32(0x10185c4c));
    cam = (const uint8_t *)(uintptr_t)G32(0x1014cae0);
    far_ = (float)((double)F32(cam, 0x40) + (double)r);
    d2 = (double)dx * (double)dx + (double)dy * (double)dy + (double)dz * (double)dz;
    far2 = (double)far_ * (double)far_;
    if (!(far2 >= d2))
        return 5;

    depth = (float)((double)dx * (double)GF32(0x10185c38) + (double)dy * (double)GF32(0x10185c3c) + (double)dz * (double)GF32(0x10185c40));
    GF32(0x101c7aa0) = depth;
    t = GF32(0x101c7aa0);
    if (!((double)t + (double)r >= (double)GF32(0x10185c50)))
        return 4;
    if ((double)t - (double)r > (double)GF32(0x10185bf8))
        return 5;

    u = (float)((double)dx * (double)GF32(0x10185c20) + (double)dy * (double)GF32(0x10185c24) + (double)dz * (double)GF32(0x10185c28));
    side = F32(cam, 0x18);
    cam = (const uint8_t *)(uintptr_t)G32(0x1014cae0);
    if ((U32(&u, 0) & 0x80000000u) == 0)
        w = (float)(((double)side * (double)u + -(double)t) / (double)F32(cam, 0xa8));
    else
        w = (float)((-(double)u * (double)side + -(double)t) / (double)F32(cam, 0xa8));
    if ((double)w > (double)r)
        return 6;

    u = (float)((double)dx * (double)GF32(0x10185c2c) + (double)dy * (double)GF32(0x10185c30) + (double)dz * (double)GF32(0x10185c34));
    cam = (const uint8_t *)(uintptr_t)G32(0x1014cae0);
    side = F32(cam, 0xb0);
    if ((U32(&u, 0) & 0x80000000u) == 0)
        w = (float)(((double)side * (double)u + -(double)t) / (double)F32(cam, 0xac));
    else
        w = (float)((-(double)u * (double)side + -(double)t) / (double)F32(cam, 0xac));
    return (double)w > (double)r ? 7 : 0;
}
