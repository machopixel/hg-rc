#include "w10d.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_1004/fns.h"
#include "w8z.h"
#include "../unsorted_1000/w24.h"

#define F(p, off) (*(float *)(BYTES(p) + (off)))
#define GF(addr) (*SIM_PTR(float, addr))

static float W10d_Sub(float a, float b)
{
    return (float)((double)a - (double)b);
}

static int W10d_Close(uint32_t w)
{
    return S32(w, 0x14) == 2 || (S32(w, 0x14) == 3 && U32(w, 0xc) == G32(0x1017b8c8));
}

// FUNCTION: HGSIM 0x100cada2
int __cdecl Sim_100cada2(int32_t who, int32_t index, uint32_t flags)
{
    int isme = who == GS32(0x1017b8c8);
    uint32_t w, t, trec, node;
    float tx, ty, tz, dx, dy, dz, a, b, c, d, p0, p1, p2;
    int32_t level;
    int close;

    g_w24TargetSet = 0;
    if (index < 0)
        return -1;
    if (index >= GS32(0x1015dc64))
        return -2;
    if ((flags & 0x15) != 0)
        return -9;
    w = PTRTAB(who);
    t = PTRTAB(index);
    close = W10d_Close(w);
    if (G32(0x1017e5b0) == 0 && (S16(t, 0x1c) & 0x10) != 0 && who == GS32(0x1017b8c8))
        return -3;
    if ((S16(t, 0x1c) & 6) != 0)
        return -3;
    if (S32(t, 4) == who)
        return -3;
    if ((flags & 0x10000) != 0 && (S16(t, 0x1c) & 0x40) == 0)
        return -4;
    if ((flags & 0x20000) != 0 && Sim_10052be7(index) != 0)
        return -9;
    if ((flags & 0x40000) != 0 && Sim_10052be7(index) != 1)
        return -9;
    if (G32(0x1017e5b0) == 0 && (S16(t, 0x1c) & 0x1400) == 0 && who == GS32(0x1017b8c8))
        return -5;
    if ((S16(t, 0x1c) & 0x800) != 0 && who == GS32(0x1017b8c8))
        return -5;
    if (U32(t, 0x48) == 0)
        return -8;
    if (Sim_100a9b71((void *)(uintptr_t)U32(t, 0x48)) == 0)
        return -8;
    tx = F(t, 0x58);
    ty = F(t, 0x5c);
    tz = F(t, 0x60);
    if (close) {
        if (S32(t, 0) == 1) {
            trec = U32(t, 0x28);
            if ((U16(trec, 0x562) & 8) != 0) {
                ty = W10d_Sub(ty, F(trec, 0x520));
            } else if (S32(w, 0x1f0) == 1) {
                /* the product of two floats is exact in the 53-bit register */
                ty = (float)((double)ty - (double)F(trec, 0x520) * (double)GF(0x1012bc44));
            } else if (S32(w, 0) == 1 && Sim_1009c929(2) != 0) {
                node = U32(trec, 0x494);
                if (node != 0) {
                    Sim_100a9cfb((void *)(uintptr_t)node, (uint32_t *)&p0, (uint32_t *)&p1, (uint32_t *)&p2);
                    ty = p1;
                }
            }
        }
        if (S32(w, 0) == 1 && W10d_Close(w) && (S16(w, 0x1a6) == 3 || S16(w, 0x1a6) == 2)) {
            node = U32(w, 0x4c);
            if (node != 0) {
                Sim_100a9cfb((void *)(uintptr_t)node, (uint32_t *)&p0, (uint32_t *)&p1, (uint32_t *)&p2);
                dx = W10d_Sub(tx, p0);
                dy = W10d_Sub(ty, p1);
                dz = W10d_Sub(tz, p2);
            } else {
                dx = W10d_Sub(tx, F(w, 0x58));
                dy = W10d_Sub(ty, F(w, 0x5c));
                dz = W10d_Sub(tz, F(w, 0x60));
            }
        } else {
            dx = W10d_Sub(tx, F(w, 0x58));
            dy = W10d_Sub(ty, F(w, 0x5c));
            dz = W10d_Sub(tz, F(w, 0x60));
        }
    } else {
        dx = W10d_Sub(tx, F(w, 0x58));
        dy = W10d_Sub(ty, F(w, 0x5c));
        dz = W10d_Sub(tz, F(w, 0x60));
    }
    Sim_100cbc6b(dx, dy, dz, &a, &b, &c, &d);
    /* the original leaves dy ([ebp-0x3c]) and the angle a ([ebp-0x10] of 0x100cbc6b) on the stack, where
       0x1000aa9e later reads them (DECISIONS.md, round 24) */
    g_w24TargetDy = dy;
    g_w24TargetAngle = a;
    g_w24TargetSet = 1;
    if ((S16(t, 0x1c) & 0x1000) == 0 && b > GF(0x1012bc48) && who == GS32(0x1017b8c8))
        return -7;
    level = (int32_t)(U32(U32(t, 0x28), 0xbdf) - U32(U32(w, 0x28), 0xbe3));
    if (level > 4)
        level = 4;
    if (level < 0)
        level = 0;
    /* the range is an unsigned dword (fild of a qword with a zero top half); the quotient rounds to double */
    if (isme && level > 0 && (double)b / (double)GF(0x1012bc4c) > (double)G32(0x1014be6du + (uint32_t)level * 0x1du) &&
        isme)
        return -7;
    F(w, 0xdc) = tx;
    F(w, 0xe0) = ty;
    F(w, 0xe4) = tz;
    F(w, 0xe8) = a;
    F(w, 0xd8) = b;
    F(w, 0xd4) = c;
    F(w, 0xec) = d;
    return 1;
}
