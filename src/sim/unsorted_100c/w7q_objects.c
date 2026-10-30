#include "w7q.h"
#include "../view/view.h"
#include "../view/w2l.h"
#include "../util/util.h"
#include "../util/access.h"

/* Copies five dwords forward, one at a time (the original's rep movsd). */
static void W7q_Copy5(uint32_t dst, uint32_t src)
{
    int i;

    for (i = 0; i < 5; i++)
        U32(dst, i * 4) = U32(src, i * 4);
}

/* Runs 0x100c7deb(obj) with the record copied into the dwords at 0x1018548c and into [obj+0xec]. */
static void W7q_RunWithRecord(uint8_t *obj, uint32_t record)
{
    uint32_t global = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x1018548c);
    uint32_t savedGlobal[5];
    uint32_t savedObj[5];

    W7q_Copy5((uint32_t)(uintptr_t)savedGlobal, global);
    W7q_Copy5((uint32_t)(uintptr_t)savedObj, U32(obj, 0xec));
    W7q_Copy5(global, record);
    W7q_Copy5(U32(obj, 0xec), record);
    W7Q_CALL_100c7deb(obj);
    W7q_Copy5(global, (uint32_t)(uintptr_t)savedGlobal);
    W7q_Copy5(U32(obj, 0xec), (uint32_t)(uintptr_t)savedObj);
}

// FUNCTION: HGSIM 0x100c81ed
void __cdecl Sim_100c81ed(void *obj)
{
    uint8_t *o = obj;
    uint32_t pair;
    uint32_t record;

    if (S16(o, 4) == 0 || G32(0x10151cf8) == 0)
        return;
    pair = U32(o, 0xf4);
    if (pair != 0) {
        if (U32(o, 0xf8) != 1)
            Sim_1008f630((void **)(uintptr_t)pair);
        record = (uint32_t)(uintptr_t)Sim_1008f867(0, (const uint32_t *)(uintptr_t)pair);
        if (record != 0)
            W7q_RunWithRecord(o, record);
        else
            W7Q_CALL_100c7deb(o);
    }
    U32(o, 0xf8) = G32(0x109e89d8);
}

// FUNCTION: HGSIM 0x100c82e8
void __cdecl Sim_100c82e8(void *obj)
{
    uint8_t *o = obj;
    uint32_t pair;
    uint32_t record;

    if (S16(o, 4) == 0 || G32(0x10151cf8) == 0)
        return;
    pair = U32(o, 0xf4);
    if (pair != 0) {
        if (U32(o, 0xf8) != 0 && U32(o, 0xf8) != 3 && U32(o, 0xf8) != 4)
            Sim_1008f630((void **)(uintptr_t)pair);
        record = (uint32_t)(uintptr_t)Sim_1008f867(1, (const uint32_t *)(uintptr_t)pair);
        if (record != 0)
            W7q_RunWithRecord(o, record);
    }
    U32(o, 0xf8) = G32(0x109e89d8);
}

/* A byte that starts a two-byte character in the text mode of 0x101668e8. */
static int W7q_LeadByte(uint32_t c)
{
    if (c < 0x81 || c == 0x85 || c == 0x86 || c == 0xeb || c == 0xec)
        return 0;
    if ((c & 0xf0) > 0x90 && (c & 0xf0) < 0xe0)
        return 0;
    return c <= 0xee || c >= 0xfa;
}

// FUNCTION: HGSIM 0x100ccb0d
int __cdecl Sim_100ccb0d(const char *s, void *table)
{
    const uint8_t *p = (const uint8_t *)s;
    int total = 0;

    while ((int8_t)*p != 0) {
        if (G32(0x101668e8) != 0)
            total += (int)Sim_100f2cec(table, *p);
        else
            total += (int)W7Q_CALL_100eccf3(table, *p);
        if (G32(0x101668e8) != 0 && W7q_LeadByte(*p))
            p += 2;
        else
            p += 1;
    }
    return total;
}
