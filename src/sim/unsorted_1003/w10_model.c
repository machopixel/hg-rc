#include "w10.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../view/w2d.h"
#include "../view/w2l.h"
#include "../unsorted_1000/w2f.h"
#include "../unsorted_1000/w2g.h"
#include "../unsorted_1000/w2i.h"
#include "../unsorted_1000/w2k.h"
#include "../unsorted_1002/w2h.h"
#include "../unsorted_1008/w3s.h"
#include "../unsorted_1008/w8x.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100a/w6g.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100d/fns.h"
#include "../unsorted_100e/w5b.h"
#include "../unsorted_100e/w6h.h"
#include "../unsorted_100f/w2e.h"

typedef void *(__cdecl *W10_3ModelLoadFn)(const char *name, uint32_t ext, uint32_t *out);
typedef void *(__stdcall *W10_3HeapAllocFn)(void *heap, uint32_t flags, uint32_t bytes);

#define W10_3MODEL(rec, i) U32(rec, 0x50u + (uint32_t)(i) * 4u)

// FUNCTION: HGSIM 0x1003aecb
uint32_t __cdecl Sim_1003aecb(int32_t index, int32_t level, uint32_t mode)
{
    uint8_t *rec = GBYTES(0x109b9e20u + (uint32_t)index * 0x7eu);
    uint8_t *other;
    uint32_t loaded = 0, model, x, y;
    int32_t count = 0, prev, i;
    uint32_t kind, node, shape, sub;
    const char *name = (const char *)(rec + 8 + (uint32_t)level * 16u);

    if (S16(rec, 0x7c) != 0)
        return 1;
    if (S32(rec, 4) == level)
        return (uint32_t)(uintptr_t)rec;      /* the original returns with the record pointer in eax */
    prev = S32(rec, 4);
    U32(rec, 4) = 0xffffffffu;
    if (W10_3MODEL(rec, level) == 0) {
        shape = (uint32_t)(uintptr_t)SIM_FN(W10_3ModelLoadFn, 0x100ba7d5)(name, G32(0x1014cedc), &loaded);
        if (shape == 0)
            return 0;
        Sim_100e2776(1.0f, 1.0f, 1.0f);
        Sim_100e282a(U32(rec, 0x64));
        W10_3MODEL(rec, level) = (uint32_t)(uintptr_t)Sim_100e2a31(shape, &count, (int32_t)loaded, 0);
        if (W10_3MODEL(rec, level) == 0)
            return 0;
        Sim_100bb2b0((const uint32_t *)(uintptr_t)name, (const char *)(uintptr_t)G32(0x1014cedc));
        if (W10_3MODEL(rec, level) == 0)
            return 0;
        if (U32(rec, 0x60) == 0) {
            if (S32(rec, 0x48) == -2) {
                node = U32(PTRTAB(U32(rec, 0)), 0x48);
                if (mode == 0)
                    U32(rec, 0x60) = (uint32_t)(uintptr_t)Sim_100a9830((void *)(uintptr_t)node, 10);
                else
                    U32(rec, 0x60) = (uint32_t)(uintptr_t)Sim_100a9a1a((void *)(uintptr_t)node, (void *)(uintptr_t)mode);
            } else {
                other = GBYTES(0x109b9e20u + U32(rec, 0x48) * 0x7eu);
                node = U32(other, 0x60);
                if (node == 0)
                    node = Sim_100096ee((void *)(uintptr_t)W10_3MODEL(other, level));
                if (mode == 0)
                    sub = (uint32_t)(uintptr_t)Sim_100a9830((void *)(uintptr_t)node, 10);
                else
                    sub = (uint32_t)(uintptr_t)Sim_100a9a1a((void *)(uintptr_t)node, (void *)(uintptr_t)mode);
                if (sub == 0)
                    return 0;
                U32(rec, 0x60) = sub;
            }
        }
        model = W10_3MODEL(rec, level);
        Sim_100a9b4c((void *)(uintptr_t)U32(rec, 0x60), model);
        Sim_100096f9((void *)(uintptr_t)model, U32(rec, 0x60));
        kind = U16(rec, 0x74u + (uint32_t)level * 2u) & 0xf0u;
        Sim_10009408((void *)(uintptr_t)model, (unsigned short)(kind | 0x100u));
        Sim_1000942e((void *)(uintptr_t)W10_3MODEL(rec, level), (int16_t)U16(rec, 0));
        Sim_1000941e((void *)(uintptr_t)W10_3MODEL(rec, level), U16(rec, 0x4c));
        Sim_10089e91(BYTES(W10_3MODEL(rec, level)));
        if (kind == 0xa0)
            Sim_100aa4f0((void *)(uintptr_t)U32(rec, 0x60), 0x1a0);
        Sim_100a9d47((void *)(uintptr_t)U32(rec, 0x60), U32(rec, 0x68), U32(rec, 0x6c), U32(rec, 0x70));
        U32(U32(rec, 0x60), 0x90) = (uint32_t)(uintptr_t)SIM_API(W10_3HeapAllocFn, 0x10ac5658)(
            (void *)(uintptr_t)G32(0x1017b8f8), 1, Str_Length(name) + 1);
        Str_Copy((char *)(uintptr_t)U32(U32(rec, 0x60), 0x90), name);
    } else {
        for (i = 0; i < 4; i++)
            if (W10_3MODEL(rec, i) != 0 && (U16(W10_3MODEL(rec, i), 2) & 0xf0) == 0x50)
                return 1;
    }
    Sim_100a9b4c((void *)(uintptr_t)U32(rec, 0x60), W10_3MODEL(rec, level));
    kind = U16(rec, 0x74u + (uint32_t)level * 2u) & 0xf0u;
    if (kind == 0x70) {
        Sim_1008a40a((void *)(uintptr_t)W10_3MODEL(rec, level));
        Sim_1008a37f((void *)(uintptr_t)W10_3MODEL(rec, level));
    } else {
        Sim_1008a46e((void *)(uintptr_t)W10_3MODEL(rec, level));
        Sim_1002d890((void *)(uintptr_t)W10_3MODEL(rec, level), 6);
    }
    if (prev != -1) {
        model = W10_3MODEL(rec, level);
        x = Sim_100093ea((const void *)(uintptr_t)model);
        y = Sim_100093ea((const void *)(uintptr_t)W10_3MODEL(rec, prev));
        /* the original keeps the upper bits of the first value: and cl, 0xf / or ecx, (y & 0xf0) */
        Sim_100093be((uint16_t *)(uintptr_t)W10_3MODEL(rec, level), (x & 0xffffff0fu) | (y & 0xf0u));
        Sim_1008a40a((void *)(uintptr_t)W10_3MODEL(rec, prev));
        Sim_1008a37f((void *)(uintptr_t)W10_3MODEL(rec, prev));
    }
    if ((S16(PTRTAB(U32(rec, 0)), 0x1c) & 0x4000) != 0) {
        Sim_100aa351((void *)(uintptr_t)U32(PTRTAB(U32(rec, 0)), 0x48));
        Sim_100aa406((void *)(uintptr_t)U32(PTRTAB(U32(rec, 0)), 0x48));
        U16(PTRTAB(U32(rec, 0)), 0x1c) |= 0x800;
    }
    Sim_100aa799((void *)(uintptr_t)U32(rec, 0x60));
    U32(rec, 4) = (uint32_t)level;
    return 1;
}
