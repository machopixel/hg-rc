#include "w2f.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_100b/w2f.h"

// FUNCTION: HGSIM 0x100c275b
void __cdecl Sim_100c275b(void *obj)
{
    uint32_t dst = (uint32_t)(uintptr_t)BYTES(obj) + 0x25a;
    uint32_t other = U32(obj, 0x28);

    if (S32(obj, 4) == GS32(0x1017b8c8)) {
        U32(dst, 0) = G32(0x1015dc78) + 10;
    } else if (S32(obj, 0) == 1) {
        if (U32(obj, 0x27c) == 0x10151b40) {
            U32(dst, 0) = G32(0x1015dc78) + 10;
        } else if ((U16(other, 0x562) & 1) != 0) {
            U32(dst, 0) = G32(0x1015dc78) + 10;
        } else {
            U32(dst, 0) = G32(0x1015dc78) + (S32(obj, 0x24a) != 0 ? 10 : 0x5a);
        }
    } else if (S32(obj, 0) == 0xa) {
        U32(dst, 0) = G32(0x1015dc78) + 0x14;
    } else {
        U32(dst, 0) = G32(0x1015dc78) + (S32(obj, 0x24a) != 0 ? 10 : 0x5a);
    }
}

// FUNCTION: HGSIM 0x100c38fe
uint32_t __cdecl Sim_100c38fe(uint16_t w)
{
    uint32_t obj = 0;
    uint32_t sel = w & 0xf00;
    uint32_t idx = w & 0xff;

    if (sel == 0x200)
        obj = U32(PTRTAB(idx), 0x48);
    else if (sel == 0x400)
        obj = Sim_100b5386((int)G32(0x109e9ba8 + idx * 0x50));
    return obj != 0 ? U32(obj, 0x8c) : 0;
}

// FUNCTION: HGSIM 0x100cba68
uint32_t __cdecl Sim_100cba68(void)
{
    uint32_t obj = 0;
    uint32_t cur = PTRTAB(G32(0x1017b8c8));
    uint32_t sel = U32(cur, 0xf0) & 0xf00;
    uint32_t idx = U32(cur, 0xf0) & 0xff;

    if (sel == 0x200)
        obj = U32(PTRTAB(idx), 0x48);
    else if (sel == 0x400)
        obj = Sim_100b5386((int)G32(0x109e9ba8 + idx * 0x50));
    return obj;
}

// FUNCTION: HGSIM 0x100cba42
uint32_t __cdecl Sim_100cba42(void)
{
    uint32_t obj = Sim_100cba68();

    return obj != 0 ? Sim_100a9b71((void *)(uintptr_t)obj) : 0;
}
