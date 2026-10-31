#include "w3q.h"
#include "w2m.h"
#include "w31.h"
#include "../unsorted_1000/w2h.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"


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
    /* 1.2 (0x1000189d) loads nothing here in 3D mode: the preload loads the .TEX images */
    if (G32(0x1015ec58) == 0 && !W31_TexMode()) {
        image = (uint8_t *)RENDER_LOAD(local, (void *)(uintptr_t)G32(0x1014ceb8));
        if (G32(0x1017b954) != 0) {
            int width = S16(image, 0);
            int height = S16(image, 2);

            G32(0x1015cff4) = G32(0x1015cff4) + (uint32_t)(width * height);
        }
    }
    if ((signed char)W31_NAME(group, 0)[0] != 0) {
        G32(0x1015cfe8) = 1;
        while (!found) {
            slot++;
            if (slot >= 0x20)
                return -1;
            if ((signed char)W31_NAME(group, slot)[0] == 0)
                found = 1;
        }
    } else {
        G32(0x1015cfe8) = 0;
    }
    U32(W31_NAME(group, slot), W31_NAME_IMAGE) = (uint32_t)(uintptr_t)image;
    U16(W31_NAME(group, slot), W31_NAME_VALUE) = (uint16_t)value;
    U32(W31_NAME(group, slot), W31_NAME_KIND) = 0;
    Str_Copy((char *)W31_NAME(group, slot), local);
    return 0;
}
