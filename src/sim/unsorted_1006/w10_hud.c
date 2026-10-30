#include "w10.h"
#include "../simmem.h"
#include "../util/access.h"
#include "w8w.h"
#include "../unsorted_100c/w10.h"
#include "../unsorted_100f/w2e.h"

#define W10_6TEXT(addr) ((const char *)(uintptr_t)G32(addr))

/* The labels are the 16-byte entries of the DLL table 0x101362e0 (at most 8 characters, DECISIONS.md round 14), so the
 * original's 16-byte stack buffers hold them. */
// FUNCTION: HGSIM 0x10062b7c
int __cdecl Sim_10062b7c(uint32_t *rec, float yaw, float pitch, float range)
{
    char a[16], b[16], c[16];
    int32_t x, y;
    uint32_t slot, wdef, kind, unit;
    int locked = 0, alt = G32(0x10176930) != 0;

    Str_Copy(c, alt ? W10_6TEXT(0x109e8550) : W10_6TEXT(0x109e854c));
    slot = U32(rec, 0x84) + U32(rec, 0x4fc) * 0x128u;
    if (S32(slot, 4) == -1) {
        if (Sim_10064824(rec, &x, &y) == 0)
            return locked;
        Sim_1006519d((uint32_t)x, (uint32_t)y, (char *)(uintptr_t)(alt ? G32(0x109e8558) : G32(0x109e8554)));
        return 0;
    }
    wdef = 0x10145508u + U32(slot, 4) * 0xd6u;
    kind = U32(SIM_PTR(uint8_t, wdef), 0x10);
    if (U32(slot, 8) == 1) {
        if (kind == 0x3e || kind == 0x3f) {
            Str_Copy(a, alt ? W10_6TEXT(0x109e8570) : W10_6TEXT(0x109e856c));
            Str_Copy(b, alt ? W10_6TEXT(0x109e8578) : W10_6TEXT(0x109e8574));
        } else if (kind == 2 || kind == 1 || kind == 0) {
            Str_Copy(b, alt ? W10_6TEXT(0x109e8560) : W10_6TEXT(0x109e855c));
            Str_Copy(a, alt ? W10_6TEXT(0x109e8568) : W10_6TEXT(0x109e8564));
        } else {
            Str_Copy(b, alt ? W10_6TEXT(0x109e8550) : W10_6TEXT(0x109e854c));
            Str_Copy(a, alt ? W10_6TEXT(0x109e8550) : W10_6TEXT(0x109e854c));
        }
        if ((U32(SIM_PTR(uint8_t, wdef), 0x86) & 1) == 0) {
            /* in range and within 3 degrees in both axes of a selected target: the "locked" label */
            unit = rec[0];
            if (U32(unit, 0xf0) != 0 && (U32(unit, 0xf0) & 0x1100) == 0 &&
                range > *SIM_PTR(float, wdef + 0x4c) && !(range >= *SIM_PTR(float, wdef + 0x50)) &&
                3.0 > (double)pitch && !(-3.0 >= (double)pitch) && 3.0 > (double)yaw && !(-3.0 >= (double)yaw)) {
                locked = 1;
                Str_Copy(c, a);
            } else {
                Str_Copy(c, b);
            }
        } else if ((U16(rec, 0x560) & 0x80) != 0) {
            Str_Copy(c, alt ? W10_6TEXT(0x109e8578) : W10_6TEXT(0x109e8574));
        } else {
            Str_Copy(c, alt ? W10_6TEXT(0x109e8570) : W10_6TEXT(0x109e856c));
        }
    } else if (kind == 0x3e || kind == 0x3f) {
        Str_Copy(b, W10_6TEXT(0x109e856c));
    } else if (kind == 2 || kind == 1 || kind == 0) {
        Str_Copy(a, alt ? W10_6TEXT(0x109e8568) : W10_6TEXT(0x109e8564));
    } else {
        Str_Copy(c, alt ? W10_6TEXT(0x109e8558) : W10_6TEXT(0x109e8554));
    }
    if (Sim_10064824(rec, &x, &y) == 0)
        return locked;
    Sim_1006519d((uint32_t)x, (uint32_t)y, c);
    Sim_100c8ba7(BYTES(PTRTAB(G32(0x1017b8c8))), x - 0x28, y + 0x14, SIM_PTR(const uint8_t, wdef));
    return locked;
}
