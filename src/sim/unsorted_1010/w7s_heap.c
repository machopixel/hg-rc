#include "w7s.h"
#include "w6k.h"
#include "../util/access.h"

#define ADDR(p) ((uint32_t)(uintptr_t)(p))

typedef uint32_t(__stdcall *W7sVirtualAllocFn)(uint32_t addr, uint32_t size, uint32_t type, uint32_t protect);

/* Region header and page layout: see w6k.h (small-block heap). The current region is 0x1015be58, the
 * static first region 0x10159e38. */

/* Looks through the entries [from, to) of a region for a page with n free map bytes; on success makes the
 * region current and returns the address, else 0. */
static uint32_t try_entries(uint32_t region, uint32_t from, uint32_t to, uint32_t page, int32_t n)
{
    uint32_t e;

    for (e = from; e < to; e += 8, page += 0x1000) {
        if (S32(e, 0) >= n && U32(e, 4) > (uint32_t)n) {
            uint32_t p = Sim_10106c00(page, U32(e, 0), (uint32_t)n);

            if (p != 0) {
                G32(0x1015be58) = region;
                U32(e, 0) -= (uint32_t)n;
                U32(region, 8) = e;
                return p;
            }
            U32(e, 4) = (uint32_t)n;
        }
    }
    return 0;
}

// FUNCTION: HGSIM 0x101069c0
uint32_t __cdecl Sim_101069c0(int32_t n)
{
    uint32_t region = G32(0x1015be58);
    uint32_t r;
    uint32_t e0;
    uint32_t page;
    uint32_t k;
    uint32_t e;
    uint32_t q;

    do {
        if (U32(region, 0x10) != 0xffffffffu) {
            uint32_t cur = U32(region, 8);

            page = ((uint32_t)((int32_t)(cur - region - 0x18) >> 3) << 12) + U32(region, 0x10);
            r = try_entries(region, cur, region + 0x2018, page, n);
            if (r != 0)
                return r;
            r = try_entries(region, region + 0x18, U32(region, 8), U32(region, 0x10), n);
            if (r != 0)
                return r;
        }
        region = U32(region, 0);
    } while (region != G32(0x1015be58));

    region = ADDR(SIM_PTR(uint8_t, 0x10159e38));
    while (U32(region, 0x10) == 0xffffffffu || U32(region, 0xc) == 0) {
        region = U32(region, 0);
        if (region == ADDR(SIM_PTR(uint8_t, 0x10159e38))) {
            r = Sim_10106660();
            if (r == 0)
                return 0;
            q = U32(r, 0x10);
            U8(q, 8) = (uint8_t)n;
            G32(0x1015be58) = r;
            U32(q, 0) = q + (uint32_t)n + 8;
            U32(q, 4) = 0xf0u - (uint32_t)n;
            U32(r, 0x18) -= (uint32_t)n & 0xff;
            return q + 0x100;
        }
    }
    e0 = U32(region, 0xc);
    page = ((uint32_t)((int32_t)(e0 - region - 0x18) >> 3) << 12) + U32(region, 0x10);
    for (k = 0, e = e0; k < 0x10 && U32(e, 0) == 0xffffffffu; e += 8)
        k++;
    if (SIM_API(W7sVirtualAllocFn, 0x10ac56ac)(page, k << 12, 0x1000, 4) != page)
        return 0;
    e = e0;
    for (q = page; k > 0; k--, q += 0x1000, e += 8) {
        U32(q, 4) = 0xf0;
        U32(q, 0) = q + 8;
        U8(q, 0xf8) = 0xff;
        U32(e, 0) = 0xf0;
        U32(e, 4) = 0xf1;
    }
    G32(0x1015be58) = region;
    while (e < region + 0x2018 && U32(e, 0) != 0xffffffffu)
        e += 8;
    U32(region, 0xc) = e < region + 0x2018 ? e : 0;
    U8(page, 8) = (uint8_t)n;
    U32(region, 8) = e0;
    U32(e0, 0) -= (uint32_t)n;
    U32(page, 4) -= (uint32_t)n;
    U32(page, 0) = page + (uint32_t)n + 8;
    return page + 0x100;
}
