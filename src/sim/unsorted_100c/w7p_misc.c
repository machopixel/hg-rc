#include "w7p.h"
#include "w2e.h"
#include "w3s.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_1001/w5c.h"
#include "../unsorted_1003/fns.h"
#include "../unsorted_1003/w3s.h"
#include "../unsorted_1009/w3o.h"
#include "../unsorted_1009/w3p.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"

#ifdef __GNUC__
typedef float __attribute__((may_alias)) w7p_f32m;
#else
typedef float w7p_f32m;
#endif

#define F32(p, off) (*(w7p_f32m *)(BYTES(p) + (off)))

// FUNCTION: HGSIM 0x100c3550
int __cdecl Sim_100c3550(void *unit, float y)
{
    uint8_t seg[0x38];
    uint8_t *hit = 0;
    uint32_t flags;
    uint16_t kind;

    Sim_1009cab0((float *)seg, F32(unit, 0x58), y, F32(unit, 0x60), F32(unit, 0xdc), F32(unit, 0xe0), F32(unit, 0xe4));
    Sim_1009cbfe(seg);
    if (W7P_CALL_1002dda9(seg, &hit, U32(unit, 4)) == 0 || hit == 0)
        return 1;
    kind = U16(hit, 2);
    flags = U32(unit, 0xf0);
    if ((kind & 0x100) != 0 && (flags & 0x200) != 0)
        return U16(hit, 0x24) == (flags & 0xff);
    if ((kind & 0x200) != 0 && (flags & 0x400) != 0)
        return U16(hit, 0x24) == (flags & 0xff);
    return 0;
}

// FUNCTION: HGSIM 0x100c4035
int __cdecl Sim_100c4035(void *a, void *b, uint32_t c)
{
    int32_t *list = 0;
    int32_t entry = -1;
    int count;
    int ok;
    int i;

    if (a == 0 || b == 0)
        return -1;
    (void)*(volatile u32a *)(BYTES(b) + 0x58);
    (void)*(volatile u32a *)(BYTES(b) + 0x5c);
    (void)*(volatile u32a *)(BYTES(b) + 0x60);
    count = W7P_CALL_10033eb3(U32(a, 0x58), U32(a, 0x5c), U32(a, 0x60), c, &list);
    if (count == 0 || list == 0)
        return Sim_10033d75(F32(a, 0x58), F32(a, 0x5c), F32(a, 0x60));
    ok = 0;
    for (i = 0; i < count; i++) {
        entry = list[i];
        ok = Sim_100c3f0c(a, b, Sim_10033c42(entry));
        if (ok != 0)
            break;
    }
    if (i < count && ok != 0)
        return entry;
    return list[Sim_1009c929(count)];
}

// FUNCTION: HGSIM 0x100c4498
void __cdecl Sim_100c4498(void *unit, uint32_t a, uint32_t *slot)
{
    uint32_t record = PTRTAB(U16(unit, 0x1aa) & 0xff);
    uint32_t block = U32(unit, 0x28);
    uint32_t out[3];
    int found = 0;

    if (W7P_CALL_1003387d(a, BYTES(record) + 0x58, U32(block, 0x53c), out) == 0)
        return;
    if (*slot != 0x80000000u) {
        if (*slot != U32(unit, 0x118))
            *slot = U32(unit, 0x118);
        found = Sim_100ca417(U32(unit, 4), *slot);
    }
    if (found == -1)
        return;
    found = Sim_100ca350(U32(unit, 4), out[0], out[1], out[2]);
    Sim_100159cf(unit, 0x100);
    *slot = (uint32_t)found | 0x100;
}

static int is_space(const char *s)
{
    if (GS32(0x1015965c) > 1)
        return Crt_IsCType((int)(signed char)*s, 8);
    return (int)(U16(G32(0x10159450), (uint32_t)(int32_t)(signed char)*s * 2) & 8);
}

// FUNCTION: HGSIM 0x100c6b30
int __cdecl Str_ParseOctal(int n, const char *s)
{
    int value;

    while (is_space(s) != 0) {
        s++;
        n--;
        if (n <= 0)
            return -1;
    }
    value = 0;
    while (n > 0 && *s >= '0' && *s <= '7') {
        value = (value << 3) | (*s - '0');
        s++;
        n--;
    }
    if (n > 0 && *s != 0 && is_space(s) == 0)
        return -1;
    return value;
}

// FUNCTION: HGSIM 0x100c6e27
void *__cdecl Sim_100c6e27(const char *name)
{
    uint8_t *rec = Heap_Malloc(0x4bc);

    if (rec == 0)
        return 0;
    U32(rec, 0) = (uint32_t)(uintptr_t)Sim_100fd850(name, SIM_PTR(const char, 0x10151cf0));
    if (U32(rec, 0) == 0)
        return 0;
    U32(rec, 0x4b4) = 0;
    W7P_CALL_100c6cf5(rec);
    return rec;
}

// FUNCTION: HGSIM 0x100c6fbd
int __cdecl Sim_100c6fbd(void *rec, void *buffer, uint32_t n)
{
    uint32_t got = W7P_CALL_100fe830(buffer, 1, n, U32(rec, 0));

    if (got != n)
        return -1;
    return (int)got;
}
