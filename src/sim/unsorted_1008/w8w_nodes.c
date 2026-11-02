#include "w8w.h"
#include "../render/w7o.h"
#include "../unsorted_1002/w3o.h"
#include "../unsorted_100f/w6h.h"
#include "../util/w3o.h"
#include "../crt/hg_winheap.h"

// FUNCTION: HGSIM 0x10086e27
int32_t __cdecl Sim_10086e27(const char *name, uint8_t *rec)
{
    int32_t done = 0;
    void *file = Sim_100fd850(name, (const char *)GBYTES(0x1014b6f0));

    if (file != 0) {
        W8W8_FREAD(rec, 0x66, 1, file);
        if (U32(rec, 4) != 0) {
            U32(rec, 0x10) = (uint32_t)(uintptr_t)Hg_HeapAlloc(G32(0x1017b8f8), 1,
                                                                                        U32(rec, 4) * 0x1e);
            W8W8_FREAD((void *)(uintptr_t)U32(rec, 0x10), 0x1e, U32(rec, 4), file);
            done = 1;
        }
        W8W8_FCLOSE(file);
    }
    return done;
}

/*
 * The common part of Sim_10088e37 and Sim_10089325: centre at obj+centre, result at obj+out, flag bit in the
 * word +0; `self` is the function itself (the children are measured with it).
 */
static float NodeRadius(uint8_t *obj, uint32_t centre, uint32_t out, uint32_t flag, int scaleW,
                        float(__cdecl *self)(uint8_t *))
{
    uint8_t *block;
    uint32_t node;
    uint8_t *child;
    float cx, cy, cz, dx, dy, dz, best, radius;
    double sum;

    if (obj == 0)
        return GFL(0x1012b8ec);
    block = BYTES(U32(obj, 0x28));
    if (block == 0) {
        U32(obj, out) = U32(obj, 0x50);
        if (U16(obj, 0) & flag)
            return GFL(0x1012b8ec);
        return FL(obj, out);
    }
    if (U32(block, 4) == 0) {
        U32(obj, out) = U32(obj, 0x50);
        if (U16(obj, 0) & flag)
            return GFL(0x1012b8ec);
    } else {
        cx = FL(obj, centre);
        cy = FL(obj, centre + 4);
        cz = FL(obj, centre + 8);
        if (U16(obj, 0) & flag) {
            best = Float_OfBits(0);
        } else {
            dx = (float)((double)FL(obj, 0x44) - (double)cx);
            dy = (float)((double)FL(obj, 0x48) - (double)cy);
            dz = (float)((double)FL(obj, 0x4c) - (double)cz);
            best = (float)(W8W8_SQRT((double)dx * (double)dx + (double)dy * (double)dy + (double)dz * (double)dz) +
                           (double)FL(obj, 0x50));
        }
        for (node = U32(block, 4); node != 0; node = U32(node, 8)) {
            child = BYTES(U32(node, 0x8c));
            if (child == 0)
                continue;
            radius = self(child);
            if (!((double)radius > (double)GFL(0x1012b8f0)))
                continue;
            dx = (float)((double)FL(child, centre) - (double)cx);
            dy = (float)((double)FL(child, centre + 4) - (double)cy);
            dz = (float)((double)FL(child, centre + 8) - (double)cz);
            sum = W8W8_SQRT((double)dx * (double)dx + (double)dy * (double)dy + (double)dz * (double)dz) +
                  (double)radius;
            if (sum > (double)best)
                best = (float)sum;
        }
        FL(obj, out) = best;
    }
    if (scaleW && (int8_t)U8(obj, 0xa0) == 0x57)
        FL(obj, out) = (float)((double)FL(obj, out) * (double)GFL(0x1012b8f4));
    return FL(obj, out);
}

// FUNCTION: HGSIM 0x10088e37
float __cdecl Sim_10088e37(uint8_t *obj)
{
    return NodeRadius(obj, 0x7c, 0x88, 0x1000, 1, Sim_10088e37);
}

// FUNCTION: HGSIM 0x10089325
float __cdecl Sim_10089325(uint8_t *obj)
{
    return NodeRadius(obj, 0x60, 0x6c, 0x800, 0, Sim_10089325);
}

// FUNCTION: HGSIM 0x100897c7
void __cdecl Sim_100897c7(uint8_t *obj)
{
    float a, b, c;

    if (obj == 0)
        return;
    /* the original first adds a constant to an uninitialised local when the byte +0xa0 is 'g'; that local is
       overwritten by the call below, so the addition has no effect */
    if (!(U32(obj, 0x94) & 2)) {
        W8W8_CALL_10089066(obj, &a, &b, &c);
        Sim_10088e37(obj);
        U32(obj, 0x94) |= 2;
    }
    if (!(U32(obj, 0x94) & 1)) {
        W8W8_CALL_1008950e(obj, &a, &b, &c);
        Sim_10089325(obj);
        U32(obj, 0x94) |= 1;
    }
}

/* The cell record of the 9 x 9 grid at 0x1017ba50 (0x18c bytes per column, 0x2c per row). */
static uint32_t GridCell(int32_t column, int32_t row)
{
    return G32(0x1017ba50 + (uint32_t)column * 0x18c + (uint32_t)row * 0x2c);
}

// FUNCTION: HGSIM 0x10089bad
void __cdecl Sim_10089bad(void)
{
    int32_t i, j;
    float x, y, z;
    uint32_t cell;
    uint8_t *camera;

    if (G32(0x1017c8c4) == 0 || G32(0x1017c8c0) == 0)
        return;
    for (i = 0; i < 9; i++) {
        for (j = 0; j < 9; j++) {
            x = (float)((double)i * (double)GFL(0x1012b8e0) - (double)GFL(0x1014b8e0) + (double)GFL(0x1012b8fc));
            z = (float)((double)j * (double)GFL(0x1012b8e0) - (double)GFL(0x1014b8e4) + (double)GFL(0x1012b8fc));
            camera = BYTES(G32(0x1014cae0));
            y = (float)Sim_1002d961(x, FL(camera, 4), z);
            Sim_10089907(x, Bits_OfFloat(y), z, Float_OfBits(0x46d90a00u), 0x1c);
            cell = GridCell(i, j);
            if (cell != 0) {
                x = FL(cell, 0x44);
                y = FL(cell, 0x48);
                z = FL(cell, 0x4c);
                Sim_10089907(x, Bits_OfFloat(y), z, Float_OfBits(0x447a0000u), 0x1c);
            }
        }
    }
    camera = BYTES(G32(0x1014cae0));
    i = Render_Ftol((double)Render_Ftol(((double)FL(camera, 0) + (double)GFL(0x1014b8e0)) / (double)GFL(0x1012b8e0)) +
                    (double)GFL(0x1012b8e4));
    camera = BYTES(G32(0x1014cae0));
    j = Render_Ftol((double)Render_Ftol(((double)FL(camera, 8) + (double)GFL(0x1014b8e4)) / (double)GFL(0x1012b8e0)) +
                    (double)GFL(0x1012b8e4));
    cell = GridCell(i, j);
    if (cell != 0) {
        x = FL(cell, 0x44);
        y = FL(cell, 0x48);
        z = FL(cell, 0x4c);
        Sim_10089907(x, Bits_OfFloat(y), z, Float_OfBits(0x447a0000u), 4);
    } else {
        x = (float)((double)i * (double)GFL(0x1012b8e0) - (double)GFL(0x1014b8e0) + (double)GFL(0x1012b8fc));
        z = (float)((double)j * (double)GFL(0x1012b8e0) - (double)GFL(0x1014b8e0) + (double)GFL(0x1012b8fc));
        camera = BYTES(G32(0x1014cae0));
        y = (float)Sim_1002d961(x, FL(camera, 4), z);
    }
    Sim_10089907(x, Bits_OfFloat(y), z, Float_OfBits(0x46d90a00u), 4);
    camera = BYTES(G32(0x1014cae0));
    Sim_10089907(FL(camera, 0), Bits_OfFloat(y), FL(camera, 8), Float_OfBits(0x447a0000u), 0x14);
    Sim_10089907(Float_OfBits(0x48f42400u), Bits_OfFloat(y), Float_OfBits(0x48f42400u), Float_OfBits(0x48f42400u),
                 0x14);
}
