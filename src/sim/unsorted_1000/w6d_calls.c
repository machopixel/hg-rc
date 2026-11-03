#include "w6d.h"
#include "w2e.h"
#include "fns.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1000799a
void __cdecl Sim_1000799a(void)
{
    uint32_t x = 10;
    uint32_t y = 10;
    int i;
    int j;

    for (i = 0; i < 0x100; i++) {
        for (j = 0; j < 2; j++) {
            W6D_CALL_100ef727(0x109e3e00u, x, y + (uint32_t)j, x + 3, y + (uint32_t)j, 0,
                              SIM_PTR(u16a, 0x101659e0)[i]);
        }
        if ((i + 1) % 16 == 0) {
            x = 10;
            y += 2;
        } else {
            x += 4;
        }
    }
}

// FUNCTION: HGSIM 0x100097d3
uint32_t __cdecl Sim_100097d3(void *obj)
{
    uint32_t size = 0xb4;
    uint32_t block;

    if (U32(obj, 0x28) != 0)
        size += 0xa4;
    for (block = U32(obj, 0x2c); block != 0; block = U32(block, 0xc)) {
        uint32_t n;
        uint32_t rec;

        size += 0x18;
        for (n = (uint32_t)(int32_t)S16(block, 4); n != 0; n--)
            size += 0x58;
        rec = block + U32(block, 8);
        for (n = (uint32_t)(int32_t)S16(block, 6); n != 0; n--) {
            size += (uint32_t)U16(rec, 2) + 0x28;
            rec += 0x28;
        }
    }
    return size + W6D_CALL_1002ff6e(obj);
}

// FUNCTION: HGSIM 0x1000e9c9
void __cdecl Sim_1000e9c9(const char *name, uint32_t value)
{
    int index;
    uint32_t i;

    if (G32(0x1015d0a4) == 0)
        return;
    index = Sim_1000df9c(name);
    for (i = 0; i < U32(G32(0x1015d0a4), 4); i++) {
        if (S32(G32(0x1015d0a4), 0x2d4 + i * 4) == index)
            W6D_CALL_10039708(U32(G32(0x1015d0a4), 0x94 + i * 8), value);
    }
}

// FUNCTION: HGSIM 0x1000ea40
int __cdecl Sim_1000ea40(uint32_t a, uint32_t b, const char *name, uint32_t c, uint32_t d, uint32_t e, uint32_t f)
{
    int index = Sim_1000df9c(name);
    uint32_t pick;

    if (G32(0x1015d0a4) == 0)
        return 0;
    if (U32(G32(0x1015d0a4), 0x10) != 0) {
        pick = Sim_1000edce();
        return W6D_CALL_1000e737(a, b, (uint32_t)index, 0, c, G32(0x101321b4), d, pick, 0xffffffffu, e & 0xffff, 2, f);
    }
    pick = Sim_1000edce();
    return W6D_CALL_1000e737(a, b, (uint32_t)index, 0, c, G32(0x101321b4), d, pick, 0xffffffffu, e & 0xffff, 0, 0);
}
