#include "w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

/* A function of the DLL, as the 32-bit address it is stored under. */
#define FN_ADDR(addr) ((uint32_t)(uintptr_t)SIM_PTR(void, addr))

// FUNCTION: HGSIM 0x10028810
void __cdecl Sim_10028810(void *obj)
{
    U32(obj, 0) = (uint32_t)(uintptr_t)obj;
    U16(obj, 6) = 0;
    U32(obj, 8) = 0;
    U32(obj, 0xc) = 0xFFFFFFFFu;
    Mem_Fill(BYTES(obj) + 0x10, 0, 0x20);
    U16(obj, 0xfc) = 0;
    U16(obj, 0xfe) = 0;
    U16(obj, 0x100) = 0;
    U16(obj, 0x102) = 0;
    U32(obj, 0x104) = FN_ADDR(0x100023a6);
    U32(obj, 0x108) = FN_ADDR(0x10002c66);
    U32(obj, 0x10c) = FN_ADDR(0x10002dfb);
    U32(obj, 0x110) = FN_ADDR(0x10001f9b);
    U32(obj, 0x114) = FN_ADDR(0x100016f4);
    U32(obj, 0x118) = FN_ADDR(0x1000160e);
    U32(obj, 0x11c) = FN_ADDR(0x100016b8);
    U32(obj, 0x120) = FN_ADDR(0x100014f1);
    U32(obj, 0x124) = FN_ADDR(0x100039f9);
    U32(obj, 0x128) = FN_ADDR(0x10001799);
    U32(obj, 0x12c) = FN_ADDR(0x10003959);
    U32(obj, 0x130) = FN_ADDR(0x10002685);
    U32(obj, 0x134) = FN_ADDR(0x100019f1);
    U32(obj, 0x138) = 0;
    U32(obj, 0x13c) = 0;
    U32(obj, 0x140) = 0;
    U32(obj, 0x144) = 0;
}

// FUNCTION: HGSIM 0x10028981
void __cdecl Sim_10028981(void *obj, const char *s)
{
    Str_CopyN((char *)BYTES(obj) + 0x10, s, 0x20);
    U8(obj, 0x2f) = 0;
}
