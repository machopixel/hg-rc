#include "w7q.h"
#include "../unsorted_1000/w2h.h"
#include "../unsorted_1000/w2f.h"
#include "../unsorted_1000/w2g.h"
#include "../unsorted_1002/w5x.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2j.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_1010/w6k.h"
#include "../render/w3q.h"
#include "../util/util.h"
#include "../util/access.h"

typedef int(__stdcall *W7qHeapFreeFn)(uint32_t heap, uint32_t flags, uint32_t block);

#define IAT_HEAPFREE 0x10ac5634u

/* The name of group number n (0x300 bytes apart from 0x10a51ac0). */
#define W7Q_GROUP_NAME(n) SIM_PTR(char, 0x10a51ac0u + (uint32_t)(n) * 0x300u)
/* The group number word of entry i of the table 0x10ac1b00 (0xe bytes each). */
#define W7Q_ENTRY_GROUP(i) (*SIM_PTR(s16a, 0x10ac1b00u + (uint32_t)(i) * 0xeu))

// FUNCTION: HGSIM 0x100da14c
int __cdecl Sim_100da14c(void *info, void *out)
{
    uint8_t *a = info;
    char *name;
    int found = 0;
    int32_t i;

    if (a == 0 || (int8_t)a[0x20] == 0)
        return 0;
    if (GS32(0x1012da30) == -2)
        Sim_10007020();
    name = (char *)a + 0x20;
    if (S32(a, 0x40) == 3) {
        for (i = 0; i < GS32(0x1017e6cc); i++) {
            int32_t group = W7Q_ENTRY_GROUP(i);
            const char *known;

            if (group == -1)
                continue;
            known = W7Q_GROUP_NAME(group);
            if (Sim_1010d240((const uint8_t *)name, (const uint8_t *)known, Str_Length(known)) == 0) {
                found = 1;
                U32(out, 0x40) = (uint32_t)i;
                break;
            }
        }
        if (found == 0) {
            if (GS32(0x1017e6cc) > 0xff) {
                G32(0x1017e6cc) = 0xff;
            } else {
                int32_t last = S32(a, 0x50) + 1;
                int32_t k;
                int16_t fields;

                for (k = S32(a, 0x4c); k < last; k++) {
                    char texture[0x20];
                    char number[8];

                    Mem_Fill(texture, 0, 0x20);
                    Str_Cat(texture, name);
                    if (k < 10)
                        Str_Cat(texture, SIM_PTR(char, 0x10152484));
                    Str_Cat(texture, Text_IntToRadix(k, number, 10));
                    Sim_10006ba7(texture, -1, 1);
                }
                Sim_10006db8(GS32(0x1017e6cc), -1);
                fields = (int16_t)(0xb5 / S32(a, 0x54));
                Sim_100071da((int16_t)G32(0x1017e6cc), fields);
                Sim_1000711c((short)G32(0x1017e6cc), 1);
                U32(out, 0x40) = G32(0x1017e6cc);
                G32(0x1017e6cc) += 1;
            }
        }
    } else {
        for (i = 0x100; i < GS32(0x10152480); i++) {
            int32_t group = W7Q_ENTRY_GROUP(i);
            const char *known;

            if (group == -1)
                continue;
            known = W7Q_GROUP_NAME(group);
            if (Str_CompareN(name, known, Str_Length(known)) == 0) {
                found = 1;
                U32(out, 0x40) = (uint32_t)i;
                break;
            }
        }
        if (found == 0) {
            if (GS32(0x10152480) > 0x1ff) {
                G32(0x10152480) = 0x1ff;
            } else {
                Sim_10006ba7(name, -1, S32(a, 0x40) == 2 ? 1 : 0);
                Sim_10006db8(GS32(0x10152480), -1);
                U32(out, 0x40) = G32(0x10152480);
                G32(0x10152480) += 1;
            }
        }
    }
    return 1;
}

// FUNCTION: HGSIM 0x100dafa8
int __cdecl Sim_100dafa8(void *src, W7qVisitFn visit)
{
    uint8_t name[0x14];
    uint8_t found[0x20];
    int go = 1;
    int result = 0;
    void *object;

    U16(name, 0) = U16(src, 8);
    Str_CopyN((char *)name + 2, (const char *)src + 0xa, 0xc);
    name[0xe] = 0;
    if ((S16(name, 0) == -2 || S16(name, 0) == -1) && (int8_t)name[2] == 0x5e) {
        if (G32(0x1017e688) == 0) {
            W7Q_CALL_100bc280(0x39, 0);
            go = 0;
        } else if (GS32(0x1017e690) < GS32(0x1017f118)) {
            Str_CopyN((char *)name + 2, (const char *)(uintptr_t)(G32(0x1017e688) + G32(0x1017e690) * 0xcu + 8u),
                      0xc);
            G32(0x1017e690) += 1;
            name[0xe] = 0;
        } else {
            W7Q_CALL_100bc280(0x3b, 0);
            go = 0;
        }
    }
    if (go != 0) {
        object = W7Q_CALL_100278c0(name, found);
        if (object != 0) {
            result = 1;
            result &= visit(object);
            Sim_10027a8f(object);
        } else {
            W7Q_CALL_100bc280(0x34, 0);
        }
    }
    return result;
}

// FUNCTION: HGSIM 0x100db259
int __cdecl Sim_100db259(void *src)
{
    char name[0x100];
    uint8_t *object;

    Str_Copy(name, (const char *)src + 0x24);
    object = W7Q_CALL_100599a0(name);
    if (object == 0)
        W7Q_CALL_100bc280(0x55, 0);
    *SIM_PTR(u16a, 0x109f1448) += 1;
    *SIM_PTR(u16a, 0x109f144a) += 1;
    *SIM_PTR(u16a, 0x109f144c) += U16(object, 0x84);
    *SIM_PTR(u16a, 0x109f144e) += U16(object, 0x84);
    *SIM_PTR(u16a, 0x109f1450) += U16(object, 0x84);
    SIM_API(W7qHeapFreeFn, IAT_HEAPFREE)(G32(0x1017b8f8), 1, (uint32_t)(uintptr_t)object);
    return 1;
}
