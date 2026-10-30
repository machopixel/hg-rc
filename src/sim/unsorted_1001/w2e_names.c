#include "w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

/*
 * The block selected by [0x1013bb40]: 0x4fc3a bytes each from 0x10473800, an int count at
 * its start and entries of 0x13f bytes from +0x3a.
 */
#define BLOCK() (BYTES(0x10473800u + G32(0x1013bb40) * 0x4fc3au))

/*
 * Shared body of the three routines. strOff and recOff select the fields of obj, code is
 * the value looked for in the dword at entry+2. The entry address is computed afresh from
 * [0x1013bb40] on every pass, as the original does.
 */
static void Fill(void *obj, void *rec, uint32_t strOff, uint32_t recOff, uint32_t code)
{
    uint8_t *block = BLOCK();
    int i;

    if (obj == 0)
        return;
    if (rec == 0)
        return;
    if (U32(rec, 8) == 0)
        return;
    U32(U32(obj, recOff), 0xc) = 0;
    Str_Copy((char *)BYTES(U32(obj, strOff)), (const char *)BYTES(G32(0x101c5158)));
    for (i = 0; i < S32(block, 0); i++) {
        uint8_t *entry = BYTES(G32(0x1013bb40) * 0x4fc3au + (uint32_t)i * 0x13fu + 0x1047383au);

        if (U8(entry, 0x74) != 0 && U32(entry, 2) == code) {
            Str_Copy((char *)BYTES(U32(obj, strOff)), (const char *)entry + 0xad);
            U32(U32(obj, recOff), 0xc) = (uint32_t)i;
        }
    }
}

// FUNCTION: HGSIM 0x10010183
void __cdecl Sim_10010183(void *obj, void *rec)
{
    Fill(obj, rec, 0x1c, 0x24, 0x8000);
}

// FUNCTION: HGSIM 0x10010267
void __cdecl Sim_10010267(void *obj, void *rec)
{
    Fill(obj, rec, 0x44, 0x4c, 0x8001);
}

// FUNCTION: HGSIM 0x1001034b
void __cdecl Sim_1001034b(void *obj, void *rec)
{
    Fill(obj, rec, 0x6c, 0x74, 0x8002);
}
