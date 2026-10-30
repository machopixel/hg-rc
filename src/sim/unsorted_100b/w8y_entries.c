#include "w8y.h"
#include "w2l.h"
#include "w6g.h"
#include "../util/access.h"
#include "../unsorted_1002/w5b.h"

/* Entry i of the table 0x101a2100 (0x88 bytes), and the record k of the table 0x109e9ba0 (0x50 bytes). */
#define ENTRY(i) (GBYTES(0x101a2100) + (uint32_t)(i) * 0x88u)
#define RECORD(k) (GBYTES(0x109e9ba0) + (uint32_t)(k) * 0x50u)

// FUNCTION: HGSIM 0x100b45b4
void __cdecl Sim_100b45b4(void *obj)
{
    int32_t index = -1;

    U32(ENTRY(U32(obj, 8)), 0x18) |= 0x200;
    if (S32(obj, 8) != -1)
        index = W8Y_CALL_100b43ce(S32(obj, 8));
    if (index != -1 && (U32(ENTRY(index), 0x18) & 0x400) != 0) {
        uint8_t *rec = RECORD(U32(ENTRY(index), 0x24));

        if ((S16(rec, 0) & 0x400) != 0 && (S16(rec, 0) & 0x8000) != 0) {
            Sim_10028f38((void *)(uintptr_t)U32(ENTRY(index), 0x2c), (int)(uintptr_t)SIM_PTR(void, 0x100035ee), 0);
            Sim_100b5ad7((uint32_t)index);
        }
    }
    Sim_100b469d();
}

// FUNCTION: HGSIM 0x100b469d
void __cdecl Sim_100b469d(void)
{
    int32_t i;

    for (i = 0; i < GS32(0x1017d218); i++) {
        uint8_t *e = ENTRY(i);

        if (S32(e, 0x14) > -1) {
            if ((U32(ENTRY(S32(e, 0x14)), 0x18) & 0x200) != 0 && (U32(e, 0x18) & 0x200) == 0) {
                G32(0x1019a07c) = 1;
                W8Y_CALL_100b43ce(i);
            }
            if ((U32(ENTRY(S32(e, 0x14)), 0x18) & 0x800) != 0) {
                if ((U32(e, 0x18) & 0x800) == 0) {
                    G32(0x1019a07c) = 1;
                    U32(e, 0x18) |= 0x800;
                    Sim_100b534b(i);
                }
            } else if ((U32(e, 0x18) & 0x800) != 0 && (U32(e, 0x18) & 0x200) == 0) {
                G32(0x1019a07c) = 1;
                U32(e, 0x18) &= ~0x800u;
                if (W8Y_CALL_100b48de(i, U32(e, 0x10)) != 0) {
                    int32_t k = S32(ENTRY(i), 0x24);

                    if (k != -1 && (S16(RECORD(k), 0) & 0x8000) != 0) {
                        Sim_10028f38((void *)(uintptr_t)U32(ENTRY(i), 0x2c), (int)(uintptr_t)SIM_PTR(void, 0x100035ee),
                                     0);
                        Sim_100b5ad7((uint32_t)i);
                    }
                } else {
                    U32(e, 0x18) &= 0x800;      /* keeps only the bit just cleared: 0 (as in the original) */
                }
            }
        }
        if (G32(0x1017d228) != 0 && U32(e, 0x2c) != 0 && U32(U32(e, 0x2c), 0) == 0)
            W8Y_CALL_100897c7(U32(e, 0x28));
    }
}
