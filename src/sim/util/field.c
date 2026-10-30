#include "util.h"
#include "access.h"

// FUNCTION: HGSIM 0x10008c69
void __cdecl Sim_10008c69(void *obj, uint32_t value)
{
    U32(obj, 0x30) = value;
}

// FUNCTION: HGSIM 0x10009408
void __cdecl Sim_10009408(void *obj, unsigned short v)
{
    if (obj != 0)
        U16(obj, 2) = v;
}

// FUNCTION: HGSIM 0x100096ee
uint32_t __cdecl Sim_100096ee(void *obj)
{
    return U32(obj, 0x28);
}

// FUNCTION: HGSIM 0x100096f9
void __cdecl Sim_100096f9(void *obj, uint32_t value)
{
    U32(obj, 0x28) = value;
}

// FUNCTION: HGSIM 0x100a9b4c
void __cdecl Sim_100a9b4c(void *obj, uint32_t value)
{
    U32(obj, 0x8c) = value;
    U32(obj, 0x94) |= 1;
}

// FUNCTION: HGSIM 0x100a9b71
uint32_t __cdecl Sim_100a9b71(void *obj)
{
    return U32(obj, 0x8c);
}

// FUNCTION: HGSIM 0x100a9cfb
void __cdecl Sim_100a9cfb(void *obj, uint32_t *a, uint32_t *b, uint32_t *c)
{
    *(u32a *)a = U32(obj, 0x70);
    *(u32a *)b = U32(obj, 0x74);
    *(u32a *)c = U32(obj, 0x78);
}

// FUNCTION: HGSIM 0x100a9d21
void __cdecl Sim_100a9d21(void *obj, uint32_t *a, uint32_t *b, uint32_t *c)
{
    *(u32a *)a = U32(obj, 0x30);
    *(u32a *)b = U32(obj, 0x34);
    *(u32a *)c = U32(obj, 0x38);
}

// FUNCTION: HGSIM 0x100a9d47
void __cdecl Sim_100a9d47(void *obj, uint32_t a, uint32_t b, uint32_t c)
{
    U32(obj, 0x30) = a;
    U32(obj, 0x34) = b;
    U32(obj, 0x38) = c;
    U32(obj, 0x94) |= 1;
}

// FUNCTION: HGSIM 0x100c274a
void __cdecl Sim_100c274a(void *obj, unsigned char value)
{
    U8(U32(obj, 0x54), 0x1e) = value;
}

// FUNCTION: HGSIM 0x100c48c9
void __cdecl Sim_100c48c9(void *obj)
{
    if (obj != 0 && U32(obj, 0x54) != 0) {
        U32(U32(obj, 0x54), 8) = 0;
        U8(U32(obj, 0x54), 0x54) = 0;
    }
}

// FUNCTION: HGSIM 0x100cc8ba
int __cdecl Sim_100cc8ba(int p)
{
    return p + 4;
}

// FUNCTION: HGSIM 0x100cc6e0
uint32_t __cdecl Sim_100cc6e0(void)
{
    return G32(0x1017e5b8);
}
