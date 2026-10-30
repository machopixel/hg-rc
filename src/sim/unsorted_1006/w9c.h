/*
 * Round 9, letter c: the state handler 0x1006bf33 of an object record (see unsorted_1006/w8w.h).
 */
#ifndef HG_SIM_UNSORTED_1006_W9C_H
#define HG_SIM_UNSORTED_1006_W9C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9c_event.c ---- */
/*
 * Among the eight 0x128-byte records at [s+0x84], the first one with dword +4 >= 0 and dword +0x30 == code is
 * closed: dword +0 = 0, +8 = -1, +0x114 = 1, +0x20 = -2. When the object [s] belongs to the current player
 * (dword +4 of it is [0x1017b8c8]), a message Sim_100203cf(n, 3) is sent first, n chosen by the record's +4
 * (0..0x2d; see the switch). The first argument is not used.
 */
void __cdecl Sim_1006bf33(uint32_t unused, int32_t code, void *s);

/* ---- w22_mounts.c (round 22, deliberate fix of an original bug; DECISIONS.md round 22) ----
 * The design loader 0x1005bb7a gives a weapon slot the mount code 7 / 3 / 4 / 2 (Sim_1005b923), but the part it
 * sits on is the part code of its kind, and 0x1006bf33 searches for that part code: no slot of such a design is
 * ever switched off when its part is destroyed. Sim_NoteWeaponMount (from Sim_1005bb7a) records the slot's part,
 * Sim_ForgetWeaponMount (from Sim_1005cd4a, whose codes match) drops a stale record, and Sim_1006bf33 switches off
 * the recorded slot of the destroyed part when its own search finds none. The table lives on the C side only.
 */
void __cdecl Sim_NoteWeaponMount(const uint8_t *gear, const uint8_t *slot, int32_t part);
void __cdecl Sim_ForgetWeaponMount(const uint8_t *slot);
/* the recorded slot of gear's part, while it still holds the same weapon; else null */
uint8_t *__cdecl Sim_FindWeaponMount(const uint8_t *gear, int32_t part);
/* observation only (the damage log): a slot switched off by the fix, with its index 0..7 and the part */
extern void(__cdecl *g_simWeaponOffObserver)(const uint8_t *gear, int32_t slotIndex, int32_t part);

#endif
