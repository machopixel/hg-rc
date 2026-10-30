#include <string.h>
#include "w6k.h"
#include "../util/access.h"

typedef void *(__stdcall *W6kHeapAllocFn)(uint32_t heap, uint32_t flags, uint32_t bytes);
typedef int (__stdcall *W6kHeapFreeFn)(uint32_t heap, uint32_t flags, uint32_t block);
typedef uint32_t (__stdcall *W6kVirtualAllocFn)(uint32_t addr, uint32_t size, uint32_t type, uint32_t protect);
typedef int (__stdcall *W6kVirtualFreeFn)(uint32_t addr, uint32_t size, uint32_t type);

#define IAT_HEAPALLOC 0x10ac5658u
#define IAT_HEAPFREE 0x10ac5634u
#define IAT_VIRTUALALLOC 0x10ac56acu
#define IAT_VIRTUALFREE 0x10ac56a8u

#define ADDR(p) ((uint32_t)(uintptr_t)(p))
#define REGION0 ADDR(GBYTES(0x10159e38))   /* the static region */
#define CRT_HEAP G32(0x10ac3714)
#define LAST_REGION G32(0x10159e3c)        /* +4 of the static region */
#define CUR_REGION G32(0x1015be58)
#define FREE_PAGES G32(0x1017e970)

// FUNCTION: HGSIM 0x10106660
uint32_t __cdecl Sim_10106660(void)
{
    uint32_t reg;
    uint32_t pages;
    uint32_t p;
    int i;

    if (G32(0x10159e48) == 0xffffffffu) {
        reg = REGION0;
    } else {
        reg = ADDR(SIM_API(W6kHeapAllocFn, IAT_HEAPALLOC)(CRT_HEAP, 0, 0x2020));
        if (reg == 0)
            return 0;
    }
    pages = SIM_API(W6kVirtualAllocFn, IAT_VIRTUALALLOC)(0, 0x400000, 0x2000, 4);
    if (pages != 0) {
        if (SIM_API(W6kVirtualAllocFn, IAT_VIRTUALALLOC)(pages, 0x10000, 0x1000, 4) != 0) {
            if (reg == REGION0) {
                if (U32(REGION0, 0) == 0)
                    U32(REGION0, 0) = REGION0;
                if (U32(REGION0, 4) == 0)
                    U32(REGION0, 4) = REGION0;
            } else {
                U32(reg, 0) = REGION0;
                U32(reg, 4) = LAST_REGION;
                LAST_REGION = reg;
                U32(U32(reg, 4), 0) = reg;
            }
            U32(reg, 0x14) = pages + 0x400000;
            U32(reg, 0x10) = pages;
            U32(reg, 8) = reg + 0x18;
            U32(reg, 0xc) = reg + 0x98;
            /* the first 16 pages are committed: 0xf0 free bytes each; the others are marked -1 */
            for (i = 0; i < 0x400; i++) {
                U32(reg, 0x18 + 8 * i) = i < 0x10 ? 0xf0u : 0xffffffffu;
                U32(reg, 0x1c + 8 * i) = 0xf1;
            }
            memset(BYTES(pages), 0, 0x10000);
            for (p = pages; p < U32(reg, 0x10) + 0x10000; p += 0x1000) {
                U32(p, 4) = 0xf0;
                U32(p, 0) = p + 8;
                U8(p, 0xf8) = 0xff;
            }
            return reg;
        }
        SIM_API(W6kVirtualFreeFn, IAT_VIRTUALFREE)(pages, 0, 0x8000);
    }
    if (reg != REGION0)
        SIM_API(W6kHeapFreeFn, IAT_HEAPFREE)(CRT_HEAP, 0, reg);
    return 0;
}

// FUNCTION: HGSIM 0x101067d0
uint32_t __cdecl Sim_101067d0(uint32_t region)
{
    uint32_t r = (uint32_t)SIM_API(W6kVirtualFreeFn, IAT_VIRTUALFREE)(U32(region, 0x10), 0, 0x8000);

    if (CUR_REGION == region)
        CUR_REGION = U32(region, 4);
    if (region == REGION0) {
        G32(0x10159e48) = 0xffffffffu;
        return r;
    }
    U32(U32(region, 4), 0) = U32(region, 0);
    U32(U32(region, 0), 4) = U32(region, 4);
    return (uint32_t)SIM_API(W6kHeapFreeFn, IAT_HEAPFREE)(CRT_HEAP, 0, region);
}

// FUNCTION: HGSIM 0x10106830
void __cdecl Sim_10106830(int32_t n)
{
    uint32_t reg = LAST_REGION;
    uint32_t done;
    uint32_t ent;
    uint32_t off;
    uint32_t cur;
    int i;

    for (;;) {
        /* a region marked unused is not stepped over (the original loops on it when it is not the last one) */
        if (U32(reg, 0x10) != 0xffffffffu) {
            done = 0;
            ent = reg + 0x2010;
            for (off = 0x3ff000; (int32_t)off >= 0; off -= 0x1000, ent -= 8) {
                if (U32(ent, 0) != 0xf0)
                    continue;
                if (!SIM_API(W6kVirtualFreeFn, IAT_VIRTUALFREE)(U32(reg, 0x10) + off, 0x1000, 0x4000))
                    continue;
                U32(ent, 0) = 0xffffffffu;
                FREE_PAGES--;
                if (U32(reg, 0xc) == 0 || U32(reg, 0xc) > ent)
                    U32(reg, 0xc) = ent;
                done++;
                if (--n == 0)
                    break;
            }
            cur = reg;
            reg = U32(reg, 4);
            if (done != 0 && U32(cur, 0x18) == 0xffffffffu) {
                for (i = 1; i < 0x400; i++) {
                    if (U32(cur, 0x18 + 8 * i) != 0xffffffffu)
                        break;
                }
                if (i == 0x400)
                    Sim_101067d0(cur);
            }
        }
        if (reg == LAST_REGION || n <= 0)
            return;
    }
}

// FUNCTION: HGSIM 0x10106960
void __cdecl Sim_10106960(uint32_t region, uint32_t p, uint8_t *mapbyte)
{
    uint32_t ent = region + (uint32_t)((int32_t)(p - U32(region, 0x10)) >> 12) * 8 + 0x18;

    U32(ent, 0) += *mapbyte;
    *mapbyte = 0;
    U32(ent, 4) = 0xf1;
    if (U32(ent, 0) == 0xf0) {
        FREE_PAGES++;
        if (FREE_PAGES == 0x20)
            Sim_10106830(0x10);
    }
}

/* Length of the run of 0 bytes starting at p + 1, plus one (the original scans until a non-zero byte). */
static uint32_t zero_run(uint32_t p)
{
    uint32_t k = 1;

    while (U8(p, k) == 0)
        k++;
    return k;
}

/* Hands out n bytes at p of the page map: moves the page's pointer and count past them (back to the start of the
 * map when they reach the end) and returns the address that belongs to p. */
static uint32_t take(uint32_t page, uint32_t p, uint32_t run, uint32_t n)
{
    if (p + n < page + 0xf8) {
        U32(page, 0) = p + n;
        U32(page, 4) = run - n;
    } else {
        U32(page, 0) = page + 8;
        U32(page, 4) = 0;
    }
    U8(p, 0) = (uint8_t)n;
    return ((p + 8) << 4) - page * 15;
}

// FUNCTION: HGSIM 0x10106c00
uint32_t __cdecl Sim_10106c00(uint32_t page, uint32_t avail, uint32_t n)
{
    uint32_t start = U32(page, 0);
    uint32_t cnt = U32(page, 4);
    uint32_t p = start;
    uint32_t c;
    uint32_t k;

    if (cnt >= n) {
        U8(start, 0) = (uint8_t)n;
        if (start + n < page + 0xf8) {
            U32(page, 0) += n;
            U32(page, 4) -= n;
        } else {
            U32(page, 4) = 0;
            U32(page, 0) = page + 8;
        }
        return ((start + 8) << 4) - page * 15;
    }
    if (U8(start + cnt, 0) != 0)
        p = start + cnt;
    /* from the page's pointer to the end of the map */
    while (p + n < page + 0xf8) {
        c = U8(p, 0);
        if (c != 0) {
            p += c;
            continue;
        }
        k = zero_run(p);
        if (k >= n)
            return take(page, p, k, n);
        if (p == start) {
            U32(page, 4) = k;
        } else {
            avail -= k;
            if (avail < n)
                return 0;
        }
        p += k;
    }
    /* from the start of the map up to the page's pointer */
    p = page + 8;
    if (p >= start)
        return 0;
    do {
        if (p + n >= page + 0xf8)
            return 0;
        c = U8(p, 0);
        if (c != 0) {
            p += c;
        } else {
            k = zero_run(p);
            if (k >= n)
                return take(page, p, k, n);
            avail -= k;
            if (avail < n)
                return 0;
            p += k;
        }
    } while (p < start);
    return 0;
}

// FUNCTION: HGSIM 0x10106e50
int32_t __cdecl Sim_10106e50(void)
{
    uint32_t reg = REGION0;
    uint32_t seen = 0;
    uint32_t pg;
    uint32_t map;
    uint32_t ent;
    uint32_t nfree;
    uint32_t mark;
    uint32_t zeros;
    uint32_t run;
    uint32_t j;
    uint32_t k;
    int i;

    do {
        if (CUR_REGION == reg)
            seen++;
        if (U32(reg, 0x10) != 0xffffffffu) {
            pg = U32(reg, 0x10);
            ent = reg + 0x18;
            nfree = 0;
            for (i = 0; i < 0x400; i++, ent += 8, pg += 0x1000) {
                map = pg + 8;
                if (U32(ent, 0) == 0xffffffffu) {
                    /* the first page marked -1 must be the one +0xc points at */
                    if (nfree == 0 && U32(reg, 0xc) != ent)
                        return -1;
                    nfree++;
                    continue;
                }
                if (U32(pg, 0) >= map + 0xf0)
                    return -2;
                if (U8(map, 0xf0) != 0xff)
                    return -3;
                j = 0;
                mark = 0;
                zeros = 0;
                run = 0;
                do {
                    if (map + j == U32(pg, 0))
                        mark++;
                    if (U8(map, j) == 0) {
                        zeros++;
                        run++;
                        j++;
                        continue;
                    }
                    if ((int32_t)run >= S32(ent, 4))
                        return -4;
                    if (mark == 1) {
                        if ((int32_t)run < S32(pg, 4))
                            return -5;
                        mark = 2;
                    }
                    run = 0;
                    for (k = j + 1; (int32_t)k < (int32_t)(j + U8(map, j)); k++) {
                        if (U8(map, k) != 0)
                            return -6;
                    }
                    j = k;
                } while ((int32_t)j < 0xf0);
                if (zeros != U32(ent, 0))
                    return -7;
                if (mark == 0)
                    return -8;
            }
        }
        reg = U32(reg, 0);
    } while (reg != REGION0);
    return seen != 0 ? 0 : -9;
}
