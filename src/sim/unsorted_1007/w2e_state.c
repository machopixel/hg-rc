#include "w2e.h"
#include "../core/w2a.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

static void SaveAndReset(void *dst)
{
    Mem_Move(dst, GBYTES(0x1014cae8), 0x68);
    G32(0x1014cb00) = 0;
    G32(0x1014caf0) = 0;
    G32(0x1014caf4) = 1;
    G32(0x1014caec) = 1;
    G32(0x1014cb38) = 0xb00;
    G32(0x1014cb34) = 1;
    G32(0x1014caf8) = G32(0x1014caf8) & 0xfffffffbu;
}

// FUNCTION: HGSIM 0x100787f4
void __cdecl Sim_100787f4(void *dst)
{
    SaveAndReset(dst);
}

// FUNCTION: HGSIM 0x10078857
void __cdecl Sim_10078857(void *dst)
{
    SaveAndReset(dst);
}

#define SLOT(i) (GBYTES(0x10149d28) + (uint32_t)(i) * 0x3cu)

// FUNCTION: HGSIM 0x1007df6c
int __cdecl Sim_1007df6c(const char *text, const char *label, int ticks, int limit)
{
    int found = 0;
    int result = 0;
    int best = -1;
    int best_priority = 0x29a;
    int best_expiry = 0;
    int i = 0;

    while (!found) {
        if (U32(SLOT(i), 0xc) == 0) {
            best = i;
            found = 1;
            continue;
        }
        if (S32(SLOT(i), 0x10) <= limit) {
            if (S32(SLOT(i), 0x10) < best_priority) {
                best = i;
                best_priority = S32(SLOT(i), 0x10);
                best_expiry = S32(SLOT(i), 0x34);
            } else if (S32(SLOT(i), 0x10) == best_priority && best_expiry < S32(SLOT(i), 0x34)) {
                best = i;
                best_expiry = S32(SLOT(i), 0x34);
            }
        }
        i++;
        if (i == 2)
            found = 1;
    }
    if (best != -1) {
        uint8_t *slot = SLOT(best);

        result = 1;
        Str_CopyN((char *)BYTES(U32(slot, 0)), text, 0xff);
        U8(U32(slot, 0), 0xff) = 0;
        U32(slot, 0xc) = 1;
        U32(slot, 0x10) = (uint32_t)limit;
        U32(slot, 0x34) = (uint32_t)(Sim_1002ab9d() + ticks);
        if (label != 0 && *label != 0)
            Str_Copy((char *)slot + 0x14, label);
        else
            Str_Copy((char *)slot + 0x14, (const char *)BYTES(G32(0x109b98c0)));
    }
    return result;
}
