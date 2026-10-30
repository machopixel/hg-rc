#include "w9g.h"
#include "../util/access.h"
#include "../unsorted_1004/w7m.h"
#include "../unsorted_1004/w8v.h"
#include "../unsorted_100f/w6i.h"

// FUNCTION: HGSIM 0x1004e838
int __cdecl Gif_Finish(void)
{
    uint8_t rec[0x14];

    rec[0] = 0x3b;
    if (Sim_1004e719(rec) != 0)
        return 2;
    Gif_CloseFile();
    if (G32(0x1015f360) != 0) {
        Heap_FreeBlock(G32(0x1015f360));
        G32(0x1015f360) = 0;
    }
    return 0;
}
