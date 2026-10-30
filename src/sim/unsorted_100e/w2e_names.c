#include "w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100e2837
void *__cdecl Sim_100e2837(const char *name, void *table)
{
    int32_t key;
    int idx;
    uint8_t *cur;
    int missing = 0;
    int cmp;

    Mem_Move(&key, name, 4);
    idx = (int)(int8_t)name[0] - 0x41;
    if (idx < 0 || idx > 0x19)
        idx = 0x1a;
    cur = BYTES(U32(table, (uint32_t)idx * 4));
    if (cur == 0) {
        missing = 1;
    } else {
        if (key < S32(cur, 0x10)) {
            cur = BYTES(U32(cur, 0));
            while (cur != 0) {
                int32_t ck = S32(cur, 0x10);

                if (key == ck)
                    break;
                if (key > ck) {
                    missing = 1;
                    break;
                }
                cur = BYTES(U32(cur, 0));
            }
        } else if (key > S32(cur, 0x10)) {
            cur = BYTES(U32(cur, 4));
            while (cur != 0) {
                int32_t ck = S32(cur, 0x10);

                if (key == ck)
                    break;
                if (key < ck) {
                    missing = 1;
                    break;
                }
                cur = BYTES(U32(cur, 4));
            }
        }
        if (!missing && cur != 0) {
            cmp = Str_Compare(name, (const char *)cur + 0x14);
            if (cmp < 0) {
                cur = BYTES(U32(cur, 0xc));
                while (cur != 0) {
                    cmp = Str_Compare(name, (const char *)cur + 0x14);
                    if (cmp == 0)
                        break;
                    if (cmp > 0) {
                        missing = 1;
                        break;
                    }
                    cur = BYTES(U32(cur, 0xc));
                }
            } else if (cmp > 0) {
                cur = BYTES(U32(cur, 8));
                while (cur != 0) {
                    cmp = Str_Compare(name, (const char *)cur + 0x14);
                    if (cmp == 0)
                        break;
                    if (cmp < 0) {
                        missing = 1;
                        break;
                    }
                    cur = BYTES(U32(cur, 8));
                }
            }
        }
        if (cur == 0)
            missing = 1;
    }
    return missing ? 0 : cur;
}
