#include "w2e.h"
#include "../unsorted_1003/w2e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10011be7
void __cdecl Mono_BlankRows(void)
{
    int row;

    for (row = 3; row < 0x19; row++) {
        if (G32(0x10a3ade8) != 0)
            Mono_PutText((const char *)GBYTES(0x10130ed0), 0, (short)row, 7);
    }
}
