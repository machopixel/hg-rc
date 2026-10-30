#include "w6g.h"
#include "w2j.h"
#include "w5b.h"
#include "w5y.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "../unsorted_1002/w3o.h"

#define PTR(v) ((void *)(uintptr_t)(v))

static float Float_FromBits(uint32_t u)
{
    union { uint32_t u; float f; } v;
    v.u = u;
    return v.f;
}

// FUNCTION: HGSIM 0x100ac21a
int __cdecl Sim_100ac21a(int kind, void *src)
{
    uint32_t result = 0xffffffffu;
    uint8_t *entry;
    uint32_t node, obj;

    if (kind >= 0x40 || kind <= 0)
        return -1;
    switch (kind) {
    case 1:
        if (src == 0)
            return -1;
        entry = Sim_100abc5d();
        if (entry == 0)
            break;
        U32(entry, 4) = (uint32_t)kind;
        U32(entry, 8) = (uint32_t)(uintptr_t)src;
        U32(entry, 0x2c) = 0x447a0000u;           /* 1000.0f */
        result = U32(entry, 0);
        node = U32(entry, 0x30);
        obj = U32(node, 0x8c);
        U32(obj, 0x50) = U32(obj, 0x50);          /* the original reads and rewrites the dword */
        U32(entry, 0x14) = U32(src, 0x58);
        U32(obj, 0x44) = U32(entry, 0x14);
        U32(entry, 0x18) = U32(src, 0x7c);
        U32(obj, 0x48) = U32(entry, 0x18);
        U32(entry, 0x1c) = U32(src, 0x60);
        U32(obj, 0x4c) = U32(entry, 0x1c);
        U32(obj, 0x38) = 0;
        U32(obj, 0x3c) = 0;
        U32(obj, 0x40) = 0;
        Sim_100aa39f(PTR(node));
        Sim_100a9d47(PTR(node), U32(entry, 0x14), U32(entry, 0x18), U32(entry, 0x1c));
        Sim_100aa799(PTR(node));
        break;
    case 2: {
        uint32_t x, y, z;

        if (W6G_CALL_100585c4(-1) == 0 || src == 0)
            return -1;
        entry = Sim_100abc5d();
        if (entry == 0)
            break;
        U32(entry, 4) = (uint32_t)kind;
        U32(entry, 8) = (uint32_t)(uintptr_t)src;
        U32(entry, 0x2c) = 0x44fa0000u;           /* 2000.0f */
        result = U32(entry, 0);
        Sim_100a9cfb(PTR(U32(src, 0x3c)), &x, &y, &z);
        node = U32(entry, 0x30);
        obj = U32(node, 0x8c);
        U32(obj, 0x50) = U32(obj, 0x50);
        U32(entry, 0x14) = x;
        U32(obj, 0x44) = x;
        FL(entry, 0x18) = (float)Sim_1002d961(Float_FromBits(x), 0.0f, Float_FromBits(z));
        U32(obj, 0x48) = U32(entry, 0x18);
        U32(entry, 0x1c) = z;
        U32(obj, 0x4c) = z;
        U32(obj, 0x38) = 0;
        U32(obj, 0x3c) = 0;
        U32(obj, 0x40) = 0;
        Sim_100aa39f(PTR(node));
        Sim_100a9d47(PTR(node), U32(entry, 0x14), U32(entry, 0x18), U32(entry, 0x1c));
        Sim_100aa799(PTR(node));
        break;
    }
    case 6:
        if (W6G_CALL_100585c4(-1) == 0)
            return -1;
        entry = Sim_100abc5d();
        if (entry == 0)
            break;
        U32(entry, 4) = (uint32_t)kind;
        U32(entry, 8) = 0;
        U32(entry, 0x2c) = 0x3f800000u;           /* 1.0f */
        result = U32(entry, 0);
        node = U32(entry, 0x30);
        obj = U32(node, 0x8c);
        U32(obj, 0x50) = U32(obj, 0x50);
        U32(entry, 0x14) = 0;
        U32(obj, 0x44) = 0;
        U32(entry, 0x18) = 0;
        U32(obj, 0x48) = 0;
        U32(entry, 0x1c) = 0;
        U32(obj, 0x4c) = 0;
        U32(obj, 0x38) = 0;
        U32(obj, 0x3c) = 0;
        U32(obj, 0x40) = 0;
        Sim_100aa39f(PTR(node));
        Sim_100aa799(PTR(node));
        break;
    }
    return (int)result;
}

// FUNCTION: HGSIM 0x100ac620
void __cdecl Sim_100ac620(int index, uint32_t flags, const uint32_t *vals)
{
    uint8_t *rec;

    if (index < 0 || index >= GS32(0x1017d1c8))
        return;
    rec = GBYTES(0x101c57e0u + (uint32_t)index * 0x34u);
    if ((flags & 1) != 0) {
        uint32_t node;

        U32(rec, 0x14) = U32(vals, 0);
        U32(rec, 0x18) = U32(vals, 4);
        U32(rec, 0x1c) = U32(vals, 8);
        node = U32(rec, 0x30);
        if (node != 0) {
            uint32_t obj = U32(node, 0x8c);

            if (obj != 0) {
                U32(obj, 0x44) = U32(vals, 0);
                U32(obj, 0x48) = U32(vals, 4);
                U32(obj, 0x4c) = U32(vals, 8);
            }
            Sim_100a9d47(PTR(node), U32(rec, 0x14), U32(rec, 0x18), U32(rec, 0x1c));
            Sim_100aa799(PTR(node));
        }
    }
    if ((flags & 2) != 0) {
        U32(rec, 0x20) = U32(vals, 0xc);
        U32(rec, 0x28) = 0;
    }
    if ((flags & 4) != 0) {
        U32(rec, 0x24) = U32(vals, 0x10);
        U32(rec, 0x2c) = 0x3f800000u;           /* 1.0f */
    }
}
