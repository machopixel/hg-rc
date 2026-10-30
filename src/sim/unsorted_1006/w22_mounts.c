#include "w9c.h"
#include "../util/access.h"

/*
 * Round 22, deliberate fix of an original bug (DECISIONS.md): which part each weapon slot of a gear built by the
 * design loader 0x1005bb7a sits on. Kept here on the C side, so the game's memory stays as in the original.
 */
#define W22M_MAX 512

static struct W22Mount {
    const uint8_t *slot;
    const uint8_t *gear;
    int32_t part;
    uint32_t kind;
} g_w22Mounts[W22M_MAX];

void(__cdecl *g_simWeaponOffObserver)(const uint8_t *gear, int32_t slotIndex, int32_t part);

static struct W22Mount *W22m_Find(const uint8_t *slot)
{
    int i;

    for (i = 0; i < W22M_MAX; i++)
        if (g_w22Mounts[i].slot == slot)
            return &g_w22Mounts[i];
    return 0;
}

void __cdecl Sim_NoteWeaponMount(const uint8_t *gear, const uint8_t *slot, int32_t part)
{
    struct W22Mount *m = W22m_Find(slot);

    if (m == 0)
        m = W22m_Find(0);
    if (m == 0)
        return;
    m->slot = slot;
    m->gear = gear;
    m->part = part;
    m->kind = U32(slot, 0x34);
}

void __cdecl Sim_ForgetWeaponMount(const uint8_t *slot)
{
    struct W22Mount *m = W22m_Find(slot);

    if (m != 0)
        m->slot = 0;
}

uint8_t *__cdecl Sim_FindWeaponMount(const uint8_t *gear, int32_t part)
{
    const uint8_t *first = BYTES(U32(gear, 0x84));
    int i;

    for (i = 0; i < W22M_MAX; i++) {
        const struct W22Mount *m = &g_w22Mounts[i];

        if (m->slot == 0 || m->gear != gear || m->part != part)
            continue;
        /* still that gear's slot, with the same weapon in it */
        if (m->slot < first || m->slot >= first + 8 * 0x128 || (uint32_t)(m->slot - first) % 0x128 != 0)
            continue;
        if (U32(m->slot, 0x34) != m->kind || S32(m->slot, 4) < 0)
            continue;
        return (uint8_t *)m->slot;
    }
    return 0;
}
