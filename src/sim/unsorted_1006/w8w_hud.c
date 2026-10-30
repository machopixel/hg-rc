#include "w8w.h"
#include "w5z.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100f/w2e.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x10062386
void __cdecl Sim_10062386(void)
{
    int32_t i;

    if (G32(0x1017692c) == 0)
        return;
    if (G32(0x1017b940) != 0) {
        for (i = 0; i < 8; i++) {
            SIM_PTR(u16a, 0x10176960)[i] = (uint16_t)W8W6_CALL_100f01ea(i);
            SIM_PTR(u16a, 0x10176b60)[i] = (uint16_t)W8W6_CALL_100f01ea(i + 8);
            SIM_PTR(u16a, 0x10176d60)[i] = (uint16_t)W8W6_CALL_100f01ea(i + 0x10);
            SIM_PTR(u16a, 0x10176f60)[i] = (uint16_t)W8W6_CALL_100f01ea(i + 0x18);
        }
        for (i = 8; i < 0x10; i++) {
            SIM_PTR(u16a, 0x10177160)[i] = (uint16_t)W8W6_CALL_100f01ea(i - 8);
            SIM_PTR(u16a, 0x10177360)[i] = (uint16_t)W8W6_CALL_100f01ea(i);
            SIM_PTR(u16a, 0x10177560)[i] = (uint16_t)W8W6_CALL_100f01ea(i + 8);
            SIM_PTR(u16a, 0x10177760)[i] = (uint16_t)W8W6_CALL_100f01ea(i + 0x10);
            SIM_PTR(u16a, 0x10177960)[i] = (uint16_t)W8W6_CALL_100f01ea(0);
        }
        for (i = 0x10; i < 0x18; i++) {
            SIM_PTR(u16a, 0x10177b60)[i] = (uint16_t)W8W6_CALL_100f01ea(i - 0x10);
            SIM_PTR(u16a, 0x10177d60)[i] = (uint16_t)W8W6_CALL_100f01ea(i - 8);
            SIM_PTR(u16a, 0x10177f60)[i] = (uint16_t)W8W6_CALL_100f01ea(i);
            SIM_PTR(u16a, 0x10178160)[i] = (uint16_t)W8W6_CALL_100f01ea(i + 8);
        }
        for (i = 0x18; i < 0x20; i++) {
            SIM_PTR(u16a, 0x10178360)[i] = (uint16_t)W8W6_CALL_100f01ea(i - 0x18);
            SIM_PTR(u16a, 0x10178560)[i] = (uint16_t)W8W6_CALL_100f01ea(i - 0x10);
            SIM_PTR(u16a, 0x10178760)[i] = (uint16_t)W8W6_CALL_100f01ea(i - 8);
            SIM_PTR(u16a, 0x10178960)[i] = (uint16_t)W8W6_CALL_100f01ea(i);
        }
        return;
    }
    for (i = 0; i < 8; i++) {
        GBYTES(0x10178b60)[i] = (uint8_t)i;
        GBYTES(0x10178c60)[i] = (uint8_t)(i + 8);
        GBYTES(0x10178d60)[i] = (uint8_t)(i + 0x10);
        GBYTES(0x10178e60)[i] = (uint8_t)(i + 0x18);
    }
    for (i = 8; i < 0x10; i++) {
        GBYTES(0x10178f60)[i] = (uint8_t)(i - 8);
        GBYTES(0x10179060)[i] = (uint8_t)i;
        GBYTES(0x10179160)[i] = (uint8_t)(i + 8);
        GBYTES(0x10179260)[i] = (uint8_t)(i + 0x10);
        GBYTES(0x10179360)[i] = 0;
    }
    for (i = 0x10; i < 0x18; i++) {
        GBYTES(0x10179460)[i] = (uint8_t)(i - 0x10);
        GBYTES(0x10179560)[i] = (uint8_t)(i - 8);
        GBYTES(0x10179660)[i] = (uint8_t)i;
        GBYTES(0x10179760)[i] = (uint8_t)(i + 8);
    }
    for (i = 0x18; i < 0x20; i++) {
        GBYTES(0x10179860)[i] = (uint8_t)(i - 0x18);
        GBYTES(0x10179960)[i] = (uint8_t)(i - 0x10);
        GBYTES(0x10179a60)[i] = (uint8_t)(i - 8);
        GBYTES(0x10179b60)[i] = (uint8_t)i;
    }
}

// FUNCTION: HGSIM 0x10062abd
void __cdecl Sim_10062abd(uint8_t *obj)
{
    /* laid out as in the original: two angles, then the vector of three that is projected in place */
    float angles[5];
    uint8_t *entry = BYTES(U32(obj, 0x84) + U32(obj, 0x4fc) * 0x128);
    uint32_t flags = U32(U32(obj, 0), 0xf0);

    if ((flags & 0xf00) != 0x200 || (U32(U32(obj, 0), 0xf0) & 0x1000) != 0 || U32(entry, 8) != 1)
        return;
    W8W6_CALL_1000c1ea(U32(obj, 0), &angles[0], &angles[1], &angles[2]);
    /* the original truncates the projected x and y into locals that it never reads */
    W8W6_CALL_100d2669(&angles[2], &angles[3], &angles[4]);
}

/* (int)value / 2, rounded toward zero like the original's cdq / sub / sar. */
static int32_t Half(uint32_t value)
{
    return (int32_t)value / 2;
}

// FUNCTION: HGSIM 0x10063f00
void __cdecl Sim_10063f00(uint8_t *obj, uint32_t unused, uint32_t unused2, float a, float b, float c)
{
    uint32_t display = G32(0x109e8940);
    uint32_t surface = U32(display, 0xec);
    int32_t x, y;

    (void)unused;
    (void)unused2;
    if (!(U32(U32(obj, 0), 0xf0) & 0x100))
        a = b;
    if (U32(obj, 0x50c) != 2) {
        if ((U32(U32(obj, 0), 0xf0) & 0xf00) == 0 || (U32(U32(obj, 0), 0xf0) & 0x1000) != 0)
            return;
        if ((double)c > (double)GFL(0x1012b5c8))
            Sim_1006535f(G32(0x10144ed0), G32(0x10144ed4) - G32(0x10166904) - G32(0x10176920), 0x10144ffc,
                         (const void *)(uintptr_t)surface);
        if (!((double)c >= (double)GFL(0x1012b5cc)))
            Sim_1006535f(G32(0x10144ed0), G32(0x10144ed4) + G32(0x10166904) + G32(0x10176928), 0x10145004,
                         (const void *)(uintptr_t)surface);
    }
    if (!(W8W6_FABS((double)a) >= (double)GFL(0x1012b5d0)))
        Sim_100652e8(G32(0x10144ed0), G32(0x10144ed4), (char *)GBYTES(0x1014500c), surface);
    else
        Sim_100652e8(G32(0x10144ed0) + (uint32_t)Render_Ftol((double)GFL(0x1017691c) * (double)a), G32(0x10144ed4),
                     (char *)GBYTES(0x10145014), surface);
    if (!(W8W6_FABS((double)a) >= (double)GFL(0x1012b5d0)))
        Sim_100652e8(G32(0x10144ed0), G32(0x10144ed4), (char *)GBYTES(0x1014501c), surface);
    if ((double)a > (double)GFL(0x1012b5c8)) {
        x = (int32_t)S16(display, 0x100) + (int32_t)G32(0x10166908) - 1;
        y = (int32_t)G32(0x10144ed4) + Half(G32(0x1016690c));
        Sim_1006535f((uint32_t)x, (uint32_t)y, 0x10145024, (const void *)(uintptr_t)surface);
        if ((double)a > (double)GFL(0x1012b5a8))
            Sim_1006535f((uint32_t)x + 1, (uint32_t)y, 0x1014502c, (const void *)(uintptr_t)surface);
    }
    if (!((double)a >= (double)GFL(0x1012b5cc))) {
        x = -(int32_t)G32(0x10166908);
        y = (int32_t)G32(0x10144ed4) + Half(G32(0x1016690c));
        Sim_1006535f((uint32_t)x, (uint32_t)y, 0x10145034, (const void *)(uintptr_t)surface);
        if (!((double)a >= (double)GFL(0x1012b5d4)))
            Sim_1006535f((uint32_t)x - 1, (uint32_t)y, 0x1014503c, (const void *)(uintptr_t)surface);
    }
}

// FUNCTION: HGSIM 0x1006419e
void __cdecl Sim_1006419e(uint32_t dest, uint32_t res, uint32_t a3, int32_t x, int32_t y, uint8_t *image, uint32_t a7,
                          uint32_t a8, uint32_t a9, uint32_t a10)
{
    /* laid out as in the original: the surface {bits, width, height} and the drawing record {surface, rectangle} */
    uint32_t surface[5];
    uint32_t record[5];

    if (image == 0)
        return;
    if (G32(0x1017b940) == 0) {
        W8W6_CALL_100e9cd1((void *)(uintptr_t)dest, res, a3, x, y, (uint32_t)(uintptr_t)image, a7, a8, a9, a10);
        return;
    }
    record[0] = (uint32_t)(uintptr_t)surface;
    surface[0] = (uint32_t)(uintptr_t)image;
    /* the size and offset of the resource are fetched but replaced by the fixed 0xb2 x 0xb2 and 0x59 below */
    W8W6_CALL_100f1f52(res, 0);
    W8W6_CALL_100f1f86(res, 0);
    Mem_Fill(image, 0, 0xfe01);
    image[0] = 0xf;
    record[1] = 0;
    record[2] = 0;
    record[3] = 0xb2;
    record[4] = 0xb2;
    surface[1] = 0xb2;
    surface[2] = 0xb2;
    W8W6_CALL_100e9cd1(record, res, a3, 0x59, 0x59, 0x10166910, a7, a8, a9, a10);
    W8W6_CALL_100ea9f5(record, 0, 0, 0, 0x10166910);
    W8W6_CALL_100f0205(dest, 0x10166910, 0, (uint32_t)(x - 0x59), (uint32_t)(y - 0x59));
}

// FUNCTION: HGSIM 0x10064824
int __cdecl Sim_10064824(uint32_t *pobj, int32_t *outX, int32_t *outY)
{
    uint32_t ray[0x10];      /* the original's record is 0x38 bytes */
    float x, y, z;
    int visible;

    W8W6_CALL_100d6f16(*pobj, ray);
    W8W6_CALL_1009cd36(ray, (float)W8W6_CALL_100d6e99(*pobj));
    x = Float_OfBits(ray[3]);
    y = Float_OfBits(ray[4]);
    z = Float_OfBits(ray[5]);
    visible = W8W6_CALL_100d2669(&x, &y, &z);
    *outX = Render_Ftol((double)x);
    *outY = Render_Ftol((double)y);
    return visible;
}

// FUNCTION: HGSIM 0x10065131
void __cdecl Sim_10065131(void)
{
    uint32_t block = U32(PTRTAB(G32(0x1017b8c8)), 0x28);
    int32_t y, x;

    if (!(U32(block, 0xbcb) & 4))
        return;
    if (W8W6_CALL_100d2669(SIM_PTR(float, 0x1017693c), SIM_PTR(float, 0x10176940), SIM_PTR(float, 0x10176944)) == 0)
        return;
    y = Render_Ftol((double)GFL(0x10176940));
    x = Render_Ftol((double)GFL(0x1017693c));
    Sim_1006519d((uint32_t)x, (uint32_t)y, (char *)(uintptr_t)G32(0x109e85c0));
}

// FUNCTION: HGSIM 0x1006519d
uint32_t __cdecl Sim_1006519d(uint32_t x, uint32_t y, char *name)
{
    uint32_t data = W8W6_CALL_100ba7d5(name, G32(0x1014cec0), 0);

    if (data == 0)
        return 0;
    if (G32(0x1017b940) != 0)
        W8W6_CALL_100f0205(0x109e3e00, data, 0, x, y);
    else
        W8W6_CALL_100e91dc(0x109e3e00, data, 0, x, y);
    return (uint32_t)Sim_100bb2b0((const uint32_t *)name, (const char *)(uintptr_t)G32(0x1014cec0));
}

// FUNCTION: HGSIM 0x10065216
uint32_t __cdecl Sim_10065216(uint32_t x, uint32_t y, char *name, uint32_t palette)
{
    uint32_t data = W8W6_CALL_100ba7d5(name, G32(0x1014cec0), 0);

    if (data == 0)
        return 0;
    if (G32(0x1017692c) != 0) {
        if (G32(0x1017b940) != 0)
            W8W6_CALL_100f01ad(palette);
        else
            W8W6_CALL_100e96a7(palette);
        if (G32(0x1017b940) != 0)
            W8W6_CALL_100f0205(0x109e3e00, data, 0, x, y);
        else
            W8W6_CALL_100e96c6(0x109e3e00, data, 0, x, y);
        if (G32(0x1017b940) != 0)
            W8W6_CALL_100f01ad(0x101659e0);
        else
            W8W6_CALL_100e96a7(0x101659e0);
    }
    return (uint32_t)Sim_100bb2b0((const uint32_t *)name, (const char *)(uintptr_t)G32(0x1014cec0));
}

// FUNCTION: HGSIM 0x100652e8
uint32_t __cdecl Sim_100652e8(uint32_t x, uint32_t y, char *name, uint32_t dest)
{
    uint32_t data = W8W6_CALL_100ba7d5(name, G32(0x1014cec0), 0);

    if (data == 0)
        return 0;
    if (G32(0x1017b940) != 0)
        W8W6_CALL_100f0205(dest, data, 0, x, y);
    else
        W8W6_CALL_100e91dc(dest, data, 0, x, y);
    return (uint32_t)Sim_100bb2b0((const uint32_t *)name, (const char *)(uintptr_t)G32(0x1014cec0));
}
