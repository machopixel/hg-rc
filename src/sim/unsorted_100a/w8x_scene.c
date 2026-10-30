#include "w8x.h"
#include "../render/w7o.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"

typedef int(__cdecl *W8xOfferFn)(void *obj);
#define DISPLAY_OFFER (*SIM_PTR(W8xOfferFn, 0x1014cb40))

/* Offers obj to the display routine; when it declines, draws it. Returns 1 when drawing has to stop. */
static int W8x_Offer(void *obj)
{
    G32(0x101cc2b4) += 1;
    if (DISPLAY_OFFER(obj) != 0)
        return 0;
    G32(0x101c7a9c) += 1;
    Sim_100a4510(obj, Float_OfBits(G32(0x101c7aa0)));
    return G32(0x101c9ea4) == 0 || GS32(0x1017d13c) >= GS32(0x1017d140);
}

/* The objects of the node's children (+4, next +8, object +0x8c) until one stops drawing. */
static int W8x_Children(uint32_t node)
{
    uint32_t c;

    for (c = U32(node, 4); c != 0; c = U32(c, 8))
        if (Sim_100a4845((void *)(uintptr_t)U32(c, 0x8c)) != 0)
            return 1;
    return 0;
}

// FUNCTION: HGSIM 0x100a4845
int __cdecl Sim_100a4845(void *obj)
{
    int ret = 0;
    uint32_t node;

    if (obj == 0)
        return 0;
    if (!(U16(obj, 0) & 0x1000) && W8x_Offer(obj))
        ret = 1;
    node = U32(obj, 0x28);
    if (node != 0 && U32(node, 4) != 0 && W8x_Children(node))
        ret = 1;
    return ret;
}

// FUNCTION: HGSIM 0x100a4933
int __cdecl Sim_100a4933(void *obj)
{
    int ret = 0;
    uint32_t node;

    if (obj == 0)
        return 0;
    node = U32(obj, 0x28);
    if (node == 0 || U32(node, 4) == 0)
        return W8x_Offer(obj);
    if ((U16(obj, 2) & 0x100) && U16(obj, 0x24) == G32(0x1017b8c8) && G32(0x1015ef0c) != 0 &&
        G32(0x1015d1ac) != 4) {
        /*
         * The original tests word +0x24 and [0x1015ef0c] a second time here and, when they differ, projects the
         * object for a size test; nothing changes in between, so that branch cannot run and is left out.
         */
        uint32_t saved = G32(0x10185c50), own;

        U32(G32(0x1014cae0), 0x3c) = 0x41000000;
        G32(0x10185c50) = 0x41000000;
        own = U32(U32(PTRTAB(G32(0x1017b8c8)), 0x28), 0x490);
        if (own != 0)
            Sim_100a4845((void *)(uintptr_t)U32(own, 0x8c));
        U32(G32(0x1014cae0), 0x3c) = saved;
        G32(0x10185c50) = saved;
        return 0;
    }
    if (W8x_Offer(obj))
        ret = 1;
    if (W8x_Children(node))
        ret = 1;
    return ret;
}

// FUNCTION: HGSIM 0x100a4d8e
void __cdecl Sim_100a4d8e(void)
{
    uint32_t base, list, o;
    int32_t i, k;

    if (G32(0x1017d134) == 0)
        Sim_100a88db();
    base = G32(0x1017d134);
    G32(0x1017d138) = 0;
    G32(0x1017d13c) = 0;
    G32(0x101c9ea4) = 1;
    G32(0x101cc2ac) = G32(0x1017d168) + base * 8;
    G32(0x101c672c) = 0;
    G32(0x101c673c) = 0;
    W8XA_CALL_10087d83((void *)(uintptr_t)G32(0x1014cae0));
    i = 0;
    for (;;) {
        list = SIM_PTR(u32a, 0x1036f2a0)[i];
        if (list == 0)
            break;
        if (U32(list, 8) == 0 || U32(list, 8) == U32(list, 4))
            continue;       /* the same entry again: the original never moves on from such a list */
        for (o = U32(list, 0x20); o != 0; o = U32(o, 0x20))
            if (Sim_100a4933((void *)(uintptr_t)o) != 0)
                break;
        i++;
    }
    if (GS32(0x1017d138) > 1)
        W8XA_CALL_100a4700((void *)(uintptr_t)(G32(0x1017d168) + base * 8),
                           (void *)(uintptr_t)(G32(0x1017d168) + (base + G32(0x1017d138)) * 8 - 8));
    G32(0x101c9ea0) = G32(0x1017d138);
    if (G32(0x1017b940) == 0)
        for (k = 0; k < GS32(0x1017d138); k++)
            W8XA_CALL_100a539e(U32(G32(0x1017d168), (uint32_t)k * 8));
}
