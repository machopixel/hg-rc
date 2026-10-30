#include "w3q.h"
#include "w2m.h"
#include "../unsorted_1000/w2h.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

#define NAME_TABLE 0x10a51ac0 /* groups of 32 records of 0x18 bytes: name, word +0x12, image +0x14 */

// FUNCTION: HGSIM 0x10006ba7
int __cdecl Sim_10006ba7(const char *name, int group, int value)
{
    char local[0x20];
    int slot = 0;
    int found = 0;
    uint8_t *image = 0;

    Str_Copy(local, name);
    if (G32(0x1014cb38) != 0) {
        if ((signed char)local[0] == 'W' || (signed char)local[0] == 'w') {
            if ((signed char)local[1] == 'X' || (signed char)local[1] == 'x')
                local[1] = 'G';
        }
    }
    if (GS32(0x1012da30) == -2)
        Sim_10007020();
    if (group == -1) {
        if (G32(0x1015cfe4) != 0) {
            G32(0x1015cfe0) = G32(0x1015cfe0) + 1;
            G32(0x1015cfe4) = 0;
        }
        group = GS32(0x1015cfe0);
    }
    if (name == 0 || group < 0 || group >= 0x200)
        return -1;
    if (G32(0x1015ec58) == 0) {
        image = (uint8_t *)RENDER_LOAD(local, (void *)(uintptr_t)G32(0x1014ceb8));
        if (G32(0x1017b954) != 0) {
            int width = S16(image, 0);
            int height = S16(image, 2);

            G32(0x1015cff4) = G32(0x1015cff4) + (uint32_t)(width * height);
        }
    }
    if ((signed char)GBYTES(NAME_TABLE)[(uint32_t)group * 0x300] != 0) {
        G32(0x1015cfe8) = 1;
        while (!found) {
            slot++;
            if (slot >= 0x20)
                return -1;
            if ((signed char)GBYTES(NAME_TABLE)[(uint32_t)group * 0x300 + (uint32_t)slot * 0x18] == 0)
                found = 1;
        }
    } else {
        G32(0x1015cfe8) = 0;
    }
    U32(GBYTES(NAME_TABLE) + (uint32_t)group * 0x300 + (uint32_t)slot * 0x18, 0x14) = (uint32_t)(uintptr_t)image;
    U16(GBYTES(NAME_TABLE) + (uint32_t)group * 0x300 + (uint32_t)slot * 0x18, 0x12) = (uint16_t)value;
    Str_Copy((char *)GBYTES(NAME_TABLE) + (uint32_t)group * 0x300 + (uint32_t)slot * 0x18, local);
    return 0;
}
