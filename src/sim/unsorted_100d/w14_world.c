#include "w14.h"
#include "fns.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "../unsorted_1000/w3s.h"
#include "../unsorted_1002/w2h.h"
#include "../unsorted_1003/w2e.h"
#include "../unsorted_1003/w7l.h"
#include "../unsorted_1006/w5y.h"
#include "../unsorted_1008/w8x.h"
#include "../unsorted_100a/w5a.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100a/w6g.h"
#include "../unsorted_100b/w2g.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100b/w7p.h"
#include "../unsorted_100b/w9e.h"
#include "../unsorted_100e/w5b.h"
#include "../unsorted_100e/w6h.h"
#include "../unsorted_100f/w2e.h"

/* The resource loader and the error report of the DLL (left original). */
#define W14W_LOAD SIM_FN(uint32_t(__cdecl *)(char *, char *, int32_t *), 0x100ba7d5)
#define W14W_REPORT4 SIM_FN(void(__cdecl *)(uint32_t, const char *, int32_t, uint32_t), 0x100bc280)
#define W14W_REPORT2 SIM_FN(void(__cdecl *)(uint32_t, uint32_t), 0x100bc280)
#define W14W_WTB() ((char *)(uintptr_t)G32(0x1014cedc))

/* A node for model under parent: placed by the chunk's angles and position. */
static void W14W_Node(uint32_t parent, uint32_t flags, uint32_t model, const uint32_t *f)
{
    void *node = Sim_100a9830((void *)(uintptr_t)parent, flags);

    Sim_100a9b4c(node, model);
    Sim_100096f9((void *)(uintptr_t)model, (uint32_t)(uintptr_t)node);
    Sim_100aa0da(node, f[3], f[4], f[5], 0);
    Sim_100a9d47(node, f[6], f[7], f[8]);
    Sim_100aa742(node);
    Sim_10089e91(BYTES(model));
}

// FUNCTION: HGSIM 0x100db78b
void __cdecl Sim_100db78b(uint8_t *chunk, uint32_t unused, uint32_t owner, int32_t add, int32_t deferred,
                          uint32_t side)
{
    /* The OBJ chunk names fit the 10-byte field +0x3a and SDF part names are 9 characters at most (DECISIONS.md round
     * 14), so the original's 12-byte buffer holds them. */
    char name[12];
    int32_t size = 0, sdf_size = 0, count = 0, report = S16(chunk, 0x38), key = S16(chunk, 8);
    int32_t parent = S16(chunk, 0xa), state = S16(chunk, 0xc), link, i;
    uint32_t colour = U32(chunk, 0x34), list[10] = {0};
    uint32_t wtb, sdf, part, model, pnode;
    union {
        float f[9];
        uint32_t u[9];
    } v;
    uint32_t m[12];

    (void)unused;
    if (state < 0 || state >= 9)
        state = 4;
    if (Str_CompareN((const char *)chunk + 0x3a, SIM_PTR(const char, 0x101524d0), 4) == 0 ||
        Str_CompareN((const char *)chunk + 0x3a, SIM_PTR(const char, 0x101524d8), 4) == 0)
        state = 8;
    wtb = W14W_LOAD((char *)chunk + 0x3a, W14W_WTB(), &size);
    if (wtb == 0) {
        W14W_REPORT4(0x38, SIM_PTR(const char, 0x101524e0), report, 0);
        return;
    }
    for (i = 0; i < 3; i++) {
        v.f[i] = (float)S32(chunk, 0xe + 4 * i);
        v.f[3 + i] = (float)((double)S32(chunk, 0x1a + 4 * i) * (double)GFL(0x1012bd88));
        v.f[6 + i] = (float)S32(chunk, 0x26 + 4 * i);
    }
    if (add != 0 && GS32(0x101507f8) != -1) {
        link = -1;
        if (parent == -2) {
            link = -2;
        } else if (parent != -1) {
            parent = Sim_100db770(parent);
            link = Sim_100b38c5(parent);
            if (link == -1)
                W14W_REPORT2(0x2d, 0);
        }
        key = Sim_100db770(key);
        Sim_100b39c3(key, (const char *)chunk + 0x3a, v.u[0], v.u[1], v.u[2], v.u[3], v.u[4], v.u[5], v.u[6], v.u[7],
                     v.u[8], G32(0x101507f8), link, owner, 0, (uint32_t)state, colour);
        Sim_100bb2b0((const uint32_t *)(chunk + 0x3a), W14W_WTB());
        return;
    }
    if (deferred != 0 && GS32(0x101c42a0) < GS32(0x101a1160)) {
        int32_t at, kind;

        SIM_PTR(uint32_t, G32(0x1017d1f8))[GS32(0x101c42a0)] = (uint32_t)Sim_100db770(key);
        kind = Sim_100b3839((uint32_t)Sim_100db770(key));
        parent = Sim_100db770(parent);
        at = Sim_100b3839((uint32_t)parent);
        SIM_PTR(uint32_t, G32(0x1017d1f4))[GS32(0x101c42a0)] = (uint32_t)Sim_1003aa7d(
            (const char *)chunk + 0x3a, v.u[6], v.u[7], v.u[8], (uint32_t)at, (int)side, kind, (unsigned short)colour, 0);
        G32(0x101c42a0) += 1;
        Sim_100bb2b0((const uint32_t *)(chunk + 0x3a), W14W_WTB());
        return;
    }
    Sim_100e2776(v.f[0], v.f[1], v.f[2]);
    Sim_100e282a(0);
    Str_Copy(name, (const char *)chunk + 0x3a);
    if ((name[0] == 'o' || name[0] == 'O') &&
        (sdf = W14W_LOAD(name, SIM_PTR(char, 0x101524ec), &sdf_size)) != 0) {
        G32(0x1017e858) = U32(sdf, 0x14c);
        model = (uint32_t)(uintptr_t)Sim_100e2a31(wtb, &count, size, 0);
        if (name[1] == 'c' || name[1] == 'C') {
            for (i = 1; i < 10 && U8(sdf, 0xc + i * 0x20) != 0; i++) {
                Str_Copy(name, (const char *)BYTES(sdf + 0xc + i * 0x20));
                part = W14W_LOAD(name, W14W_WTB(), &sdf_size);
                if (part == 0) {
                    W14W_REPORT4(0x38, SIM_PTR(const char, 0x101524f0), report, 0);
                    return;
                }
                list[i - 1] = (uint32_t)(uintptr_t)Sim_100e2a31(part, &count, size, 0);
                Sim_100bb2b0((const uint32_t *)name, W14W_WTB());
            }
        } else {
            for (i = 1; i < 10 && U8(sdf, 0xc + i * 0x20) != 0; i++) {
                int32_t added = 0;

                G32(0x1017e858) = U32(sdf, 0x14c + i * 4);
                Str_Copy(name, (const char *)BYTES(sdf + 0xc + i * 0x20));
                part = W14W_LOAD(name, W14W_WTB(), &sdf_size);
                if (part == 0) {
                    W14W_REPORT4(0x38, SIM_PTR(const char, 0x101524f4), report, 0);
                    return;
                }
                Sim_100e3ba4(part, &added, sdf_size, 0, (void *)(uintptr_t)model);
                Sim_100bb2b0((const uint32_t *)name, W14W_WTB());
            }
        }
        Sim_100bb2b0((const uint32_t *)(chunk + 0x3a), SIM_PTR(const char, 0x101524f8));
        Sim_100b9203((char *)chunk + 0x3a, SIM_PTR(char, 0x101524fc));
    } else {
        model = (uint32_t)(uintptr_t)Sim_100e2a31(wtb, &count, size, 0);
    }
    if (model != 0) {
        U16(model, 2) = (uint16_t)colour;
        if (state >= 0 && state < 9) {
            Sim_1002d890((void *)(uintptr_t)model, state);
        } else {
            Sim_1002d890((void *)(uintptr_t)model, 4);
            W14W_REPORT2(0x37, 0);
        }
        Sim_100b367e(Sim_100db770(key), (int)model);
        if (parent == -2) {
            Sim_10060656((uint32_t)(uintptr_t)m, v.u[3], v.u[4], v.u[5], v.u[6], v.u[7], v.u[8]);
            Sim_10009c55((void *)(uintptr_t)model, (const float *)(void *)m);
            Sim_10089e91(BYTES(model));
            if (S32(model, 0x34) == 5)
                Sim_1003129f((void *)(uintptr_t)model);
            else if (S32(model, 0x34) == 8)
                Sim_1003079c((void *)(uintptr_t)model);
        } else {
            uint32_t at;

            pnode = 0;
            parent = Sim_100db770(parent);
            at = Sim_100b36d8((uint32_t)parent);
            if (at != 0)
                pnode = Sim_100096ee((void *)(uintptr_t)at);
            W14W_Node(pnode, 10, model, v.u);
            for (i = 0; i < 10 && list[i] != 0; i++)
                W14W_Node(pnode, 0x14, list[i], v.u);
        }
    }
    Sim_100bb2b0((const uint32_t *)(chunk + 0x3a), W14W_WTB());
}
