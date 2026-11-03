#include "w8x.h"
#include "../render/w2m.h"
#include "../render/w33.h"
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

/* the renderer's state call [0x109e3d40], 1.0's numbering */
#define W8X_STATE RENDER_SLOT(int(__cdecl *)(unsigned, unsigned), 0x109e3d40)
/* an entry's object kind: the top four bits of the object's word +2 */
#define W8X_KIND(e) (U16(U32((e), 0), 2) & 0xf000u)
#define W8X_SPECIAL(k) ((k) == 0x8000u || (k) == 0x9000u)

/*
 * 1.2's sort for its 3DFX / PowerVR mode: one partition pass over the 8-byte entries [first, last] that, around the
 * first entry, separates the objects of kind 0x8000 / 0x9000 from the others, then the depth sort (1.0's
 * 0x100a4700) on both sides. As 1.2, the kind of an entry is only read when its object is set, and kept otherwise.
 */
// FUNCTION: HGSIM12 0x10090a85
void __cdecl Sim12_10090a85(uint32_t first, uint32_t last)
{
    uint32_t i, j, t;
    uint32_t pivot, kind = 0;   /* 1.2 leaves kind unset until an entry with an object is seen */

    if (!(last > first))
        return;
    i = first;
    j = last + 8;
    pivot = W8X_KIND(first);
    for (;;) {
        do {
            i += 8;
            if (i != 0 && U32(i, 0) != 0)
                kind = W8X_KIND(i);
        } while (i != 0 && U32(i, 0) != 0 && !W8X_SPECIAL(pivot) && !W8X_SPECIAL(kind) && i <= last);
        do {
            j -= 8;
            if (j != 0 && U32(j, 0) != 0)
                kind = W8X_KIND(j);
#ifdef W8XV_MUTATE /* the checker's negative check only: 0x9000 not special on this side */
        } while (j != 0 && U32(j, 0) != 0 && (pivot == 0x8000u) && (kind == 0x8000u) && j >= first);
#else
        } while (j != 0 && U32(j, 0) != 0 && W8X_SPECIAL(pivot) && W8X_SPECIAL(kind) && j >= first);
#endif
        if (!(i < j))
            break;
        t = U32(i, 0); U32(i, 0) = U32(j, 0); U32(j, 0) = t;
        t = U32(i, 4); U32(i, 4) = U32(j, 4); U32(j, 4) = t;
    }
    t = U32(first, 0); U32(first, 0) = U32(j, 0); U32(j, 0) = t;
    t = U32(first, 4); U32(first, 4) = U32(j, 4); U32(j, 4) = t;
    if (first < j - 8)
        W8XA_CALL_100a4700((void *)(uintptr_t)first, (void *)(uintptr_t)(j - 8));
    if (j + 8 < last)
        W8XA_CALL_100a4700((void *)(uintptr_t)(j + 8), (void *)(uintptr_t)last);
}

// FUNCTION: HGSIM 0x100a4d8e
void __cdecl Sim_100a4d8e(void)
{
    uint32_t base, list, o;
    int32_t i;

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
    if (GS32(0x1017d138) > 1) {
        /* 1.2 (0x100911ec): its 3DFX / PowerVR mode with [0x1014cb1c] 0 sorts with its own partition first */
        if (g_w33Voodoo == 0 || G32(0x1014cb1c) != 0)
            W8XA_CALL_100a4700((void *)(uintptr_t)(G32(0x1017d168) + base * 8),
                               (void *)(uintptr_t)(G32(0x1017d168) + (base + G32(0x1017d138)) * 8 - 8));
        else
            Sim12_10090a85(G32(0x1017d168) + base * 8, G32(0x1017d168) + (base + G32(0x1017d138)) * 8 - 8);
    }
    G32(0x101c9ea0) = G32(0x1017d138);
    /* 1.2: with fog on, its 3DFX / PowerVR mode sets the renderer's state 1 (1.0's 2) to 0x200000 */
    if (G32(0x1017d108) != 0 && g_w33Voodoo != 0)
        W8X_STATE(2, 0x200000);
}
