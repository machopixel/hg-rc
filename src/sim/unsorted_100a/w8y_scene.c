#include "w8y.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../util/util.h"
#include "../render/w7o.h"

typedef int(__cdecl *W8yDriverTestFn)(uint32_t);

/* The deferred list: 8-byte entries from the base 0x1017d168. */
#define LIST_AT(i) (G32(0x1017d168) + (uint32_t)(i) * 8)

static void draw_all(void)
{
    int32_t i;

    if (GS32(0x1017d138) > 1)
        W8Y_CALL_100a4700(G32(0x1017d168), LIST_AT(G32(0x1017d138)) - 8);
    for (i = 0; i < GS32(0x1017d138); i++)
        W8Y_CALL_100a539e(U32(G32(0x1017d168), (uint32_t)i * 8));
}

// FUNCTION: HGSIM 0x100a4f03
void __cdecl Sim_100a4f03(void *list)
{
    uint32_t obj;

    Sim_100a88db();
    if (list == 0 || U32(list, 8) == 0 || U32(list, 8) == U32(list, 4))
        return;
    G32(0x1017d138) = 0;
    G32(0x1017d13c) = 0;
    G32(0x101c9ea4) = 1;
    G32(0x101cc2ac) = G32(0x1017d168);
    for (obj = U32(list, 0x20); obj != 0; obj = U32(obj, 0x20))
        if (W8Y_CALL_100a4933(obj) != 0)
            break;
    draw_all();
}

// FUNCTION: HGSIM 0x100a4ff0
void __cdecl Sim_100a4ff0(void *node)
{
    G32(0x1017d134) = 0;
    Sim_100a88db();
    G32(0x1017d138) = 0;
    G32(0x1017d13c) = 0;
    G32(0x101cc2ac) = LIST_AT(0);
    Sim_100a526b(node);
    if (GS32(0x1017d138) > 1)
        W8Y_CALL_100a4700(LIST_AT(0), LIST_AT(G32(0x1017d138)) - 8);
    G32(0x1017d134) = G32(0x1017d138);
}

// FUNCTION: HGSIM 0x100a512a
void __cdecl Sim_100a512a(void *node)
{
    uint32_t first;

    G32(0x1017d130) = 0;
    first = G32(0x101c9ea0);
    G32(0x1017d138) = 0;
    G32(0x1017d13c) = 0;
    G32(0x101cc2ac) = LIST_AT(first);
    Sim_100a526b(node);
    if (GS32(0x1017d138) > 1)
        W8Y_CALL_100a4700(LIST_AT(first), LIST_AT(first + G32(0x1017d138)) - 8);
    G32(0x1017d130) = G32(0x1017d138);
    G32(0x101c9ea0) = G32(0x101c9ea0) + G32(0x1017d138);
}

// FUNCTION: HGSIM 0x100a526b
void __cdecl Sim_100a526b(void *node)
{
    uint32_t obj;
    uint32_t child;

    if (node == 0 || G32(0x101c9ea4) == 0 || GS32(0x1017d13c) >= GS32(0x1017d140))
        return;
    obj = U32(node, 0x8c);
    if (obj != 0 && (U16(obj, 2) & 0xf0) != 0x20) {
        G32(0x101cc2b4) = G32(0x101cc2b4) + 1;
        if ((U16(obj, 0) & 0x1000) == 0 && ((W8yDriverTestFn)(uintptr_t)G32(0x1014cb40))(obj) == 0) {
            G32(0x101c7a9c) = G32(0x101c7a9c) + 1;
            Sim_100a4510((void *)(uintptr_t)obj, GFL(0x101c7aa0));
        }
    }
    for (child = U32(node, 4); child != 0; child = U32(child, 8))
        Sim_100a526b((void *)(uintptr_t)child);
}
