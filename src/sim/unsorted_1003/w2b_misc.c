#include "w2b.h"
#include "../simmem.h"

#define U32(p, off) (*(uint32_t *)((unsigned char *)(p) + (off)))

// FUNCTION: HGSIM 0x10035469
void __cdecl Sim_10035469(void)
{
    Sim_100d32e5(0);
}

// FUNCTION: HGSIM 0x10038d65
void __cdecl Sim_10038d65(int id)
{
    unsigned char *obj = (unsigned char *)(uintptr_t)*SIM_PTR(uint32_t, 0x109E3760);

    while (obj != 0) {
        if ((int)U32(obj, 0) == id) {
            uint32_t a, b, c;

            Sim_100a9cfb((void *)(uintptr_t)id, &a, &b, &c);
            U32(obj, 0x680) = a;
            U32(obj, 0x684) = b;
            U32(obj, 0x688) = c;
            return;
        }
        obj = (unsigned char *)(uintptr_t)U32(obj, 0x690);
    }
}
