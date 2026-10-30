#include "w14.h"
#include "w6g.h"
#include "w9e.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "../util/w10_interp.h"
#include "../view/w3r.h"
#include "../unsorted_1000/w2i.h"
#include "../unsorted_1000/w2k.h"
#include "../unsorted_1000/w3s.h"
#include "../unsorted_1002/w2h.h"
#include "../unsorted_1002/w2l.h"
#include "../unsorted_1002/w3o.h"
#include "../unsorted_1003/w7l.h"
#include "../unsorted_1006/w2e.h"
#include "../unsorted_1008/w8w.h"
#include "../unsorted_1008/w8x.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100a/w6g.h"
#include "../unsorted_100d/fns.h"
#include "../unsorted_100e/w5b.h"
#include "../unsorted_100e/w6h.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6h.h"

/* The resource loader and the error report of the DLL (left original). */
#define W14M_LOAD SIM_FN(uint32_t(__cdecl *)(char *, char *, int32_t *), 0x100ba7d5)
#define W14M_REPORT SIM_FN(void(__cdecl *)(uint32_t, const char *, const char *, uint32_t), 0x100bc280)
#define W14M_WTB() ((char *)(uintptr_t)G32(0x1014cedc))

/* The entry's matrix under the scene, its ground height and (for a BOLD object) its turn to the sun. */
static void W14M_Place(uint8_t *e, int32_t scene, float *m, uint32_t bold)
{
    float r[12];

    Mat3x4_Multiply(SIM_PTR(const float, 0x101a11c8 + scene * 0x7c), (const float *)(e + 0x54), m);
    if (!(m[10] >= GFL(0x1012bac0)))
        m[10] = (float)Sim_1002d961(m[9], m[10], m[11]);
    if (Crt_StrStr((const char *)e, SIM_PTR(const char, bold)) != 0) {
        float a = (float)((double)GFL(0x1012bac4) - (double)Sim_1008486e(GFL(0x1015dd50), GFL(0x1015dd54)));
        float c = (float)((double)Sim_1008486e(GFL(0x1015dd50), GFL(0x1015dd4c)) + (double)GFL(0x1012bac8));
        union {
            float f;
            uint32_t u;
        } tx, ty, tz;

        tx.f = m[9];
        ty.f = m[10];
        tz.f = m[11];
        Sim_10060220(r, a, FL(e, 0x40), c, tx.u, ty.u, tz.u, 2);
        Sim_10060209(r, m);
    }
}

/* The node's place from the entry: angles +0x3c..+0x44 and position +0x48..+0x50 (the height raised to the ground). */
static void W14M_Node(uint8_t *e, void *node)
{
    Sim_100aa0da(node, U32(e, 0x3c), U32(e, 0x40), U32(e, 0x44), 0);
    if (!(FL(e, 0x4c) >= GFL(0x1012bac0)))
        FL(e, 0x4c) = (float)Sim_1002d961(FL(e, 0x48), FL(e, 0x4c), FL(e, 0x50));
    Sim_100a9d47(node, U32(e, 0x48), U32(e, 0x4c), U32(e, 0x50));
    Sim_100aa799(node);
}

// FUNCTION: HGSIM 0x100b48de
uint32_t __cdecl Sim_100b48de(int32_t index, int32_t scene)
{
    uint8_t *e = SIM_PTR(uint8_t, 0x101a2100 + index * 0x88);
    /* The names are archive names (8 characters at most) and SDF sub-model names (9 at most, DECISIONS.md round 14),
     * so the original's 16-byte buffer holds them. */
    char name[16];
    int32_t size = 0, count = 0, sdf_size = 0, parts = 0;
    uint32_t list[10] = {0};
    uint32_t wtb, sdf, part, parent, i;
    int32_t kind, trigger;
    float m[12];

    if ((U32(e, 0x18) & 0x800) != 0 || e[0] == 0)
        return 0;
    trigger = S32(e, 0x24);
    if (trigger != -1 && (*SIM_PTR(uint16_t, 0x109e9ba0 + trigger * 0x50) & 0x8000) != 0 && Sim_10028b50() != 0)
        return 0;
    if (U32(e, 0x28) != 0 && (Sim_100093ea((const void *)(uintptr_t)U32(e, 0x28)) & 0x400) == 0)
        return 1;
    wtb = W14M_LOAD((char *)e, W14M_WTB(), &size);
    if (wtb == 0)
        return 0;
    Sim_100e2776(FL(e, 0x30), FL(e, 0x34), FL(e, 0x38));
    Sim_100e282a(U32(e, 0x18) & 0x1ff);
    parent = 0;
    if (S32(e, 0x14) > -1)
        parent = G32(0x101a212c + S32(e, 0x14) * 0x88);
    if (S32(e, 0x24) != -1) {
        G32(0x1017ed80) = 1;
        G32(0x1017ed84) = 0x200;
        G32(0x1017ed88) = U32(e, 0x24);
    }
    Str_Copy(name, (const char *)e);
    if (name[0] == 'o' || name[0] == 'O') {
        sdf = W14M_LOAD(name, SIM_PTR(char, 0x10150850), &sdf_size);
        if (sdf == 0) {
            U32(e, 0x28) = (uint32_t)(uintptr_t)Sim_100e2a31(wtb, &count, size, parent);
        } else {
            G32(0x1017e858) = U32(sdf, 0x14c);
            U32(e, 0x28) = (uint32_t)(uintptr_t)Sim_100e2a31(wtb, &count, size, parent);
            if (name[1] == 'c' || name[1] == 'C') {
                for (i = 1; i < 10; i++) {
                    count = 0;
                    if (U8(sdf, 0xc + i * 0x20) == 0)
                        break;
                    Str_Copy(name, (const char *)BYTES(sdf + 0xc + i * 0x20));
                    part = W14M_LOAD(name, W14M_WTB(), &sdf_size);
                    if (part == 0) {
                        W14M_REPORT(0x38, SIM_PTR(const char, 0x10150854), name, 0);
                    } else {
                        list[i - 1] = (uint32_t)(uintptr_t)Sim_100e2a31(part, &count, sdf_size, 0);
                        parts++;
                    }
                    Sim_100bb2b0((const uint32_t *)name, W14M_WTB());
                }
            } else {
                for (i = 1; i < 10; i++) {
                    int32_t added = 0;

                    if (U8(sdf, 0xc + i * 0x20) == 0)
                        break;
                    G32(0x1017e858) = U32(sdf, 0x14c + i * 4);
                    Str_Copy(name, (const char *)BYTES(sdf + 0xc + i * 0x20));
                    part = W14M_LOAD(name, W14M_WTB(), &sdf_size);
                    if (part == 0)
                        W14M_REPORT(0x38, SIM_PTR(const char, 0x10150858), name, 0);
                    else
                        Sim_100e3ba4(part, &added, sdf_size, parent, (void *)(uintptr_t)U32(e, 0x28));
                    Sim_100bb2b0((const uint32_t *)name, W14M_WTB());
                }
            }
            Sim_100bb2b0((const uint32_t *)e, SIM_PTR(const char, 0x1015085c));
            Sim_100b9203((char *)e, SIM_PTR(char, 0x10150860));
        }
    } else {
        U32(e, 0x28) = (uint32_t)(uintptr_t)Sim_100e2a31(wtb, &count, size, parent);
    }
    (void)parts;
    G32(0x1017ed80) = 0;
    Sim_100bb2b0((const uint32_t *)e, W14M_WTB());
    if (U32(e, 0x28) == 0)
        return 0;
    kind = (int32_t)(U32(e, 0x18) & 0xf000) >> 12;
    if (S32(e, 0x14) == -2) {
        W14M_Place(e, scene, m, 0x10150864);
        Sim_10009c55((void *)(uintptr_t)U32(e, 0x28), m);
        U32(e, 0x2c) = 0;
        Sim_10009408((void *)(uintptr_t)U32(e, 0x28), (unsigned short)U32(e, 0x1c));
        if ((U32(e, 0x18) & 0x400) != 0)
            Sim_1000942e((void *)(uintptr_t)U32(e, 0x28), (int16_t)U16(e, 0x24));
        else
            Sim_1000942e((void *)(uintptr_t)U32(e, 0x28), (int16_t)index);
        Sim_1002d890((void *)(uintptr_t)U32(e, 0x28), kind);
        Sim_10089e91(BYTES(U32(e, 0x28)));
    } else {
        if (U32(e, 0x2c) == 0)
            return 0;
        Sim_100a9b4c((void *)(uintptr_t)U32(e, 0x2c), U32(e, 0x28));
        Sim_100096f9((void *)(uintptr_t)U32(e, 0x28), U32(e, 0x2c));
        Sim_10089e91(BYTES(U32(e, 0x28)));
        if (S32(e, 0x14) == -1) {
            W14M_Place(e, scene, m, 0x1015086c);
            Sim_100a9dc3((void *)(uintptr_t)U32(e, 0x2c), m);
            Sim_100aa799((void *)(uintptr_t)U32(e, 0x2c));
        } else {
            for (i = 0; i < 10 && list[i] != 0; i++) {
                void *node = Sim_100a9830((void *)(uintptr_t)parent, 0x14);

                Sim_100a9b4c(node, list[i]);
                Sim_100096f9((void *)(uintptr_t)list[i], (uint32_t)(uintptr_t)node);
                W14M_Node(e, node);
                Sim_10089e91(BYTES(list[i]));
                Sim_1002d890((void *)(uintptr_t)list[i], GS32(0x1015267c));
                Sim_10009408((void *)(uintptr_t)list[i], 0x50);
                Sim_100aa4f0(node, (int)U32(e, 0x1c));
                Sim_100aa541(node, U32(e, 0x24));
            }
            W14M_Node(e, (void *)(uintptr_t)U32(e, 0x2c));
        }
        if ((U32(e, 0x18) & 0x400) != 0) {
            Sim_100aa4f0((void *)(uintptr_t)U32(e, 0x2c), (int)U32(e, 0x1c));
            Sim_100aa541((void *)(uintptr_t)U32(e, 0x2c), U32(e, 0x24));
        } else {
            Sim_10009408((void *)(uintptr_t)U32(e, 0x28), (unsigned short)U32(e, 0x1c));
            Sim_1000942e((void *)(uintptr_t)U32(e, 0x28), (int16_t)index);
        }
        Sim_100aa592((void *)(uintptr_t)U32(e, 0x2c), kind);
    }
    if (kind == 5)
        Sim_1003129f((void *)(uintptr_t)U32(e, 0x28));
    else if (kind == 8)
        Sim_1003079c((void *)(uintptr_t)U32(e, 0x28));
    return 1;
}
