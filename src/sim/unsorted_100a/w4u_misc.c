#include "w4u.h"
#include "../simmem.h"
#include "../unsorted_1002/w4u.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100a87f0
void __cdecl Sim_100a87f0(void)
{
    if (G32(0x1017d180) != 0) {
        Heap_Free_100285a3(BYTES(G32(0x1017d180)));
        G32(0x1017d180) = 0;
    }
}
