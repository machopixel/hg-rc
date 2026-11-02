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
