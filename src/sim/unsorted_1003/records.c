#include "fns.h"
#include "../simmem.h"

#define ENABLED SIM_PTR(int32_t, 0x1015EC84)
#define ROOT (*SIM_PTR(unsigned char *, 0x1015D004))
#define PTR_AT(base, off) (*(unsigned char **)((base) + (off)))

/* ROOT+0x16 points at the players' key words; ROOT+0x126 at records of 0x1e bytes with a word id at +0x1a.
 * 1.2 (0x100312c6) looks at 10 keys (its shell's 10-player game); NETL_MUTATE (checker negative only) at 1.0's 8. */
#ifdef NETL_MUTATE
#define NETL_KEYS 8
#else
#define NETL_KEYS 10
#endif
// FUNCTION: HGSIM 0x1003db56
uint32_t __cdecl Sim_1003db56(int16_t key)
{
    if (*ENABLED != 0) {
        uint32_t i;

        for (i = 0; i < NETL_KEYS; i++) {
            if (((uint16_t *)PTR_AT(ROOT, 0x16))[i] == (uint16_t)key)
                return *(uint16_t *)(PTR_AT(ROOT, 0x126) + i * 0x1e + 0x1a);
        }
    }
    return 0xFFFFFFFFu;
}

/* Index of the record (count at ROOT+0x12) whose id word equals the argument, else -1. */
// FUNCTION: HGSIM 0x1003dbc0
int __cdecl Sim_1003dbc0(uint32_t id)
{
    if (*ENABLED != 0) {
        int i;

        for (i = 0; i < *(int32_t *)(ROOT + 0x12); i++) {
            if (id == *(uint16_t *)(PTR_AT(ROOT, 0x126) + (uint32_t)i * 0x1e + 0x1a))
                return i;
        }
    }
    return -1;
}
