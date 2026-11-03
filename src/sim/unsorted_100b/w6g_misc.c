#include "w6g.h"
#include "../unsorted_100f/w6i.h"
#include "../../platform/plat.h"
#include "fns.h"
#include "w2h.h"
#include "w2l.h"
#include "w3o.h"
#include "w3s.h"
#include "w5c.h"
#include "w5z.h"
#include "../util/access.h"
#include "../core/w2a.h"
#include "../view/view.h"
#include "../unsorted_1001/w5y.h"
#include "../unsorted_1002/w2g.h"
#include "../unsorted_1003/w5c.h"
#include "../unsorted_1005/w3o.h"
#include "../unsorted_1008/w2i.h"
#include "../unsorted_100c/w5a.h"
#include "../unsorted_100d/w2j.h"
#include "../unsorted_100f/w2e.h"

#define PTR(v) ((void *)(uintptr_t)(v))

// FUNCTION: HGSIM 0x100b5ad7
int __cdecl Sim_100b5ad7(uint32_t id)
{
    int i;

    for (i = 0; i < GS32(0x1017d218); i++) {
        if (G32(0x101a2114u + (uint32_t)i * 0x88u) == id)
            Sim_100b5ad7((uint32_t)i);
    }
    if (id < 0x402) {
        uint8_t *rec = GBYTES(0x101a2100u + id * 0x88u);

        if (U32(rec, 0x24) < 0xfe)
            Sim_1002a16b((int)U32(rec, 0x24));
        Sim_100b4073((int32_t)id);
        return 1;
    }
    return 0;
}

// FUNCTION: HGSIM 0x100b65b7
int __cdecl Sim_100b65b7(int amount, uint32_t key)
{
    int i;
    uint8_t *entry;

    for (i = 0; i < GS32(0x1017d4fc); i++) {
        if (key == G32(0x1017d278u + (uint32_t)i * 0x14u))
            break;
    }
    entry = GBYTES(0x1017d278u + (uint32_t)i * 0x14u);
    if (i == GS32(0x1017d4fc)) {
        U32(entry, 0) = key;
        U32(entry, 4) = 0;
        G32(0x1017d4fc) = G32(0x1017d4fc) + 1;
        if (GS32(0x1017d4fc) > 0x20)
            W6G_CALL_100bc280(0x44, 0);
    }
    U32(entry, 4) = U32(entry, 4) + (uint32_t)amount;
    return S32(entry, 4) > 0xffdc ? 0 : 1;
}

/* The bucket chain of the key in the table Sim_100bda9f finds for name (shared by the two lookups below). */
static uint32_t Name_FindEntry(const uint32_t *key, const char *name)
{
    uint32_t table, entry;
    int32_t bucket;

    W6G_CALL_1010d180(key);
    W6G_CALL_1010d180(name);
    table = Sim_100bda9f(PTR(G32(0x1017d604)), (uint32_t)(uintptr_t)name, W6G_CMP_100b971d);
    bucket = (int32_t)(U32(key, 0) ^ U32(key, 4)) % S32(table, 0);
    for (entry = U32(U32(table, 0x10), (uint32_t)bucket * 4); entry != 0; entry = U32(entry, 0x1c)) {
        if (Str_CompareN((const char *)PTR(entry), (const char *)key, 8) == 0)
            return entry;
    }
    return 0;
}

// FUNCTION: HGSIM 0x100bb2b0
int __cdecl Sim_100bb2b0(const uint32_t *key, const char *name)
{
    uint32_t entry = Name_FindEntry(key, name);

    if (entry == 0)
        return 0;
    U32(entry, 8) = U32(entry, 8) - 1;
    return 1;
}

// FUNCTION: HGSIM 0x100bb350
int __cdecl Sim_100bb350(const uint32_t *key, const char *name)
{
    uint32_t entry = Name_FindEntry(key, name);

    if (entry == 0) {
        Sim_100d9fc0();
        return 0;
    }
    U32(entry, 8) = U32(entry, 8) + 1;
    return 1;
}

// FUNCTION: HGSIM 0x100bc47e
void __cdecl Sim_100bc47e(uint32_t code)
{
    char *text;

    Sim_100354ff();
    text = Sim_100bc4f7(G32(0x10151414), code);
    Sim_1002ab26();
    Sim_100556c2();
    /* ShowCursor loop, ShowWindow(SW_SHOWMINNOACTIVE) and MessageBoxA: SDL3 */
    Plat_CursorVisible(1);
    Plat_WindowShow((void *)(uintptr_t)G32(0x1017b8f0), PLAT_SHOW_MINIMIZE);
    Plat_MessageBox(0, text, (const char *)GBYTES(0x1015146c), 0x12010);
    Sim_10083495();
    W6G_CALL_100ff3b0(0xff);
}

// FUNCTION: HGSIM 0x100bc535
char *__cdecl Sim_100bc535(uint32_t code)
{
    char *text = Sim_100bc4f7(G32(0x10151418), code);

    G32(0x1017de18) = 0;
    return text;
}

// FUNCTION: HGSIM 0x100bd69c
void __cdecl Sim_100bd69c(void *obj, void(__cdecl *fn)(uint32_t))
{
    int i;

    if (obj == 0)
        return;
    for (i = 0; i < S32(obj, 4); i++) {
        if (U32(U32(obj, 8), (uint32_t)i * 4) != 0) {
            if (fn != 0)
                fn(U32(U32(obj, 8), (uint32_t)i * 4));
            else
                W6G_CALL_100fefe0(U32(U32(obj, 8), (uint32_t)i * 4));
        }
    }
    if (U32(obj, 8) != 0)
        W6G_CALL_100fefe0(U32(obj, 8));
    W6G_CALL_100fefe0((uint32_t)(uintptr_t)obj);
}

// FUNCTION: HGSIM 0x100beced
uint32_t __cdecl Sim_100beced(void *obj)
{
    uint32_t self = (uint32_t)(uintptr_t)obj;
    uint32_t partner = U32(obj, 0x28);
    uint32_t kind, target;
    int r;

    if (U32(obj, 0) != 1)
        return (self & 0xffff0000u) | 0xffff;
    if (U32(partner, 0xbdb) == 5)
        return (partner & 0xffff0000u) | 0xffff;
    r = Sim_100c4da3(obj);
    if (r == 1) {
        target = (uint32_t)(uintptr_t)GBYTES(0x10151b40);
        if (U32(obj, 0x27c) != target) {
            U32(obj, 0x27c) = target;
            U16(obj, 0x1f8) = 0xffff;
            U16(obj, 0x1fa) = 0xffff;
        }
        return (self & 0xffff0000u) | 0xffff;
    }
    if (r != 0)
        return ((uint32_t)r & 0xffff0000u) | 0xffff;
    kind = U32(obj, 0);
    if (U32(obj, 0x27c) == (uint32_t)(uintptr_t)GBYTES(0x10151ae0) + kind * 8)
        return (kind & 0xffff0000u) | 0xffff;
    U32(obj, 0x27c) = (uint32_t)(uintptr_t)GBYTES(0x10151ae0) + U32(obj, 0) * 8;
    if (S16(obj, 0x1fa) != -1)
        return (self & 0xffff0000u) | 0xffff;
    if ((U16(obj, 0x1aa) & 0x200) == 0)
        return self & 0xffff0000u;
    r = Sim_10079feb((int32_t)U32(PTRTAB(U16(obj, 0x1aa) & 0xff), 0));
    return ((uint32_t)r & 0xffff0000u) | (r != 0 ? 2u : 1u);
}

// FUNCTION: HGSIM 0x100bf186
int __cdecl Sim_100bf186(void *obj)
{
    int16_t v = (int16_t)Sim_100beced(obj);

    if (v != -1)
        return v;
    for (;;) {
        Sim_1001403a(obj, S16(obj, 0x1a8));
        v = (int16_t)Sim_100bee33(obj);
        if (v != -1)
            break;
        v = Sim_100beee4(obj);
        if (v != -1)
            break;
        v = (int16_t)Sim_100bf026(obj);
        if (v != -1)
            break;
        v = (int16_t)Sim_100bf0b8(obj);
        if (v != -1)
            break;
    }
    return v;
}
