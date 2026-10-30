#include "w2i.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100b3520
uint32_t __cdecl Sim_100b3520(uint32_t unused)
{
    (void)unused;
    return Sim_1003a53b();
}

// FUNCTION: HGSIM 0x100b4300
void __cdecl Sim_100b4300(int index, uint32_t v)
{
    uint32_t entry = 0x101A2100u + (uint32_t)index * 0x88u;

    G32(entry + 0x18) = G32(entry + 0x18) & 0xFFFF0FFFu;
    G32(entry + 0x18) = G32(entry + 0x18) | ((v << 12) & 0xF000u);
}
