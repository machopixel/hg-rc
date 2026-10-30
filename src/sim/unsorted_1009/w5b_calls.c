#include "w5b.h"

// FUNCTION: HGSIM 0x1009ff33
uint32_t __cdecl Sim_1009ff33(void *file, char *buf, uint32_t size)
{
    uint32_t length;
    uint32_t got = 0;

    for (length = 0; length < size - 1; length++) {
        got = SIM_FREAD(buf + length, 1, 1, file);
        if (got == 0 || buf[length] == '\n')
            break;
    }
    if (length != 0 || got != 0) {
        length++;
        buf[length] = 0;
    }
    return length;
}
