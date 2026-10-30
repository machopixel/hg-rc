#include "w7m.h"
#include "../util/util.h"
#include "../unsorted_1004/w3s.h"
#include "../unsorted_1004/w6e.h"
#include "../unsorted_100a/w3t.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_1008/w6f.h"
#include "../unsorted_1007/w2g.h"
#include "../unsorted_1006/w6e.h"
#include "w2f.h"
#include "w4v.h"
#include "w6e.h"
#include "../unsorted_1000/w5a.h"
#include "../unsorted_1002/w3o.h"
#include "../unsorted_1004/w3p.h"
#include "../unsorted_100a/w5c.h"

#define TENTHS3 0.3 /* the double 0x3fd3333333333333 */

// FUNCTION: HGSIM 0x10036cc0
void __cdecl Sim_10036cc0(void *obj)
{
    uint8_t *s = BYTES(U32(obj, 0x28));

    if (s == 0)
        return;
    U32(s, 0x4fc) = 0;
    U32(s, 0x4e8) = 0;
    U32(s, 0x500) = 0x3f800000;
    U32(s, 0x508) = 0;
    U32(s, 0x4f8) = 0;
    U32(s, 0x4f4) = 0;
    U16(s, 0x560) = 0;
    U32(s, 0x4f0) = 0;
    U32(s, 0x4d8) = 0;
    U32(s, 0x4e0) = 0;
    U32(s, 0x504) = 0;
    U32(s, 0x50c) = 0;
    U32(s, 0x54c) = 0;
    U32(s, 0x544) = 0;
    Sim_100a9d7c(BYTES(U32(U32(s, 0), 0x48)), 0.0f, W7M_F(s, 0x520), 0.0f);
    Sim_100aa799(BYTES(U32(U32(s, 0), 0x48)));
    Sim_100a9ca3(BYTES(U32(U32(s, 0), 0x48)), U32(s, 0) + 0x64, U32(s, 0) + 0x68, U32(s, 0) + 0x6c);
    Sim_100a9cfb(BYTES(U32(U32(s, 0), 0x48)), (uint32_t *)BYTES(U32(s, 0) + 0x58), (uint32_t *)BYTES(U32(s, 0) + 0x5c),
                 (uint32_t *)BYTES(U32(s, 0) + 0x60));
    Sim_10047b76(s + 0x24, U32(U32(s, 0), 0x58), U32(U32(s, 0), 0x58), TENTHS3);
    Sim_10047b76(s + 0x74, U32(U32(s, 0), 0x5c), U32(U32(s, 0), 0x5c), TENTHS3);
    Sim_10047b76(s + 0x34, U32(U32(s, 0), 0x60), U32(U32(s, 0), 0x60), TENTHS3);
    U32(U32(s, 0), 0x78) = 0;
    U32(U32(s, 0), 0x74) = 0;
    U32(U32(s, 0), 0x70) = 0;
    U32(U32(s, 0), 0x94) = 0;
    U32(U32(s, 0), 0x90) = 0;
    U8(U32(s, 0), 0x98) = 0;
    Sim_100aa454(BYTES(U32(U32(s, 0), 0x48)));
    U32(U32(s, 0), 0xd0) = 0;
    U32(U32(s, 0), 0xcc) = 0x3f800000;
    W7M_CALL_10011093(U32(s, 0));
}

// FUNCTION: HGSIM 0x10036f25
void __cdecl Sim_10036f25(uint32_t *state)
{
    uint32_t angle;

    if (state == 0 || state[0x4f0 / 4] != 2)
        return;
    Sim_10047bcd(state + 9);
    Sim_10047bcd(state + 0x1d);
    Sim_10047bcd(state + 0xd);
    U32(state[0], 0x58) = state[0xb];
    U32(state[0], 0x5c) = state[0x1f];
    U32(state[0], 0x60) = state[0xf];
    Sim_100a9d47(BYTES(U32(state[0], 0x48)), U32(state[0], 0x58), U32(state[0], 0x5c), U32(state[0], 0x60));
    Sim_100aa0da(BYTES(U32(state[0], 0x48)), U32(state[0], 0x64), U32(state[0], 0x68), U32(state[0], 0x6c), 0);
    Sim_100aa799(BYTES(U32(state[0], 0x48)));
    angle = U32(state[0], 0x68);
    W7M_F(state[0], 0xd0) = (float)W7M_CALL_10084766(W7m_Float(angle));
    W7M_F(state[0], 0xcc) =
        (float)W7M_CALL_10084766((float)((double)W7m_Float(angle) + (double)W7M_GF(0x10111328)));
}

// FUNCTION: HGSIM 0x1003705a
void __cdecl Sim_1003705a(uint32_t *state)
{
    W7M_CALL_10011681(state[0]);
    if (state[0x4f0 / 4] == 2) {
        uint32_t id = U32(state[0], 8);
        uint32_t rec = id * 0x4fc3a + G32(0x104727a0 + id * 4) * 0x13f + 0x1047383a;

        U32(state[0], 0xf0) = 0;
        U8(U32(state[0], 0x54), 0x46) = 0;
        if (U32(rec, 2) == 0x100 && U8(rec, 0xee) != 0 && U8(rec, 0xf0) == 1) {
            uint32_t k = U8(rec, 0xef);
            uint32_t pos[3];
            uint32_t target[3];
            float d[3], speed;
            double sum;

            Sim_100a9cfb(BYTES(U32(state[0], 0x48)), &pos[0], &pos[1], &pos[2]);
            target[0] = G32(0x109a8658 + k * 0x54);
            target[1] = G32(0x109a865c + k * 0x54);
            target[2] = G32(0x109a8660 + k * 0x54);
            d[0] = (float)((double)W7m_Float(target[0]) - (double)W7m_Float(pos[0]));
            d[1] = (float)((double)W7m_Float(target[1]) - (double)W7m_Float(pos[1]));
            d[2] = (float)((double)W7m_Float(target[2]) - (double)W7m_Float(pos[2]));
            speed = (float)((double)GS32(0x1015dc80) * (double)W7M_F(state, 0x4d4) / (double)W7M_GF(0x1011132c));
            sum = (double)d[0] * (double)d[0] + (double)d[1] * (double)d[1] + (double)d[2] * (double)d[2];
            if ((double)speed * (double)speed >= sum) {
                Sim_10084c95(&d[0], &d[1], &d[2]);
                W7M_F(state, 0x28) = (float)((double)d[0] * (double)speed);
                W7M_F(state, 0x78) = (float)((double)d[1] * (double)speed);
                W7M_F(state, 0x38) = (float)((double)d[2] * (double)speed);
            } else {
                state[0x28 / 4] = target[0];
                state[0x78 / 4] = target[1];
                state[0x38 / 4] = target[2];
            }
        }
    } else {
        U32(state[0], 0x94) = 0;
        U32(state[0], 0x90) = 0;
    }
    switch (state[0x4f0 / 4]) {
    case 0:
        state[0x4f0 / 4] = 2;
        state[0x4d8 / 4] = 0;
        break;
    case 1:
    case 3:
        state[0x4f0 / 4] = 2;
        break;
    case 4:
        if (state[0x4d8 / 4] == 0) {
            state[0x4d8 / 4] = G32(0x1015dc78) + 0x712;
            Sim_1007cbb4();
        }
        if (GS32(0x1015dc78) < (int32_t)state[0x4d8 / 4])
            Sim_10061c32(state);
        break;
    }
}

// FUNCTION: HGSIM 0x10038dd3
void __cdecl Sim_10038dd3(uint32_t *data, uint32_t *a, uint32_t *b, uint32_t *obj, int32_t key, int skip)
{
    uint32_t pos[3];
    uint32_t e;

    Sim_100a9cfb(BYTES(U32(obj[0], 0x48)), &pos[0], &pos[1], &pos[2]);
    e = G32(0x109e3760);
    *b = 0;
    if (e != 0 && skip != 0)
        e = U32(e, 0x690);
    for (; e != 0; e = U32(e, 0x690)) {
        if (key == -1) {
            double dist = Sim_10084b8c((float)((double)W7m_Float(pos[0]) - (double)W7M_F(e, 0x680)),
                                       (float)((double)W7m_Float(pos[1]) - (double)W7M_F(e, 0x684)),
                                       (float)((double)W7m_Float(pos[2]) - (double)W7M_F(e, 0x688)));

            if (dist >= (double)W7M_GF(0x10111334))
                continue;
        } else if (S32(e, 0x124) != key) {
            continue;
        }
        *a = U32(e, 8);
        *b = U32(e, 0);
        *data = e + 0xc;
        return;
    }
}

// FUNCTION: HGSIM 0x100386d5
int __cdecl Sim_100386d5(uint32_t *obj, int32_t id, int place)
{
    uint32_t node;
    uint32_t value = 0;
    uint32_t rec;
    int again = 0;
    void *root = BYTES(U32(obj[0], 0x48));

    if (U32(obj, 0x490) == 0)
        return 0;
    node = (uint32_t)(uintptr_t)Sim_100aae0f(root, SIM_PTR(const char, 0x1013a530));        /* "WRF" */
    if (node == 0)
        node = (uint32_t)(uintptr_t)Sim_100aae0f(root, SIM_PTR(const char, 0x1013a534));    /* "WBZ" */
    if (node == 0)
        return 0;
    rec = U32(obj, 0x84) + U32(obj, 0x5fb) * 0x128;
    if (id != -1 && S32(rec, 0x118) != id)
        return 0;
    if (place != 0) {
        uint32_t pos[3];
        float h;

        Sim_100ab151(BYTES(U32(obj[0], 0x48)), 10, &value, &node);
        Sim_100a9cfb(BYTES(node), &pos[0], &pos[1], &pos[2]);
        h = (float)Sim_1002d961(W7m_Float(pos[0]), W7m_Float(pos[1]), W7m_Float(pos[2]));
        Sim_100a9d7c(BYTES(node), 0.0f, (float)((double)h - (double)W7m_Float(pos[1])), 0.0f);
        Sim_100aa799(BYTES(node));
    } else {
        value = U32(node, 0);
    }
    Sim_10009408(BYTES(U32(node, 0x8c)), 0x800);
    if (node == 0 || value == 0)
        return 0;
    if (id == -1 && Sim_10038eea(obj, -1) != 0 && place != 0)
        again = 1;
    Sim_10038fac(value, node, obj, BYTES(rec));
    U32(obj, 0x5ff) = 0;
    Sim_10038a9c(obj);
    Sim_1000ebfb(SIM_PTR(const char, 0x1013a538), 0, 0, 5, 0x50);       /* "AFXHDWPS" */
    if (G32(0x1015ec58) != 0 && place != 0 &&
        (U32(obj[0], 4) == G32(0x1017b8c8) || (U32(obj[0], 0x14) == 3 && U32(obj[0], 0xc) == G32(0x1017b8c8))))
        Sim_100412cb((int)U32(obj[0], 4), place, U32(rec, 0x118));
    if (again != 0)
        W7M_CALL_10038913(obj, -1, 1);
    return 1;
}
