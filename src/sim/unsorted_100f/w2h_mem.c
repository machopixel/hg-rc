#include "w2h.h"

// FUNCTION: HGSIM 0x100fded0
void __cdecl Mem_Swap(uint8_t *a, uint8_t *b, uint32_t count)
{
    if (a == b)
        return;
    while (count != 0) {
        uint8_t t = *b;

        *b = *a;
        *a = t;
        a++;
        b++;
        count--;
    }
}
