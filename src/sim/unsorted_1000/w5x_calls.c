#include "w5x.h"
#include "w2e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10006530
void __cdecl Sim_10006530(void)
{
    if (G32(0x1015cff0) != 0) {
        SIM_CALL_100bb2b0(GBYTES(0x10a3ae80), G32(0x1014cf0c));
        SIM_CALL_100b9203(GBYTES(0x10a3ae80), G32(0x1014cf0c));
        G32(0x1015cff0) = 0;
    }
}

// FUNCTION: HGSIM 0x1000e983
void __cdecl Sim_1000e983(const char *name, uint32_t a, uint32_t b, uint32_t c)
{
    int index = Sim_1000df9c(name);

    SIM_CALL_1000e737(0, 0, (uint32_t)index, 0, a, G32(0x101321b4), b, c, 0xffffffffu, 0x450, 0, 0);
}

// FUNCTION: HGSIM 0x1000eaeb
void __cdecl Sim_1000eaeb(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
    SIM_CALL_1000eb12(a0, a1, a2, a3, a4, 0x32, a5);
}

// FUNCTION: HGSIM 0x1000ec60
void __cdecl Sim_1000ec60(const char *name, uint32_t a, uint32_t b, int which, uint32_t flags)
{
    int index = Sim_1000df9c(name);

    SIM_CALL_1000e737(0, 0, (uint32_t)index, 0, a, G32(0x101321b4), b,
                      SIM_PTR(u32a, 0x1012df30u + (uint32_t)which * 4u)[0], 0xffffffffu, flags | 0x2000u, 0, 0);
}

// FUNCTION: HGSIM 0x1000ed52
void __cdecl Sim_1000ed52(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
    Sim_1000eaeb(a0, a1, a2, a3, a4, a5);
}
