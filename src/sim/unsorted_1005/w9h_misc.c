#include "w9h.h"
#include "w6e.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100f/w2e.h"

// FUNCTION: HGSIM 0x100527fa
int __cdecl Sim_100527fa(int index, uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    int result = 0;
    int32_t idx = 0; /* the original leaves this unset when index >= 16 */
    uint32_t i, rec, x, y, z;

    if (index < 0x10) {
        idx = S32(GBYTES(0x109b7d40), (uint32_t)index * 0x6c);
        if ((uint32_t)idx < G32(0x1015dc64)) {
            rec = PTRTAB(idx);
            U32(rec, 0x58) = a;
            U32(rec, 0x5c) = b;
            U32(rec, 0x60) = c;
            U32(rec, 0x68) = d;
            Sim_100a9d47(BYTES(U32(rec, 0x48)), a, b, c);
            Sim_100aa0da(BYTES(U32(rec, 0x48)), 0, d, 0, 0);
            Sim_100aa799(BYTES(U32(rec, 0x48)));
            for (i = 0; i < G32(0x1015dc64); i++) {
                rec = PTRTAB(i);
                if (S32(rec, 8) != index)
                    continue;
                Sim_100529a2(i, &x, &y, &z);
                Sim_100a9d47(BYTES(U32(rec, 0x48)), x, b, y);
                Sim_100aa0da(BYTES(U32(rec, 0x48)), 0, z, 0, 0);
                Sim_100aa799(BYTES(U32(rec, 0x48)));
                U32(rec, 0x58) = x;
                U32(rec, 0x5c) = b;
                U32(rec, 0x60) = y;
                U32(rec, 0x68) = z;
            }
            result = 1;
        }
    }
    Mem_Move(GBYTES(0x109b7d7c) + (uint32_t)index * 0x6c, Sim_100aa863(BYTES(U32(PTRTAB(idx), 0x48))), 0x30);
    return result;
}
