#include "w8t.h"
#include "fns.h"
#include "w3t.h"
#include "w6d.h"
#include "../unsorted_1001/w5y.h"
#include "../unsorted_1001/w6d.h"
#include "../unsorted_1002/w3o.h"
#include "../unsorted_1002/w7l.h"
#include "../unsorted_1009/w3o.h"
#include "../unsorted_100c/w6h.h"
#include "../unsorted_100d/w5c.h"
#include "../util/util.h"
#include "../util/w3o.h"

/* The segment record of Sim_100d6f16 / Sim_1009cd36 (0x38 bytes, as in the original frame). */
typedef struct {
    uint32_t w[14];
} W8tSegment;

// FUNCTION: HGSIM 0x1000a6f0
int __cdecl Sim_1000a6f0(void *obj, int check)
{
    W8tSegment seg;
    uint32_t hit;
    float a = (float)W8T_FMOD_1000((double)FL(obj, 0x68) + (double)FL(obj, 0x74) - (double)FL(obj, 0xe8) +
                                   (double)GFL(0x10111030), 360.0);
    uint16_t kind;
    uint32_t flags;

    if (check != 0 && !((double)a >= (double)GFL(0x10111034)) && (double)a > (double)GFL(0x10111038))
        return 0;
    Sim_100d6f16(obj, &seg);
    Sim_1009cd36(&seg, FL(obj, 0xd8));
    hit = 0;
    if (Sim_1002dda9(&seg, &hit, (int)U32(obj, 4)) == 0 && Sim_100c2851(&seg, &hit, U32(obj, 4), 1) == 0)
        return 1;
    if (hit == 0)
        return 1;
    kind = U16(hit, 2);
    flags = U32(obj, 0xf0);
    if ((kind & 0x100) != 0 && (flags & 0x200) != 0)
        return U16(hit, 0x24) == (flags & 0xff);
    if ((kind & 0x200) != 0 && (flags & 0x400) != 0)
        return U16(hit, 0x24) == (flags & 0xff);
    return 0;
}

// FUNCTION: HGSIM 0x1000a866
int __cdecl Sim_1000a866(void *obj, float x, float y, float z)
{
    float ang, dist, unused14, elev;     /* the four results of 0x100cbc6b */
    float k = GFL(0x1011103c);
    float s30, dx, s34, dz, dy;
    int32_t steps, i;

    W8T_CALL_100cbc6b_1000((float)((double)FL(obj, 0x58) - (double)x), (float)((double)FL(obj, 0x5c) - (double)y),
                           (float)((double)FL(obj, 0x60) - (double)z), &ang, &unused14, &dist, &elev);
    steps = Render_Ftol((double)dist / (double)k);
    s30 = (float)(W8T_CALL_10084766_1000((float)((double)ang + (double)GFL(0x10111038))) * (double)GFL(0x1011103c));
    dx = (float)(W8T_CALL_10084766_1000((float)((double)elev + (double)GFL(0x10111038))) * (double)s30);
    s34 = (float)(W8T_CALL_10084766_1000(ang) * (double)GFL(0x1011103c));
    dz = (float)(W8T_CALL_10084766_1000((float)((double)elev + (double)GFL(0x10111038))) * (double)s34);
    dy = (float)(W8T_CALL_10084766_1000(elev) * (double)GFL(0x1011103c));
    for (i = 0; i < steps; i++) {
        double h;
        float hf;

        x = (float)((double)x - (double)dx);
        y = (float)((double)y - (double)dy);
        z = (float)((double)z - (double)dz);
        h = Sim_1002d961(x, y, z);
        hf = (float)h;
        if (h - (double)GFL(0x1011103c) > 0.0)
            return 0;
        if (!((double)hf + (double)GFL(0x1011103c) >= 0.0))
            return 0;
    }
    return 1;
}

// FUNCTION: HGSIM 0x1000d0e2
int __cdecl Sim_1000d0e2(void *obj, float a)
{
    int32_t wait = 0;
    uint32_t found = 0;
    int done = 0;
    union { uint32_t u; float f; } turn;
    float unused10;
    float da, r;
    uint8_t *state;

    turn.u = U32(obj, 0xec);
    W8T_CALL_1000c1ea(obj, &a, &turn.f, &unused10);
    da = (float)((double)a - (double)FL(obj, 0x74));
    r = (float)W8T_FMOD_1000((double)FL(obj, 0x64) + (double)FL(U32(obj, 0x28), 0x1c) + (double)turn.f, 360.0);
    if (S32(obj, 0x1be) <= GS32(0x1015dc78)) {
        wait = Sim_1009c929((int)(int8_t)U8(obj, 0x1b6));
        U32(obj, 0x1be) = G32(0x1015dc78) + (uint32_t)wait * 0x16u;
        if (!((double)da >= (double)GFL(0x1011109c)) && (double)da > (double)GFL(0x101110b0) &&
            !((double)r >= (double)GFL(0x1011104c)) && (double)r > (double)GFL(0x101110b4) &&
            ((U16(obj, 0x1aa) & 0xffu) == G32(0x1017b8c8) || Sim_1009c929(3) == 0) && wait == 0) {
            int got;

            if (U32(obj, 0x1c2) != 0)
                got = Sim_1000bf73(obj, (int *)&found);
            else
                got = Sim_1000bba7(obj, &found);
            if (got != 0 && Sim_1000a6f0(obj, 1) != 0) {
                if (U32(obj, 0x1c2) != 0)
                    U8(U32(obj, 0x54), 0x28) = 1;
                else
                    U8(U32(obj, 0x54), 0x26) = 1;
                done = 1;
            }
        }
    }
    if (found != 0 && done != 0)
        Sim_1000c172(obj);
    if ((U16(obj, 0x1aa) & 0xf00) == 0x200) {
        float v = FL(U32(PTRTAB(U16(obj, 0x1aa) & 0xff), 0x54), 8);

        if ((double)v < (double)GFL(0x10111060) || (double)v > (double)GFL(0x10111060))
            wait = 0;
    }
    if (done == 0)
        wait = 0;
    state = BYTES(U32(obj, 0x54));
    FL(state, 4) = (float)((double)Sim_1001448d((uint32_t)(uintptr_t)obj, a, wait) * (double)GFL(0x10111064));
    state = BYTES(U32(obj, 0x54));
    FL(state, 0) = (float)(-(double)Sim_100144bb((uint32_t)(uintptr_t)obj, turn.f, wait) * (double)GFL(0x10111068));
    return done;
}
