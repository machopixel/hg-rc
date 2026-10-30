#include <string.h>
#include "w7s.h"
#include "w2g.h"
#include "w2h.h"
#include "w2i.h"
#include "w6k.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"
#include "../util/access.h"

#define ADDR(p) ((uint32_t)(uintptr_t)(p))

typedef int(__stdcall *W7sGetStringTypeAFn)(uint32_t locale, uint32_t infoType, const char *src, int32_t count,
                                             uint16_t *types);
typedef int(__stdcall *W7sGetStringTypeWFn)(uint32_t infoType, const uint16_t *src, int32_t count, uint16_t *types);
typedef int(__stdcall *W7sMultiByteToWideCharFn)(uint32_t cp, uint32_t flags, const char *src, int32_t count,
                                                 uint16_t *dst, int32_t dstCount);
typedef int(__stdcall *W7sWideCharToMultiByteFn)(uint32_t cp, uint32_t flags, const uint16_t *src, int32_t count,
                                                 char *dst, int32_t dstCount, const char *defChar, int32_t *used);
typedef int(__stdcall *W7sLCMapStringAFn)(uint32_t locale, uint32_t flags, const char *src, int32_t count, char *dst,
                                          int32_t dstCount);
typedef int(__stdcall *W7sLCMapStringWFn)(uint32_t locale, uint32_t flags, const uint16_t *src, int32_t count,
                                          uint16_t *dst, int32_t dstCount);
typedef int(__stdcall *W7sCompareStringAFn)(uint32_t locale, uint32_t flags, const char *s1, int32_t n1,
                                            const char *s2, int32_t n2);
typedef int(__stdcall *W7sCompareStringWFn)(uint32_t locale, uint32_t flags, const uint16_t *s1, int32_t n1,
                                            const uint16_t *s2, int32_t n2);
typedef int(__stdcall *W7sGetCPInfoFn)(uint32_t cp, void *info);
typedef uint32_t(__stdcall *W7sGetLastErrorFn)(void);

#define GET_STRING_TYPE_A SIM_API(W7sGetStringTypeAFn, 0x10ac5674)
#define GET_STRING_TYPE_W SIM_API(W7sGetStringTypeWFn, 0x10ac5678)
#define MB_TO_WIDE SIM_API(W7sMultiByteToWideCharFn, 0x10ac567c)
#define WIDE_TO_MB SIM_API(W7sWideCharToMultiByteFn, 0x10ac5670)
#define LCMAP_A SIM_API(W7sLCMapStringAFn, 0x10ac5680)
#define LCMAP_W SIM_API(W7sLCMapStringWFn, 0x10ac5684)
#define COMPARE_A SIM_API(W7sCompareStringAFn, 0x10ac56e0)
#define COMPARE_W SIM_API(W7sCompareStringWFn, 0x10ac56e4)
#define GET_CP_INFO SIM_API(W7sGetCPInfoFn, 0x10ac56b4)

/* The one-character texts the wrappers probe Windows with. */
#define PROBE_A SIM_PTR(const char, 0x1012c138)
#define PROBE_W SIM_PTR(const uint16_t, 0x1012c13c)

// FUNCTION: HGSIM 0x10104fd0
int32_t __cdecl Sim_10104fd0(uint32_t infoType, const char *src, int32_t count, uint16_t *types,
                             uint32_t codePage, uint32_t locale)
{
    uint16_t probe;
    uint32_t how = G32(0x1017e93c);
    int32_t wn;
    uint16_t *wide;
    int32_t got;
    int32_t result;

    if (how == 0) {
        if (GET_STRING_TYPE_A(0, 1, PROBE_A, 1, &probe) != 0)
            how = 2;
        else if (GET_STRING_TYPE_W(1, PROBE_W, 1, &probe) != 0)
            how = 1;
        else
            return 0;
        G32(0x1017e93c) = how;
    }
    if (how == 2) {
        if (locale == 0)
            locale = G32(0x1017e950);
        return GET_STRING_TYPE_A(locale, infoType, src, count, types);
    }
    if (how != 1)
        return (int32_t)how;
    if (codePage == 0)
        codePage = G32(0x1017e960);
    wide = 0;
    wn = MB_TO_WIDE(codePage, 9, src, count, 0, 0);
    if (wn != 0) {
        wide = W7S_FN_10100580(2, (uint32_t)wn);
        if (wide != 0) {
            got = MB_TO_WIDE(codePage, 1, src, count, wide, wn);
            if (got != 0) {
                result = GET_STRING_TYPE_W(infoType, wide, got, types);
                Heap_FreeBlock(ADDR(wide));
                return result;
            }
        }
    }
    Heap_FreeBlock(ADDR(wide));
    return 0;
}

// FUNCTION: HGSIM 0x101053c0
int32_t __cdecl Sim_101053c0(uint32_t locale, uint32_t flags, const char *src, int32_t count, char *dst,
                             int32_t dstCount, uint32_t codePage)
{
    uint32_t how = G32(0x1017e944);
    int32_t wn;
    int32_t mapped;
    int32_t result;
    uint16_t *wsrc;
    uint16_t *wdst = 0;

    if (how == 0) {
        if (LCMAP_A(0, 0x100, PROBE_A, 1, 0, 0) != 0)
            how = 2;
        else if (LCMAP_W(0, 0x100, PROBE_W, 1, 0, 0) != 0)
            how = 1;
        else
            return 0;
        G32(0x1017e944) = how;
    }
    if (count > 0) {
        count = W7S_FN_101055e0(src, count);
        how = G32(0x1017e944);
    }
    if (how == 2)
        return LCMAP_A(locale, flags, src, count, dst, dstCount);
    if (how != 1)
        return (int32_t)how;
    if (codePage == 0)
        codePage = G32(0x1017e960);
    wn = MB_TO_WIDE(codePage, 9, src, count, 0, 0);
    if (wn == 0)
        return 0;
    wsrc = Heap_Malloc((uint32_t)wn * 2u);
    if (wsrc == 0)
        return 0;
    if (MB_TO_WIDE(codePage, 1, src, count, wsrc, wn) == 0)
        goto fail;
    mapped = LCMAP_W(locale, flags, wsrc, wn, 0, 0);
    if (mapped == 0)
        goto fail;
    if (flags & 0x400) {
        if (dstCount != 0) {
            if (mapped > dstCount)
                goto fail;
            if (LCMAP_W(locale, flags, wsrc, wn, (uint16_t *)dst, dstCount) == 0)
                goto fail;
        }
        result = mapped;
    } else {
        wdst = Heap_Malloc((uint32_t)mapped * 2u);
        if (wdst == 0)
            goto fail;
        if (LCMAP_W(locale, flags, wsrc, wn, wdst, mapped) == 0)
            goto fail;
        if (dstCount == 0)
            result = WIDE_TO_MB(codePage, 0x220, wdst, mapped, 0, 0, 0, 0);
        else
            result = WIDE_TO_MB(codePage, 0x220, wdst, mapped, dst, dstCount, 0, 0);
        if (result == 0)
            goto fail;
    }
    Heap_FreeBlock(ADDR(wsrc));
    Heap_FreeBlock(ADDR(wdst));
    return result;
fail:
    Heap_FreeBlock(ADDR(wsrc));
    Heap_FreeBlock(ADDR(wdst));
    return 0;
}

/* Sets flag in the lead-byte table 0x1017eac1 for every byte of the 0-ended (first, last) pairs. */
static void flag_ranges(const uint8_t *pairs, uint32_t tableFlagAddr, uint8_t fixedFlag)
{
    for (; pairs[0] != 0 && pairs[1] != 0; pairs += 2) {
        uint32_t k;

        for (k = pairs[0]; k <= pairs[1]; k++)
            GBYTES(0x1017eac1)[k] |= tableFlagAddr != 0 ? *SIM_PTR(uint8_t, tableFlagAddr) : fixedFlag;
    }
}

// FUNCTION: HGSIM 0x10107d30
int32_t __cdecl Sim_10107d30(uint32_t cp)
{
    uint32_t c = Sim_10107f20(cp);
    uint32_t i;
    uint32_t info[5];           /* CPINFO: MaxCharSize, DefaultChar[2], LeadByte[12] at +6 */
    uint32_t off;
    uint32_t j;

    if (c == G32(0x1017ebc4))
        return 0;
    if (c == 0) {
        Sim_10107fd0();
        return 0;
    }
    for (i = 0; 0x1015be68 + i * 0x30 < 0x1015bf58; i++)
        if (G32(0x1015be68 + i * 0x30) == c)
            goto known;
    if (GET_CP_INFO(c, info) != 1) {
        if (G32(0x1017ebdc) != 0) {
            Sim_10107fd0();
            return 0;
        }
        return -1;
    }
    memset(GBYTES(0x1017eac0), 0, 0x101);
    if (info[0] > 1) {
        uint32_t k;

        if (U8(info, 6) != 0)
            flag_ranges(BYTES(info) + 6, 0, 4);
        for (k = 1; k < 0xff; k++)
            GBYTES(0x1017eac1)[k] |= 8;
        G32(0x1017ebc4) = c;
        G32(0x1017ebc8) = Sim_10107f70(c);
    } else {
        G32(0x1017ebc4) = 0;
        G32(0x1017ebc8) = 0;
    }
    G32(0x1017ebd0) = 0;
    G32(0x1017ebd4) = 0;
    G32(0x1017ebd8) = 0;
    return 0;
known:
    memset(GBYTES(0x1017eac0), 0, 0x101);
    off = i * 0x30;
    for (j = 0; j < 4; j++) {
        const uint8_t *pairs = SIM_PTR(const uint8_t, 0x1015be78 + off + j * 8);

        if (pairs[0] != 0)
            flag_ranges(pairs, 0x1015be60 + j, 0);
    }
    G32(0x1017ebc4) = c;
    G32(0x1017ebc8) = Sim_10107f70(c);
    G32(0x1017ebd0) = G32(0x1015be6c + off);
    G32(0x1017ebd4) = G32(0x1015be70 + off);
    G32(0x1017ebd8) = G32(0x1015be74 + off);
    return 0;
}

// FUNCTION: HGSIM 0x10108010
int32_t __cdecl Sim_10108010(void)
{
    return Sim_10107d30(0xfffffffdu);
}

// FUNCTION: HGSIM 0x10109200
int32_t __cdecl Sim_10109200(uint16_t *pwc, const uint8_t *s, uint32_t n)
{
    int32_t mb;

    if (s == 0 || n == 0)
        return 0;
    if (s[0] == 0) {
        if (pwc != 0)
            *pwc = 0;
        return 0;
    }
    if (G32(0x1017e950) == 0) {
        if (pwc != 0)
            *pwc = s[0];
        return 1;
    }
    if (U8(G32(0x10159450), (uint32_t)s[0] * 2u + 1u) & 0x80) {
        mb = GS32(0x1015965c);
        if (mb > 1 && (int32_t)n >= mb) {
            int ok = MB_TO_WIDE(G32(0x1017e960), 9, (const char *)s, mb, pwc, pwc != 0);

            mb = GS32(0x1015965c);
            if (ok != 0)
                return mb;
        }
        if (n < (uint32_t)mb || s[1] == 0) {
            G32(0x1017e888) = 0x2a;
            return -1;
        }
        return mb;
    }
    if (MB_TO_WIDE(G32(0x1017e960), 9, (const char *)s, 1, pwc, pwc != 0) == 0) {
        G32(0x1017e888) = 0x2a;
        return -1;
    }
    return 1;
}

// FUNCTION: HGSIM 0x101093f0
int32_t __cdecl Sim_101093f0(int32_t c)
{
    if (GS32(0x1015965c) > 1)
        return Crt_IsCType(c, 8);
    return U8(G32(0x10159450), (uint32_t)c * 2u) & 8;
}

/* Whether the first byte of s lies in one of the lead-byte ranges of the CPINFO. */
static int in_lead_range(const uint8_t *info, uint8_t c)
{
    const uint8_t *q = info + 6;

    if (q[0] == 0)
        return 0;
    for (; q[1] != 0; q += 2) {
        if (c >= q[0] && c <= q[1])
            return 1;
        if (q[2] == 0)
            break;
    }
    return 0;
}

// FUNCTION: HGSIM 0x1010afa0
int32_t __cdecl Sim_1010afa0(uint32_t locale, uint32_t flags, const char *s1, int32_t n1, const char *s2,
                             int32_t n2, uint32_t codePage)
{
    uint32_t how = G32(0x1017ed14);
    uint32_t info[5];
    int32_t w1n;
    int32_t w2n;
    uint16_t *w1;
    uint16_t *w2 = 0;
    int32_t result = 0;

    if (how == 0) {
        if (COMPARE_A(0, 0, PROBE_A, 1, PROBE_A, 1) != 0)
            G32(0x1017ed14) = 2;
        else if (COMPARE_W(0, 0, PROBE_W, 1, PROBE_W, 1) != 0)
            G32(0x1017ed14) = 1;
        else
            return 0;
    }
    if (n1 > 0)
        n1 = (int32_t)Str_LengthMax_1010b270(s1, (uint32_t)n1);
    if (n2 > 0)
        n2 = (int32_t)Str_LengthMax_1010b270(s2, (uint32_t)n2);
    how = G32(0x1017ed14);
    if (how == 2)
        return COMPARE_A(locale, flags, s1, n1, s2, n2);
    if (how != 1)
        return (int32_t)how;
    if (codePage == 0)
        codePage = G32(0x1017e960);
    if (n1 == 0 || n2 == 0) {
        int ok;

        if (n1 == n2)
            return 2;
        if (n2 > 1)
            return 1;
        if (n1 > 1)
            return 3;
        ok = GET_CP_INFO(codePage, info);
        if (ok == 0)
            return 0;
        if (n1 > 0) {
            if (info[0] < 2)
                return 3;
            return in_lead_range(BYTES(info), (uint8_t)s1[0]) ? 2 : 3;
        }
        if (n2 > 0) {
            if (info[0] < 2)
                return 1;
            return in_lead_range(BYTES(info), (uint8_t)s2[0]) ? 2 : 1;
        }
    }
    w1n = MB_TO_WIDE(codePage, 9, s1, n1, 0, 0);
    if (w1n == 0)
        return 0;
    w1 = Heap_Malloc((uint32_t)w1n * 2u);
    if (w1 == 0)
        return 0;
    if (MB_TO_WIDE(codePage, 1, s1, n1, w1, w1n) != 0) {
        w2n = MB_TO_WIDE(codePage, 9, s2, n2, 0, 0);
        if (w2n != 0) {
            w2 = Heap_Malloc((uint32_t)w2n * 2u);
            if (w2 != 0 && MB_TO_WIDE(codePage, 1, s2, n2, w2, w2n) != 0)
                result = COMPARE_W(locale, flags, w1, w1n, w2, w2n);
        }
    }
    Heap_FreeBlock(ADDR(w1));
    Heap_FreeBlock(ADDR(w2));
    return result;
}

// FUNCTION: HGSIM 0x1010c4b0
int32_t __cdecl Sim_1010c4b0(char *s, const uint16_t *pwcs, uint32_t n)
{
    int32_t used = 0;
    int32_t r;
    uint32_t i;

    if (s != 0 && n == 0)
        return 0;
    if (s == 0) {
        if (G32(0x1017e950) == 0)
            return Wstr_Length_1010b5a0(pwcs);
        r = WIDE_TO_MB(G32(0x1017e960), 0x220, pwcs, -1, 0, 0, 0, &used);
        if (r == 0 || used != 0)
            goto bad;
        return r - 1;
    }
    if (G32(0x1017e950) == 0) {
        for (i = 0; i < n; i++) {
            if (pwcs[i] > 0xff)
                goto bad;
            s[i] = (char)pwcs[i];
            if (pwcs[i] == 0)
                return (int32_t)i;
        }
        return (int32_t)i;
    }
    if (G32(0x1015965c) == 1) {
        uint32_t cnt = n;

        if (n != 0)
            cnt = W7S_FN_1010c6a0(pwcs, n);
        r = WIDE_TO_MB(G32(0x1017e960), 0x220, pwcs, (int32_t)cnt, s, (int32_t)cnt, 0, &used);
        if (r == 0 || used != 0)
            goto bad;
        if (s[r - 1] == 0)
            return r - 1;
        return r;
    }
    r = WIDE_TO_MB(G32(0x1017e960), 0x220, pwcs, -1, s, (int32_t)n, 0, &used);
    if (r != 0) {
        if (used != 0)
            goto bad;
        return r - 1;
    }
    if (used != 0 || SIM_API(W7sGetLastErrorFn, 0x10ac5600)() != 0x7a)
        goto bad;
    i = (uint32_t)r;
    while (i < n) {
        char tmp[8];            /* the original uses its own argument slots here: MB_CUR_MAX bytes */
        int32_t k;
        int32_t j;

        k = WIDE_TO_MB(G32(0x1017e960), 0, pwcs, 1, tmp, GS32(0x1015965c), 0, &used);
        if (k == 0 || used != 0)
            goto bad;
        if ((uint32_t)k + i > n)
            break;
        for (j = 0; j < k; j++, i++) {
            s[i] = tmp[j];
            if (tmp[j] == 0)
                return (int32_t)i;
        }
        pwcs++;
    }
    return (int32_t)i;
bad:
    G32(0x1017e888) = 0x2a;
    return -1;
}

// FUNCTION: HGSIM 0x1010d180
char *__cdecl Crt_StrUpper(char *s)
{
    char *copy = 0;
    int32_t len;

    if (G32(0x1017e950) == 0) {
        char *p;

        for (p = s; *p != 0; p++)
            if (*p >= 'a' && *p <= 'z')
                *p -= 0x20;
        return s;
    }
    len = Sim_101053c0(G32(0x1017e950), 0x200, s, -1, 0, 0, 0);
    if (len != 0) {
        copy = Heap_Malloc((uint32_t)len);
        if (copy != 0 && Sim_101053c0(G32(0x1017e950), 0x200, s, -1, copy, len, 0) != 0)
            memcpy(s, copy, strlen(copy) + 1);
    }
    Heap_FreeBlock(ADDR(copy));
    return s;
}
