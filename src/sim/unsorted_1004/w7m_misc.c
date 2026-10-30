#include "w7m.h"
#include "w3s.h"
#include "w6e.h"
#include "../unsorted_1003/fns.h"
#include "../unsorted_1007/w3p.h"
#include "../unsorted_100a/w6g.h"
#include "../unsorted_100f/w2e.h"

// FUNCTION: HGSIM 0x1004ee50
uint32_t __cdecl Sim_1004ee50(uint32_t *obj)
{
    uint8_t *block;
    uint8_t *other;
    const char *name;
    uint32_t kind = 0;

    /* (callers never pass NULL; the original then returns whatever eax held, this returns 0) */
    if (obj == 0)
        return 0;
    block = BYTES(obj[10]);
    if (block == 0)
        return (uint32_t)(uintptr_t)obj;    /* what the original leaves in eax there */
    switch (obj[0]) {
    case 1:
        other = BYTES(obj[0x12]);
        name = (const char *)(uintptr_t)U32(other, 0x90);
        if (name[0] != 'S')
            break;
        if (name[1] == 'A') {
            obj[0x24] |= 0x3000;
            kind = 0x3000;
        } else {
            obj[0x24] |= 0x2000;
            kind = 0x2000;
        }
        if ((obj[0x24] & 0xff) == 4 || (obj[0x24] & 0xff) == 5) {
            if (U16(block, 0x562) & 0x2000)
                obj[0x24] = (obj[0x24] & ~0xffu) | 4;
            else
                obj[0x24] = (obj[0x24] & ~0xffu) | 5;
        }
        break;
    case 9:
        obj[0x24] |= 0x1000;
        kind = 0x1000;
        if (U16(block, 0x560) & 0x400)
            obj[0x24] = (obj[0x24] & ~0xffu) | 0x18;
        if (U16(block, 0x560) & 0x10)
            W7M_CALL_1006ad98((int16_t)U32(U32(block, 0), 0x150), block, 2, 0);
        break;
    case 5:
        other = BYTES(obj[0x12]);
        name = (const char *)(uintptr_t)U32(other, 0x90);
        if (name[1] == '1' && name[2] == '3') {
            obj[0x24] |= 0x5000;
            kind = 0x5000;
        } else if (name[1] == '6' && name[2] == '3') {
            obj[0x24] |= 0x4000;
            kind = 0x4000;
        }
        break;
    }
    return kind;
}

// FUNCTION: HGSIM 0x10047cf2
float __cdecl Sim_10047cf2(uint32_t *obj)
{
    float half = (float)((double)W7M_F(obj, 0x10) / (double)W7M_GF(0x101115e8));
    float diff = (float)((double)W7M_F(obj, 4) - (double)W7M_F(obj, 8));
    double m = W7M_FMOD((double)diff, (double)W7M_F(obj, 0x10));
    float step;
    int set;

    diff = (float)m;
    if (m > (double)half)
        diff = (float)((double)diff - (double)W7M_F(obj, 0x10));
    else if (-(double)half > (double)diff)
        diff = (float)((double)diff + (double)W7M_F(obj, 0x10));
    step = (float)((double)(int32_t)(G32(0x1015dc78) - obj[0]) * (double)diff / (double)W7M_F(obj, 0xc));
    if ((W7m_Bits(diff) & 0x80000000u) != 0 && !(step > diff))
        set = 1;
    else if (!(diff > W7M_GF(0x101115e0)))
        set = 0;
    else
        set = step >= diff;
    if (set)
        obj[2] = obj[1];
    else
        W7M_F(obj, 8) = (float)((double)W7M_F(obj, 8) + (double)step);
    W7M_F(obj, 8) = (float)W7M_FMOD((double)W7M_F(obj, 8), (double)W7M_F(obj, 0x10));
    obj[0] = G32(0x1015dc78);
    return W7M_F(obj, 8);
}

// FUNCTION: HGSIM 0x10045308
void __cdecl Sim_10045308(float *a, float *b, uint32_t *c, uint32_t *d, uint32_t *e, uint32_t *f)
{
    float x, y;

    Sim_10044b2c((uint32_t *)a, (uint32_t *)b, c, d, e, f);
    if (G32(0x10179cd4) == 0) {
        G32(0x109b9b94) = G32(0x1015fd9c);
        x = Sim_10047bcd(SIM_PTR(uint32_t, 0x109b9b90));
        G32(0x109b9ae4) = G32(0x1015fd98);
        y = Sim_10047bcd(SIM_PTR(uint32_t, 0x109b9ae0));
        *(w7m_f32 *)b = (float)((double)*(w7m_f32 *)b + (double)x);
        *(w7m_f32 *)a = (float)((double)*(w7m_f32 *)a + (double)y);
    }
    G32(0x1015ef08) = 1;
}

// FUNCTION: HGSIM 0x1004409f
void __cdecl Sim_1004409f(int index)
{
    /* v: 0 = [ebp-0x24], 1 = [ebp-4], 2 = [ebp-0xc], 3 = [ebp-0x10], 4 = [ebp-0x14], 5 = [ebp-0x18] */
    uint32_t v[6];
    uint32_t rec;
    double d;
    int i;
    static const uint8_t from[6] = {0, 1, 2, 4, 3, 5};

    if (index >= GS32(0x1013aec4) || index >= 10 || index < 0)
        return;
    if (index == 0) {
        Sim_10045308((float *)&v[3], (float *)&v[4], &v[5], &v[0], &v[1], &v[2]);
        for (i = 0; i < 6; i++)
            v[i] = W7m_Bits((float)((double)W7M_F(G32(0x1014cae0), 4 * from[i]) - (double)W7m_Float(v[i])));
    } else {
        v[0] = G32(0x1015ed58);
        v[1] = G32(0x1015ee90);
        v[2] = G32(0x1015ed30);
        v[3] = G32(0x1015ed68);
        v[4] = G32(0x1015ed18);
        v[5] = G32(0x1015ed48);
    }
    rec = 0x1015ed70 + (uint32_t)index * 0x1c;
    d = (double)*SIM_PTR(s32a, rec + 0x18) / *SIM_PTR(double, 0x10111558);
    Sim_10047b76(SIM_PTR(void, 0x1015ed50), *SIM_PTR(u32a, rec), v[0], d);
    Sim_10047b76(SIM_PTR(void, 0x1015ee88), *SIM_PTR(u32a, rec + 4), v[1], d);
    Sim_10047b76(SIM_PTR(void, 0x1015ed28), *SIM_PTR(u32a, rec + 8), v[2], d);
    Sim_10047b76(SIM_PTR(void, 0x1015ed60), *SIM_PTR(u32a, rec + 0x10), v[3], d);
    Sim_10047b76(SIM_PTR(void, 0x1015ed10), *SIM_PTR(u32a, rec + 0xc), v[4], d);
    Sim_10047b76(SIM_PTR(void, 0x1015ed40), *SIM_PTR(u32a, rec + 0x14), v[5], d);
}

// FUNCTION: HGSIM 0x100410ff
void __cdecl Sim_100410ff(const uint8_t *packet)
{
    uint32_t p = (uint32_t)(uintptr_t)packet;
    /* the original leaves the upper halves of these registers in the 16-bit arguments it pushes */
    uint32_t high = p & 0xffff0000u;
    uint32_t obj;

    if (S16(packet, 3) != 0x4200) {
        obj = PTRTAB(U8(packet, 3));
        if (obj == 0 || U32(obj, 0x14) != 3)
            return;
        if (U32(obj, 0xc) != G32(0x1017b8c8)) {
            Mem_Move(BYTES(G32(0x1015ecac)), packet, 8);
            if (S32(obj, 0xc) >= 0) {
                uint32_t i = (uint32_t)Sim_1003dbc0(U32(obj, 0xc));

                Net_DpSend_1007e950((i & 0xffff0000u) | U16(U32(G32(0x1015d004), 0x16), i * 2),
                                    BYTES(G32(0x1015ecac)), 8, 1);
            } else {
                Net_DpSend_1007e950((obj & 0xffff0000u) | (G32(0x1015ecd8) & 0xffffu), BYTES(G32(0x1015ecac)), 8, 1);
            }
            return;
        }
        high = obj & 0xffff0000u;
    }
    W7M_CALL_10015d30(PTRTAB(U8(packet, 2)), (p & 0xffff0000u) | U16(packet, 3), (p & 0xffff0000u) | U8(packet, 5),
                      high | U16(packet, 6));
}

// FUNCTION: HGSIM 0x10040191
void __cdecl Sim_10040191(const uint8_t *packet)
{
    uint32_t who = U8(packet, 2);
    uint32_t obj, state, entry;
    int32_t i;

    if (PTRTAB(who) == 0)
        return;
    if (G32(0x1017b8c8) == who)
        return;
    if ((U32(PTRTAB(who), 0x14) == 3 && U32(PTRTAB(who), 0xc) == G32(0x1017b8c8)) || !((int32_t)who < GS32(0x1015dc64)))
        return;
    obj = PTRTAB(who);
    state = U32(obj, 0x28);
    if ((S16(obj, 0x1c) & 2) != 0 || (S16(obj, 0x1c) & 4) != 0)
        return;
    U32(obj, 0x150) = U16(packet, 3);
    for (i = 0; i < 0x11; i++) {
        uint32_t level = U8(packet, 5 + i) & 0xf;
        double q;
        float qf;

        entry = U32(state, 0x88) + (uint32_t)i * 0x3c;
        q = (double)(int32_t)(0xf - level) * (double)S32(entry, 0x38) / (double)W7M_GF(0x10111530);
        qf = (float)q;
        W7M_F(entry, 0) = (float)(q * (double)W7M_GF(0x1011150c));
        W7M_F(entry, 8) = (float)((double)qf * (double)W7M_GF(0x1011150c));
        *SIM_PTR(uint8_t, 0x1015da45 + U32(obj, 4) * 0x16 + (uint32_t)i) = U8(packet, 5 + i);
        Sim_100aae7b(BYTES(U32(U32(state, 0), 0x48)), (int)level, (uint32_t)i + 1);
        if ((U8(packet, 5 + i) & 0x10) != 0 && (S16(entry, 0x36) & 0x2000) == 0)
            W7M_CALL_1006abf6((int16_t)U32(obj, 0x150), state, i + 1);
    }
}
