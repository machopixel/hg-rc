#include "w3s.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1005/w3s.h"

// FUNCTION: HGSIM 0x10012843
int __cdecl Sim_10012843(int index)
{
    uint32_t list[8];
    int current = Sim_10052aaf(index);
    int found = -1;
    int count = Sim_10016cb7(index, list, 1);

    if (count != 0) {
        int i;

        for (i = 0; i < count; i++) {
            uint32_t rec = list[i];

            if (S32(rec, 4) == current)
                continue;
            if ((S16(rec, 0x1c) & 6) == 0) {
                found = S32(rec, 4);
                break;
            }
        }
        Sim_100526e3(index, found);
    }
    return found;
}
