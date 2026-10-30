#include "w5c.h"
#include "../unsorted_1006/w3q.h"
#include "../util/access.h"

#define NODE(p, off) ((void *)(uintptr_t)U32(p, off))

// FUNCTION: HGSIM 0x100a6414
int __cdecl Sim_100a6414(const void *a, const void *table)
{
    const uint8_t *entry = BYTES(table) + (uint32_t)U8(a, U32(a, 4)) * 0x58;
    uint32_t v1 = U32(a, 0xc);
    uint32_t v2 = U32(a, 0x10);
    uint32_t v3 = U32(a, 0x14);
    uint32_t e0 = U32(entry, 0);
    uint32_t e1 = U32(entry, 4);
    uint32_t e2 = U32(entry, 8);

    return SIM_CALL_100a648c(e0, e1, e2, v1, v2, v3);
}

// FUNCTION: HGSIM 0x100a9ba7
void __cdecl Sim_100a9ba7(void *obj)
{
    uint32_t save0;
    uint32_t save1;
    uint32_t save2;

    if ((U32(obj, 0x94) & 0x8000) != 0)
        return;
    save0 = U32(obj, 0x70);
    save1 = U32(obj, 0x74);
    save2 = U32(obj, 0x78);
    SIM_CALL_10060b76(BYTES(obj) + 0x7c, BYTES(obj) + 0x4c);
    U32(obj, 0x70) = save0;
    U32(obj, 0x74) = save1;
    U32(obj, 0x78) = save2;
    U32(obj, 0x94) = U32(obj, 0x94) | 0x8000;
}

// FUNCTION: HGSIM 0x100aa127
void __cdecl Sim_100aa127(void *obj, uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    float q[4];

    SIM_CALL_1006123b(a, b, c, d, q);
    Quat_Multiply(q, (const float *)(BYTES(obj) + 0x3c), (float *)(BYTES(obj) + 0x3c));
    U32(obj, 0x94) = U32(obj, 0x94) & 0xffffbfffu;
    U32(obj, 0x94) = U32(obj, 0x94) | 1u;
}

// FUNCTION: HGSIM 0x100aae0f
void *__cdecl Sim_100aae0f(void *node, const char *name)
{
    void *found = 0;

    if (node != 0) {
        if (SIM_CALL_100fc970((const char *)(uintptr_t)U32(node, 0x90), name) != 0) {
            found = node;
        } else {
            found = Sim_100aae0f(NODE(node, 4), name);
            if (found == 0)
                found = Sim_100aae0f(NODE(node, 8), name);
        }
    }
    return found;
}

// FUNCTION: HGSIM 0x100aad46
void *__cdecl Sim_100aad46(void *node, const char *key)
{
    void *found = 0;

    if (node != 0) {
        if (U32(U32(node, 0x90), 0) == U32(key, 0)) {
            found = node;
        } else {
            found = Sim_100aae0f(NODE(node, 4), key);
            if (found == 0)
                found = Sim_100aae0f(NODE(node, 8), key);
        }
    }
    return found;
}

// FUNCTION: HGSIM 0x100aadaa
void *__cdecl Sim_100aadaa(void *node, const char *key)
{
    void *found = 0;

    if (node != 0) {
        if (U32(U32(node, 0x90), 4) == U32(key, 0)) {
            found = node;
        } else {
            found = Sim_100aae0f(NODE(node, 4), key);
            if (found == 0)
                found = Sim_100aae0f(NODE(node, 8), key);
        }
    }
    return found;
}

// FUNCTION: HGSIM 0x100ab0d6
void __cdecl Sim_100ab0d6(void *node, int level)
{
    if (node == 0 || level < 1)
        return;
    Sim_100ab0d6(NODE(node, 8), level);
    if (U32(node, 0x8c) != 0 && U16(U32(node, 0x8c), 0x26) == (uint32_t)level) {
        G32(0x101c6504) = U32(node, 0);
        SIM_CALL_100aa8cb(node);
        G32(0x101c64f4) = (uint32_t)(uintptr_t)node;
        return;
    }
    Sim_100ab0d6(NODE(node, 4), level);
}
