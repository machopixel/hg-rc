#include "w7o.h"
#include "w5b.h"
#include "w5c.h"
#include "w5x.h"
#include "w5y.h"
#include "w5z.h"
#include "w6g.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../util/util.h"
#include "../view/w3r.h"
#include "../unsorted_1000/w3s.h"
#include "../unsorted_1002/w3o.h"
#include "../unsorted_1006/w6e.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_1010/w2g.h"

#define PTR(v) ((void *)(uintptr_t)(v))
#define FPTR(p, off) ((float *)(BYTES(p) + (off)))
#define CAMERA BYTES(G32(0x1014cae0))

/* The common end of the two turns. */
static void TurnDone(void *node)
{
    W7O_CALL_10060d2e(FPTR(node, 0xc), FPTR(node, 0x3c));
    U32(node, 0x94) |= 1;
    U32(node, 0x98) -= 1;
    if (U32(node, 0x98) == 0) {
        U32(node, 0x98) = G32(0x1014d1a8);
        G32(0x1014d1a8) = ((G32(0x1014d1a8) + 7) & 0x7f) + 0x40;
        Sim_100607d5(FPTR(node, 0xc));
    }
}

// FUNCTION: HGSIM 0x100a9ec6
void __cdecl Sim_100a9ec6(void *node, const float *m)
{
    Sim_100a9c25(node);
    Mat3_Multiply(FPTR(node, 0xc), m, FPTR(node, 0xc));
    TurnDone(node);
}

// FUNCTION: HGSIM 0x100a9f6e
void __cdecl Sim_100a9f6e(void *node, const float *m)
{
    Sim_100a9c25(node);
    Mat3x4_Multiply(FPTR(node, 0xc), m, FPTR(node, 0xc));
    TurnDone(node);
}

// FUNCTION: HGSIM 0x100aa64a
void __cdecl Sim_100aa64a(void *node)
{
    uint32_t obj, child;

    U32(node, 0x94) &= ~1u;
    if (U32(node, 0) != 0) {
        Sim_100a9c25(node);
        Mat3x4_Multiply(FPTR(U32(node, 0), 0x4c), FPTR(node, 0xc), FPTR(node, 0x4c));
    } else {
        Sim_100a9c25(node);
        Mem_Move(BYTES(node) + 0x4c, BYTES(node) + 0xc, 0x30);
    }
    U32(node, 0x94) |= 0x8000;
    obj = U32(node, 0x8c);
    if (obj != 0) {
        Sim_10009a53(PTR(obj), FPTR(node, 0x4c));
        U32(obj, 0x28) = (uint32_t)(uintptr_t)node;
    }
    for (child = U32(node, 4); child != 0; child = U32(child, 8))
        Sim_100aa64a(PTR(child));
}

// FUNCTION: HGSIM 0x100aaa08
void __cdecl Sim_100aaa08(void *node, void *parent)
{
    float inverse[12];

    if (U32(node, 0) != 0)
        Sim_100aa8cb(node);
    U32(node, 0) = (uint32_t)(uintptr_t)parent;
    U32(node, 8) = U32(parent, 4);
    U32(parent, 4) = (uint32_t)(uintptr_t)node;
    Sim_100a9ba7(parent);
    Mat3x4_InvertRigid(FPTR(parent, 0x4c), inverse);
    Sim_100a9c25(node);
    Mat3x4_Multiply(inverse, FPTR(node, 0x4c), FPTR(node, 0xc));
    W7O_CALL_10060d2e(FPTR(node, 0xc), FPTR(node, 0x3c));
    U32(node, 0x94) |= 0x4000;
    U32(node, 0x94) &= ~1u;
    U32(parent, 0x94) |= 1;
    U16(U32(node, 0x8c), 0x24) = U16(U32(parent, 0x8c), 0x24);
    Sim_100aa799(parent);
}

// FUNCTION: HGSIM 0x100aafe4
void __cdecl Sim_100aafe4(void *node, uint32_t key)
{
    if (node == 0)
        return;
    Sim_100aafe4(PTR(U32(node, 8)), key);
    if (U32(node, 0x8c) != 0 && U16(U32(node, 0x8c), 0x26) == key) {
        Sim_100aaf9b(PTR(U32(node, 4)));
        Sim_100aaf32(node);
    } else {
        Sim_100aafe4(PTR(U32(node, 4)), key);
    }
}

#define STEP GS32(0x1017d1a4)

// FUNCTION: HGSIM 0x100a9300
void __cdecl Sim_100a9300(void *node)
{
    uint32_t model, v;
    float minX = Float_OfBits(0x7f7fc99eu), minZ = Float_OfBits(0x7f7fc99eu);
    float maxX = Float_OfBits(0xff7fc99eu), maxZ = Float_OfBits(0xff7fc99eu);
    int32_t n, w, h, q;

    if (node == 0 || U32(node, 0x8c) == 0)
        return;
    model = U32(U32(node, 0x8c), 0x2c);
    if (model == 0)
        return;
    for (n = S16(model, 4), v = model + 0x18; n-- != 0; v += 0x58) {
        if (FL(v, 0) > maxX)
            maxX = FL(v, 0);
        if (FL(v, 8) > maxZ)
            maxZ = FL(v, 8);
        if (!(FL(v, 0) >= minX))
            minX = FL(v, 0);
        if (!(FL(v, 8) >= minZ))
            minZ = FL(v, 8);
    }
    w = Render_Ftol((double)maxX - (double)minX);
    h = Render_Ftol((double)maxZ - (double)minZ);
    if (h > w)
        w = h;
    if (w <= 0)
        return;
    q = U32(node, 4) == 0 ? w / 3 : w;
    G32(0x1017d1a4) = ((uint32_t)(q / 0x4000) + 1) << 14;
    G32(0x1017d1b4) = U32(node, 4) == 0 ? 0 : 1;
    G32(0x1017d1a0) = (uint32_t)((int32_t)(G32(0x1017d1a4) + (uint32_t)(STEP >> 3)) >> 1);
    G32(0x1017d1b8) = (uint32_t)(uintptr_t)node;
    G32(0x1017d1a8) = 1;
    G32(0x1017d1b0) = 1;
    G32(0x1017d1ac) = 0;
    G32(0x1017d19c) = 0;
    G32(0x1017d198) = 0;
    G32(0x1017d194) = 0;
    G32(0x1017d190) = 0;
    Sim_100aa351(node);
    Sim_100aa406(node);
}

/* (v + half) / step with half = step / 2 toward the sign of v: the nearest cell. */
static int32_t NearestCell(int32_t v)
{
    int32_t half = v >= 0 ? STEP >> 1 : (int32_t)(0u - (uint32_t)STEP) >> 1;

    return (int32_t)((uint32_t)v + (uint32_t)half) / STEP;
}

// FUNCTION: HGSIM 0x100a94fa
void __cdecl Sim_100a94fa(void)
{
    int32_t cx, cz, gx, gz;
    uint32_t node;

    if (G32(0x1017d1b0) == 0 || G32(0x1017d1a8) == 0)
        return;
    cx = Render_Ftol(FL(CAMERA, 0));
    cz = Render_Ftol(FL(CAMERA, 8));
    if (G32(0x1017d1b4) != 0) {
        int32_t dx = Math_AbsInt_101003d0((int)((uint32_t)cx - G32(0x1017d198)));
        int32_t dz = Math_AbsInt_101003d0((int)((uint32_t)cz - G32(0x1017d19c)));

        if (dx > GS32(0x1017d1a0) || dz > GS32(0x1017d1a0)) {
            gx = NearestCell(cx);
            gz = NearestCell(cz);
        } else {
            gx = GS32(0x1017d190);
            gz = GS32(0x1017d194);
        }
    } else {
        gx = cx / STEP;
        gz = cz / STEP;
    }
    if (G32(0x1017d1ac) == 0) {
        Sim_100aa39f(PTR(G32(0x1017d1b8)));
        G32(0x1017d1ac) = 1;
    }
    if (gx != GS32(0x1017d190) || gz != GS32(0x1017d194)) {
        G32(0x1017d198) = (uint32_t)gx * (uint32_t)STEP;
        G32(0x1017d19c) = (uint32_t)gz * (uint32_t)STEP;
        node = G32(0x1017d1b8);
        if (node != 0) {
            Sim_100aa39f(PTR(node));
            Sim_100a9d47(PTR(G32(0x1017d1b8)), Bits_OfFloat((float)GS32(0x1017d198)), 0,
                         Bits_OfFloat((float)GS32(0x1017d19c)));
            Sim_100aa799(PTR(G32(0x1017d1b8)));
        }
    }
    G32(0x1017d190) = (uint32_t)gx;
    G32(0x1017d194) = (uint32_t)gz;
}

// FUNCTION: HGSIM 0x100ac001
void __cdecl Sim_100ac001(void)
{
    int32_t removed = 0;
    uint32_t e;
    int32_t i;

    if (G32(0x1017d1d0) == 0 || G32(0x1017d1d4) == 0 || G32(0x1017d1c8) == 0)
        return;
    for (e = G32(0x1017d1d0); e != 0; e = U32(e, 0x10)) {
        switch (U32(e, 4)) {
        case 1: {
            uint32_t src = U32(e, 8);

            U32(e, 0x14) = U32(src, 0x58);
            FL(e, 0x18) = (float)((double)FL(src, 0x7c) + (double)GFL(0x1012baa0));
            U32(e, 0x1c) = U32(src, 0x60);
            Sim_100a9d47(PTR(U32(e, 0x30)), U32(e, 0x14), U32(e, 0x18), U32(e, 0x1c));
            Sim_100aa799(PTR(U32(e, 0x30)));
            break;
        }
        case 2: {
            uint32_t src = U32(e, 8);
            uint32_t x, y, z;

            Sim_100a9cfb(PTR(U32(src, 0x3c)), &x, &y, &z);
            U32(e, 0x14) = x;
            FL(e, 0x18) = (float)Sim_1002d961(Float_OfBits(x), 0.0f, Float_OfBits(z));
            U32(e, 0x1c) = z;
            Sim_100a9d47(PTR(U32(e, 0x30)), U32(e, 0x14), U32(e, 0x18), U32(e, 0x1c));
            Sim_100aa799(PTR(U32(e, 0x30)));
            break;
        }
        case 6:
            U32(e, 0x28) += G32(0x1015dc80);
            if (S32(e, 0x28) >= S32(e, 0x20)) {
                if (removed < 0x40) {
                    SIM_PTR(u32a, 0x101c52a0)[removed] = U32(e, 0);
                    removed++;
                }
            } else {
                int32_t left = (int32_t)(U32(e, 0x20) - U32(e, 0x28) * 2u);
                double v = W7O_CALL_100fcb70_A((double)left * (double)FL(e, 0x24) / (double)S32(e, 0x20));

                FL(e, 0x2c) = (float)((double)FL(e, 0x24) - v);
            }
            break;
        default:
            break;
        }
    }
    for (i = 0; i < removed; i++)
        Sim_100ac58e((int)SIM_PTR(u32a, 0x101c52a0)[i]);
}
