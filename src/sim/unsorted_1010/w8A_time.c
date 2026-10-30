#include <string.h>
#include "w8A.h"
#include "w7s.h"
#include "w7r.h"
#include "w6k.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

#define ADDR(p) ((uint32_t)(uintptr_t)(p))
#define ZONE GS32(0x1015c020)     /* time zone in seconds */
#define DAYLIGHT GS32(0x1015c024) /* daylight saving used */
#define DSTBIAS GS32(0x1015c028)  /* daylight bias in seconds */

typedef uint32_t(__stdcall *W8aGetTimeZoneInformationFn)(void *info);

static int32_t sub32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static int32_t add32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

// FUNCTION: HGSIM 0x101071e0
uint8_t *__cdecl Crt_LocalTime(const int32_t *t)
{
    uint8_t *tm;
    int32_t v, q;

    if (*t < 0)
        return 0;
    Sim_10109dd0();
    v = *t;
    if (v > 0x3f480 && v < 0x7ffc0b7f) {
        int32_t lt = sub32(v, ZONE);

        tm = W8A_FN_1010aa60(&lt);
        if (DAYLIGHT != 0 && Sim_1010a0a0(tm) != 0) {
            lt = sub32(lt, DSTBIAS);
            tm = W8A_FN_1010aa60(&lt);
            S32(tm, 0x20) = 1;
        }
        return tm;
    }
    /* near the ends of the range: convert the plain time and correct the fields */
    tm = W8A_FN_1010aa60(t);
    v = S32(tm, 0);
    if (Sim_1010a0a0(tm) != 0)
        v = sub32(v, DSTBIAS);
    v = sub32(v, ZONE);
    S32(tm, 0) = v % 60;
    if (S32(tm, 0) < 0) {
        S32(tm, 0) += 60;
        v = sub32(v, 60);
    }
    v = add32(v / 60, S32(tm, 4));
    S32(tm, 4) = v % 60;
    if (S32(tm, 4) < 0) {
        S32(tm, 4) += 60;
        v = sub32(v, 60);
    }
    v = add32(v / 60, S32(tm, 8));
    S32(tm, 8) = v % 24;
    if (S32(tm, 8) < 0) {
        S32(tm, 8) += 24;
        v = sub32(v, 24);
    }
    q = v / 24;
    if (q > 0) {
        S32(tm, 0x18) = add32(q, S32(tm, 0x18)) % 7;
        S32(tm, 0xc) = add32(S32(tm, 0xc), q);
        S32(tm, 0x1c) = add32(S32(tm, 0x1c), q);
    } else if (q < 0) {
        S32(tm, 0x18) = add32(add32(q, S32(tm, 0x18)), 7) % 7;
        S32(tm, 0xc) = add32(S32(tm, 0xc), q);
        if (S32(tm, 0xc) <= 0) {
            S32(tm, 0xc) = add32(S32(tm, 0xc), 31);
            S32(tm, 0x1c) = 0x16c;
            S32(tm, 0x10) = 0xb;
            S32(tm, 0x14) = sub32(S32(tm, 0x14), 1);
        } else {
            S32(tm, 0x1c) = add32(S32(tm, 0x1c), q);
        }
    }
    return tm;
}

static int is_digit_byte(char c)
{
    return c >= '0' && c <= '9';
}

// FUNCTION: HGSIM 0x10109df0
uint32_t __cdecl Crt_TzSet(void)
{
    const char *tz;
    const char *last;
    char *copy;
    int32_t neg = 0;
    int32_t zone;
    uint32_t r;

    G32(0x1017ec28) = 0;
    G32(0x1015c0c8) = 0xffffffffu;
    G32(0x1015c0b8) = 0xffffffffu;
    tz = Crt_GetEnv(SIM_PTR(const char, 0x1012c480)); /* "TZ" */
    if (tz == 0) {
        uint32_t stdbias;

        if (SIM_API(W8aGetTimeZoneInformationFn, 0x10ac564c)(SIM_PTR(void, 0x1017ec30)) == 0xffffffffu)
            return 0xffffffffu;
        stdbias = G32(0x1017ec84);
        G32(0x1017ec28) = 1;
        G32(0x1015c020) = G32(0x1017ec30) * 60u;
        if (*SIM_PTR(u16a, 0x1017ec76) != 0)
            G32(0x1015c020) = G32(0x1015c020) + stdbias * 60u;
        if (*SIM_PTR(u16a, 0x1017ecca) != 0 && G32(0x1017ecd8) != 0) {
            G32(0x1015c024) = 1;
            G32(0x1015c028) = (G32(0x1017ecd8) - stdbias) * 60u;
        } else {
            G32(0x1015c024) = 0;
            G32(0x1015c028) = 0;
        }
        Sim_1010c4b0((char *)(uintptr_t)G32(0x1015c0b0), SIM_PTR(const uint16_t, 0x1017ec34), 0x40);
        r = (uint32_t)Sim_1010c4b0((char *)(uintptr_t)G32(0x1015c0b4), SIM_PTR(const uint16_t, 0x1017ec88), 0x40);
        U8(G32(0x1015c0b4), 0x3f) = 0;
        U8(G32(0x1015c0b0), 0x3f) = 0;
        return r;
    }
    if (*tz == 0)
        return ADDR(tz);
    last = (const char *)(uintptr_t)G32(0x1017ecdc);
    if (last != 0 && strcmp(tz, last) == 0)
        return 0;
    Heap_FreeBlock(ADDR(last));
    copy = Heap_Malloc((uint32_t)strlen(tz) + 1u);
    G32(0x1017ecdc) = ADDR(copy);
    if (copy == 0)
        return 0;
    memcpy(copy, tz, strlen(tz) + 1);
    Str_CopyN((char *)(uintptr_t)G32(0x1015c0b0), tz, 3);
    U8(G32(0x1015c0b0), 3) = 0;
    tz += 3;
    if (*tz == '-') {
        neg = 1;
        tz++;
    }
    zone = (int32_t)((uint32_t)Crt_AToL(tz) * 3600u);
    G32(0x1015c020) = (uint32_t)zone;
    while (*tz == '+' || is_digit_byte(*tz))
        tz++;
    if (*tz == ':') {
        tz++;
        zone = (int32_t)((uint32_t)zone + (uint32_t)Crt_AToL(tz) * 60u);
        G32(0x1015c020) = (uint32_t)zone;
        while (is_digit_byte(*tz))
            tz++;
        if (*tz == ':') {
            tz++;
            zone = (int32_t)((uint32_t)zone + (uint32_t)Crt_AToL(tz));
            G32(0x1015c020) = (uint32_t)zone;
            while (is_digit_byte(*tz))
                tz++;
        }
    }
    if (neg)
        G32(0x1015c020) = 0u - (uint32_t)zone;
    G32(0x1015c024) = (uint32_t)(int32_t)*tz;
    if (*tz != 0) {
        Str_CopyN((char *)(uintptr_t)G32(0x1015c0b4), tz, 3);
        U8(G32(0x1015c0b4), 3) = 0;
        return G32(0x1015c0b4);
    }
    U8(G32(0x1015c0b4), 0) = 0;
    return 0;
}
