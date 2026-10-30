#include "w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

#define PTR(p) ((uint32_t)(uintptr_t)(p))

// FUNCTION: HGSIM 0x100da4f7
int __cdecl Sim_100da4f7(void *node, void *table)
{
    int32_t key = S32(node, 0x10);
    int idx = (int)(int8_t)U8(node, 0x14) - 0x41;
    const char *name = (const char *)BYTES(node) + 0x14;
    uint8_t *cur;
    int done = 0;
    int cmp;

    if (idx < 0 || idx > 0x19)
        idx = 0x1a;
    cur = BYTES(U32(table, (uint32_t)idx * 4));
    if (cur == 0) {
        U32(table, (uint32_t)idx * 4) = PTR(node);
        return 1;
    }

    if (key < S32(cur, 0x10)) {
        while (cur != 0) {
            int32_t ck = S32(cur, 0x10);

            if (key > ck) {
                U32(node, 0) = PTR(cur);
                U32(node, 4) = U32(cur, 4);
                U32(U32(node, 4), 0) = PTR(node);
                U32(cur, 4) = PTR(node);
                done = 1;
                break;
            }
            if (key == ck)
                break;
            if (U32(cur, 0) == 0) {
                U32(node, 4) = PTR(cur);
                U32(cur, 0) = PTR(node);
                done = 1;
                break;
            }
            cur = BYTES(U32(cur, 0));
        }
    } else if (key > S32(cur, 0x10)) {
        while (cur != 0) {
            int32_t ck = S32(cur, 0x10);

            if (key < ck) {
                U32(node, 4) = PTR(cur);
                U32(node, 0) = U32(cur, 0);
                U32(U32(node, 0), 4) = PTR(node);
                U32(cur, 0) = PTR(node);
                done = 1;
                break;
            }
            if (key == ck)
                break;
            if (U32(cur, 4) == 0) {
                U32(node, 0) = PTR(cur);
                U32(cur, 4) = PTR(node);
                done = 1;
                break;
            }
            cur = BYTES(U32(cur, 4));
        }
    }
    if (done)
        return 1;

    cmp = Str_Compare(name, (const char *)cur + 0x14);
    if (cmp < 0) {
        while (cur != 0) {
            cmp = Str_Compare(name, (const char *)cur + 0x14);
            if (cmp > 0) {
                U32(node, 0xc) = PTR(cur);
                U32(node, 8) = U32(cur, 8);
                U32(U32(node, 8), 0xc) = PTR(node);
                U32(cur, 8) = PTR(node);
                break;
            }
            if (cmp == 0)
                return 0;
            if (U32(cur, 0xc) == 0) {
                U32(node, 8) = PTR(cur);
                U32(cur, 0xc) = PTR(node);
                break;
            }
            cur = BYTES(U32(cur, 0xc));
        }
    } else if (cmp > 0) {
        while (cur != 0) {
            cmp = Str_Compare(name, (const char *)cur + 0x14);
            if (cmp < 0) {
                U32(node, 8) = PTR(cur);
                U32(node, 0xc) = U32(cur, 0xc);
                U32(U32(node, 0xc), 8) = PTR(node);
                U32(cur, 0xc) = PTR(node);
                break;
            }
            if (cmp == 0)
                return 0;
            if (U32(cur, 8) == 0) {
                U32(node, 0xc) = PTR(cur);
                U32(cur, 8) = PTR(node);
                break;
            }
            cur = BYTES(U32(cur, 8));
        }
    } else {
        return 0;
    }
    return 1;
}
