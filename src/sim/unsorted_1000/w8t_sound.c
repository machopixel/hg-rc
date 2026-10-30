#include "w8t.h"
#include "w2j.h"
#include "w5z.h"
#include "../unsorted_1002/w2j.h"
#include "../util/util.h"
#include "../util/w3o.h"

/* The slot table of the sounds (pointer at 0x1015d0a4). */
#define W8T_SLOTS BYTES(G32(0x1015d0a4))

// FUNCTION: HGSIM 0x1000e737
int32_t __cdecl Sim_1000e737(int32_t a0, uint32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, uint32_t a6,
                             int32_t a7, int32_t a8, uint32_t a9, uint32_t a10, uint32_t a11)
{
    float scale;
    int32_t v, slot;

    (void)a7;   /* the original only stores a word of table 0x109f1524 (or [0x1012df44]) into this argument */
    if (G32(0x1015d0a4) == 0 || G32(0x109e369c) == 0)
        return -2;
    if (U32(W8T_SLOTS, 0) == 0)
        return -2;
    scale = (float)((double)(int32_t)((uint32_t)a4 - 0x2710u) / (double)GFL(0x101110d0));
    v = Render_Ftol((double)a4 - (double)(int32_t)((uint32_t)a5 - 0xffffu) * (double)scale);
    if (v < -0x1770)
        return 0;
    if (v > 0)
        v = 0;
    a9 &= 0xff00;
    if (a2 < 1) {
        if (a3 == 0)
            return -6;
        a9 |= 0x32;
    } else if (*SIM_PTR(s16a, 0x109f1520 + a2 * 8) != -1) {
        a9 |= (uint32_t)(int32_t)*SIM_PTR(s16a, 0x109f1520 + a2 * 8);
    } else {
        a9 |= 0x32;
    }
    slot = W8T_CALL_1000e0d9(a2, a3, (uint16_t)a9, -1, a8, a10, a11);
    if (slot < 0)
        return slot;
    U32(W8T_SLOTS, slot * 4 + 0x1d4) = G32(0x1015dc78);
    if (a0 > GS32(0x1015dc80))
        U32(W8T_SLOTS, slot * 4 + 0x1d4) += (uint32_t)a0;
    if (a8 >= 0)
        S32(W8T_SLOTS, slot * 4 + 0x334) = a8;
    else
        S32(W8T_SLOTS, slot * 4 + 0x334) = -1;
    U32(W8T_SLOTS, slot * 4 + 0x254) = a6;
    U32(W8T_SLOTS, slot * 4 + 0x214) = a1;
    S32(W8T_SLOTS, slot * 4 + 0x294) = v;
    U32(W8T_SLOTS, slot * 4 + 0x54) = 1;
    return slot;
}

// FUNCTION: HGSIM 0x1000eb12
int32_t __cdecl Sim_1000eb12(float x, float y, float z, const char *name, int quieter, uint32_t a5, uint32_t a6)
{
    float r14, r18, r4, rc;     /* the four results of 0x100cbc6b */
    float d;

    if (name == 0)
        return 0;
    if (name[0] == 0)
        return 0;
    W8T_CALL_100cbc6b_1000(x, y, z, &r14, &r18, &r4, &rc);
    d = (float)((double)r18 / (double)GFL(0x101110d4));
    if ((double)GS32(0x1015ebb4) > (double)d || GS32(0x1015ebb4) == 0) {
        int32_t vol = W8T_CALL_1000f096(d);

        if (vol <= 0) {
            int32_t pan;

            if (quieter != 0)
                vol -= 1000;
            if (vol < -10000)
                vol = -10000;
            pan = Render_Ftol((double)r18 * (double)GFL(0x1013abbc));
            Sim_1000ed25((uint32_t)pan, (uint32_t)Render_Ftol(r14), (uint32_t)(uintptr_t)name, (uint32_t)vol,
                         0xffffffffu, (uint16_t)a5, a6);
        }
    }
    return Render_Ftol(r18);
}

// FUNCTION: HGSIM 0x1000ee23
void __cdecl Sim_1000ee23(void *obj)
{
    int started = 0;
    float px, py, pz;
    float r1c, r18, r28;
    const uint8_t *cam;
    int32_t vol;

    if (G32(0x1015d0a4) == 0)
        return;
    if (U32(obj, 0x18) != 0) {
        U32(obj, 0x18) = 0;
        return;
    }
    Sim_100a9cfb((void *)(uintptr_t)U32(obj, 0x10), (uint32_t *)&px, (uint32_t *)&py, (uint32_t *)&pz);
    cam = BYTES(G32(0x1014cae0));
    px = (float)((double)FL(cam, 0) - (double)px);
    cam = BYTES(G32(0x1014cae0));
    py = (float)((double)FL(cam, 4) - (double)py);
    cam = BYTES(G32(0x1014cae0));
    pz = (float)((double)FL(cam, 8) - (double)pz);
    W8T_CALL_100cbc6b_1000(px, py, pz, &r1c, &r18, &r28, &r28);
    if (!((double)S32(obj, 0) >= (double)r18)) {
        Sim_1000f029(obj);
        return;
    }
    if (S32(obj, 4) == -1) {
        int i;

        for (i = 0xb; i < 0x10; i++) {
            if (U32(W8T_SLOTS, i * 4 + 0x54) == 0) {
                S32(obj, 4) = i;
                break;
            }
        }
        if (S32(obj, 4) == -1)
            return;
        if (U32(obj, 8) == 0)
            U32(obj, 8) = W8T_CALL_100282d4(0, (int32_t)S16(obj, 0x1c), G32(0x1014ceb4), 0);
        if (U32(obj, 8) == 0) {
            if (S16(obj, 0x1c) != -1)
                Sim_1002828e((int)S16(obj, 0x1c), (const void *)(uintptr_t)G32(0x1014ceb4));
            S32(obj, 4) = -1;
            U32(obj, 8) = 0;
            return;
        }
        S32(W8T_SLOTS, S32(obj, 4) * 4 + 0x2d4) = (int32_t)S16(obj, 0x1c);
        U16(W8T_SLOTS, S32(obj, 4) * 2 + 0x314) = 0x1000;
        started = 1;
    }
    vol = W8T_CALL_1000f096(r18);
    if (vol <= -10000) {
        Sim_1000f029(obj);
        return;
    }
    if (started != 0 || U32(W8T_SLOTS, S32(obj, 4) * 4 + 0x54) == 0)
        U32(W8T_SLOTS, S32(obj, 4) * 4 + 0x54) = 1;
}
