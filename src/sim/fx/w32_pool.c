#include "w32.h"
#include "../unsorted_1009/w2l.h"
#include "../unsorted_100a/w5x.h"
#include "../util/access.h"

#define W32_CALL_SPRINTF SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)

// FUNCTION: HGSIM 0x10090d15
int32_t __cdecl Sim_10090d15(int32_t value)
{
    uint32_t node;
    int32_t i;

    if (value == -1)
        return GS32(0x1014b9b8);
    if (value == 0 && G32(0x1017cc28) != 0) {
        G32(0x10217c00) = 0;
        for (node = G32(0x1017cc28); node != 0; node = U32(node, 4)) {
            G32(0x10215be0u + G32(0x10217c00) * 4) = U32(node, 8);
            G32(0x10217c00) = G32(0x10217c00) + 1;
        }
        for (i = 0; i < GS32(0x10217c00); i++) {
            uint8_t *record = SIM_PTR(uint8_t, 0x10217c20u + G32(0x10215be0u + (uint32_t)i * 4) * W32_REC_SIZE);

            U16(record, 0x10) = (uint16_t)(U16(record, 0x10) & 0xfffe);
            Sim_100aa351((void *)(uintptr_t)U32(record, W32_REC_OBJ));
#ifdef W32P_MUTATE /* the checker's negative check only: the record is not freed */
            continue;
#endif
            Sim_10090513((int)U32(record, 8));
        }
        G32(0x1017cc24) = 0;
    }
    G32(0x1014b9b8) = (uint32_t)value;
    W32_CALL_SPRINTF(SIM_PTR(char, 0x10217b40), SIM_PTR(const char, 0x1014b9bc), GS32(0x1014b9b8));
    return value;
}
