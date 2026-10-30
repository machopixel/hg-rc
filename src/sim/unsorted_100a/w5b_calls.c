#include "w5b.h"
#include "w2f.h"
#include "w2l.h"
#include "../win/w4u_api.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_1002/w2h.h"
#include "../unsorted_1008/w3s.h"
#include "../unsorted_100c/w2i.h"

// FUNCTION: HGSIM 0x100a78fd
int __cdecl Sim_100a78fd(uint32_t handle, const void *buffer, uint32_t size)
{
    uint32_t left = size;
    const uint8_t *p = (const uint8_t *)buffer;

    while (left != 0) {
        uint32_t piece = left < 0x4000 ? left : 0x4000;

        if ((uint32_t)SIM_WRITE(handle, p, piece) != piece)
            return -1;
        left -= piece;
        p += piece;
    }
    return (int)size;
}

// FUNCTION: HGSIM 0x100a8068
int __cdecl Sim_100a8068(int a, int b, int c)
{
    uint32_t word = (uint32_t)c & 0xffff;

    if (word < 0xf000)
        return (int)SIM_CALL_100a7f8c(a, b, (uint16_t)word);
    return Sim_100c6f0e(BYTES(G32(0x1017d158)), (int)((word - 0xf000u) & 0xffff));
}

// FUNCTION: HGSIM 0x100a9c25
void __cdecl Sim_100a9c25(void *obj)
{
    uint32_t a, b, c;

    if ((U32(obj, 0x94) & 0x4000) != 0)
        return;
    a = U32(obj, 0x30);
    b = U32(obj, 0x34);
    c = U32(obj, 0x38);
    SIM_CALL_10060b76(BYTES(obj) + 0x3c, BYTES(obj) + 0xc);
    U32(obj, 0x30) = a;
    U32(obj, 0x34) = b;
    U32(obj, 0x38) = c;
    U32(obj, 0x94) = U32(obj, 0x94) | 0x4000;
}

// FUNCTION: HGSIM 0x100a9dc3
void __cdecl Sim_100a9dc3(void *obj, const void *src)
{
    Mem_Move(BYTES(obj) + 0xc, src, 0x30);
    SIM_CALL_10060d2e(src, BYTES(obj) + 0x3c);
    U32(obj, 0x94) = U32(obj, 0x94) | 1;
    U32(obj, 0x94) = U32(obj, 0x94) | 0x4000;
}

// FUNCTION: HGSIM 0x100aa39f
void __cdecl Sim_100aa39f(void *node)
{
    uint32_t child;

    if (U32(node, 0x8c) != 0 && (U16(U32(node, 0x8c), 2) & 0xf0) != 0x70)
        Sim_1008a46e(BYTES(U32(node, 0x8c)));
    for (child = U32(node, 4); child != 0; child = U32(child, 8))
        Sim_100aa39f(BYTES(child));
}

// FUNCTION: HGSIM 0x100aa592
void __cdecl Sim_100aa592(void *node, int32_t state)
{
    uint32_t child;

    if (U32(node, 0x8c) != 0)
        Sim_1002d890(BYTES(U32(node, 0x8c)), state);
    for (child = U32(node, 4); child != 0; child = U32(child, 8))
        Sim_100aa592(BYTES(child), state);
}

// FUNCTION: HGSIM 0x100aab03
void __cdecl Sim_100aab03(void *node, void(__cdecl *callback)(void *))
{
    uint32_t child, next;

    SIM_CALL_100aa8cb(node);
    for (child = U32(node, 4); child != 0; child = next) {
        next = U32(child, 8);
        Sim_100aab03(BYTES(child), callback);
    }
    if (U32(node, 0x8c) != 0 && callback != 0)
        callback(BYTES(U32(node, 0x8c)));
    if ((U32(node, 0x94) & 4) != 0)
        SIM_API(W4uHeapFreeFn, W4U_IAT_HEAPFREE)(W4U_HEAP, 1, node);
}

// FUNCTION: HGSIM 0x100aab8e
void *__cdecl Sim_100aab8e(void *node, const char *name)
{
    uint32_t child;

    if (U32(node, 0x90) == 0 || SIM_STRCMPI((const char *)BYTES(U32(node, 0x90)), name) != 0) {
        for (child = U32(node, 4); child != 0; child = U32(child, 8)) {
            void *found = Sim_100aab8e(BYTES(child), name);

            if (found != 0)
                return found;
        }
        return 0;
    }
    return node;
}
