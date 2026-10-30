/*
 * Round 8, letter t: functions in 0x10011093..0x1001fc18. Most work on the object records of the pointer table
 * 0x109e8b40 (state word +0x1a6, the requested and current action words +0x1a8 / +0x1aa, flags +0x1ae, the
 * float distances +0xd4 / +0xd8); the action handlers are reached through the tables 0x10130e58, 0x10130e98 and
 * 0x10130eb8 of the DLL's data, called through those tables like the original. The last group keeps the queue of
 * sound requests (eight 0x6b-byte slots at 0x1015d1d0, list head [0x1015d528], next pointer at +0x67).
 * Helpers that are not converted (or that another round-8 list converts) are called through their original
 * address with SIM_FN; their prototypes are below. Floating point as in unsorted_1000/w8t.h.
 */
#ifndef HG_SIM_UNSORTED_1001_W8T_H
#define HG_SIM_UNSORTED_1001_W8T_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: returns a float in st(0) for the object (0x10010fbf). */
#define W8T_CALL_10010fbf SIM_FN(double(__cdecl *)(void *), 0x10010fbf)
/* Not converted: (group) (0x10017263). */
#define W8T_CALL_10017263 SIM_FN(void(__cdecl *)(uint32_t), 0x10017263)
/* Not converted: (obj, action word) (0x1001484e). */
#define W8T_CALL_1001484e SIM_FN(void(__cdecl *)(void *, uint32_t), 0x1001484e)
/* Not converted: (obj) (0x100bfa33). */
#define W8T_CALL_100bfa33 SIM_FN(void(__cdecl *)(void *), 0x100bfa33)
/* Not converted: (void) (0x10011c28) and (number) (0x10012af0), the two mono-screen pages of Sim_1001349a. */
#define W8T_CALL_10011c28 SIM_FN(void(__cdecl *)(void), 0x10011c28)
#define W8T_CALL_10012af0 SIM_FN(void(__cdecl *)(uint32_t), 0x10012af0)
/* The C library fabs of the DLL on a double, the result in st(0) (0x100fcb70). */
#define W8T_FABS_1001 SIM_FN(double(__cdecl *)(double), 0x100fcb70)
/* The C library fclose of the DLL (0x100fd760). */
#define W8T_FCLOSE SIM_FN(int(__cdecl *)(void *), 0x100fd760)
/* Not converted: (void) (0x1001d0fa) and (0) -> record pointer (0x1001b902). */
#define W8T_CALL_1001d0fa SIM_FN(void(__cdecl *)(void), 0x1001d0fa)
#define W8T_CALL_1001b902 SIM_FN(uint32_t(__cdecl *)(uint32_t), 0x1001b902)
/* Not converted: upper-cases its two texts in place and checks the resource (0x100ba7d5, see the round-8 notes). */
#define W8T_CALL_100ba7d5 SIM_FN(int(__cdecl *)(const char *, uint32_t, uint32_t), 0x100ba7d5)

/* The handler tables of the DLL's data: by state word (0x10130e58), by action kind (0x10130e98, 16-bit result),
 * by the byte +6 of an action entry (0x10130eb8). */
typedef void(__cdecl *W8tStateFn)(void *obj, uint32_t action);
typedef uint32_t(__cdecl *W8tKindFn)(void *obj, uint32_t key, uint32_t sel);
typedef int(__cdecl *W8tEntryFn)(void *obj, void *entry);

/*
 * Sets up an object record after loading: +0x170 = the 0x18-byte entry obj[4] of 0x10a38a80, clears several
 * fields, +0x246 = 0x10010fbf(obj) (float), Sim_1001150d, +0x1b2 from the kind +0x14 of record
 * PTRTAB(Sim_10052aaf(obj[8])) (0 for kinds 0 and 1); for objects of kind 2 or 3 also Sim_10016a6b, the byte and
 * word settings +0x1b6..+0x1d2 from +0x188..+0x196 (zeros replaced by defaults 1, 250, 250, 250, 1, 0, 5; the
 * three times are then multiplied by 100), Sim_10020456 for the player's group, Sim_1001139e and Sim_100bea42.
 */
void __cdecl Sim_10011093(void *obj);

/*
 * Per-tick update of an object (once per tick count [0x1015dc78], kept in +0x19c): objects of kind 2 (or 3 of
 * the player [0x1017b8c8]) run Sim_100117f8 and, when that finds nothing, the state handler of table
 * 0x10130e58; then Sim_1001403a(obj, word +0x1aa) and Sim_100125a3. Others only do the bookkeeping in the code.
 */
void __cdecl Sim_10011681(void *obj);

/*
 * Goes through the action list obj[0x170] (pointers to entries: word +0 kind, +2, +4, +9 codes for
 * Sim_10013fe1, +6 handler byte, +7 action): the first entry whose kind handler (table 0x10130e98, kinds 0..7)
 * gives a non-zero word (or that matches word +0x1a0 / +0x1a2) and whose entry handler (table 0x10130eb8) agrees
 * is started with Sim_10015630; returns 1 then, 0 at the end of the list. Calls Sim_100123bf first.
 */
int __cdecl Sim_100117f8(void *obj);

/*
 * Answers an attack (key Sim_10013f2a(obj)) of obj: unless flag 1 of +0x1ae, the key -1, obj already on that
 * target in state 8 or 2/3: with +0x1dc a found partner (Sim_100135d4) is started or the key dropped; then obj
 * and the other members of its group (Sim_10016cb7, up to 8) that are not the player's, not obj and not in state
 * 2/3 on that key are started with Sim_10015630(member, 2, key, 1).
 */
void __cdecl Sim_100123bf(void *obj);

/* Table 0x10130e98 kind 3: Sim_10013297 with sel (sel 0: trunc(Sim_10014061(key) / f[0x10111108])) plus the
 * state-7 follow-ups of the code; returns the 16-bit result (only ax is read by the caller). */
uint16_t __cdecl Sim_1001305e(void *obj, int16_t key, int16_t sel);

/* Table 0x10130e98 kind 1: of the targets Sim_10015018(obj, key, ..) with Sim_100152fc(obj[8], t, 2) == 0 and
 * Sim_1001403a(obj, t) the one with the smallest trunc(obj[0xd4]) below sel * 100 (sel -3: 100000000); 0 if none.
 * Only ax is defined. */
uint16_t __cdecl Sim_10013297(void *obj, int16_t key, int16_t sel);

/* Table 0x10130e98 kind 2: as Sim_10013297 with the largest trunc(obj[0xd8]) above the limit (sel -3: 0, -4:
 * obj[0x1d2], -2: obj[0x1ce], 0: trunc(Sim_10014061(key)), else sel * 100). Only ax is defined. */
uint16_t __cdecl Sim_10013372(void *obj, int16_t key, int16_t sel);

/* The mono-screen page [0x101303d0] when [0x1015d06c] is set (a new page blanks the rows first). */
void __cdecl Sim_1001349a(void);

/*
 * Picks targets for obj (see the code): *near = the nearest target t (trunc obj[0xd4]) that Sim_1001403a accepts,
 * *other the nearest without bit 0x200 or one of kind 0x200 not taken by a group member; both are dropped when
 * Sim_10013297(obj, t, sel) gives 0. Returns the group member found on the target (or 0).
 */
void *__cdecl Sim_100135d4(void *obj, int16_t key, int16_t sel, int16_t *other, int16_t *near);

/* Table 0x10130e98 kind 7: Sim_100135d4 with the limit sel (-2: obj[0x1ce] / 100, -1: obj[0x1ca] / 100, 0: from
 * Sim_10014061); returns its *other when obj[0x1dc] is set, else its *near; 0 when +0x1ae has bit 0 or 1. */
uint16_t __cdecl Sim_10013848(void *obj, int16_t key, int16_t sel);

/* The float step of obj against the limit (see the code; constants 0x10111100..0x10111148); returned in st(0). */
float __cdecl Sim_1001412d(void *obj, float limit);

/* Ends the current action of obj according to its state word +0x1a6 (see the switch), clears bit 0x10 of +0x1ae
 * and ors in the kept bits. */
void __cdecl Sim_1001457f(void *obj);

/* Starts action sel (when listed for obj, Sim_100155b7) with key on obj: Sim_10013e3c or Sim_10013e20 by flag,
 * Sim_1001457f, the action words +0x1a8 / +0x1aa from Sim_10012a72, 0x1001484e and Sim_100119a7. */
void __cdecl Sim_10015630(void *obj, int16_t sel, int16_t key, int flag);

/* Starts action sel on the group members Sim_10015018(obj, key, ..) of kind 0x200 (see the code); returns the
 * number skipped (only ax is defined). */
uint16_t __cdecl Sim_10015d30(void *obj, int16_t key, int16_t sel, int16_t other);

/* Writes the text description of an animation ("# Header", "AnimFile", tracks with position and rotation keys,
 * the control track) to the file `name` (texts at 0x101317ac..0x10131924). */
void __cdecl Anim_WriteText(const char *name, const uint8_t *model);

/* Resets the texts, tables and the 25 records at 0x10a1d140 (see the code). */
void __cdecl Sim_1001db6f(void);

/*
 * Queues the sound request of the records a and b (texts at +0, +0x10, number at +0x14) with priority prio (-1:
 * from table 0x109f1520) in a free slot of 0x1015d1d0: sorted into the list [0x1015d528] by priority, or (empty
 * list) started at once with Sim_1001fc18. Returns 1 when queued.
 */
int __cdecl Sim_1001f790(uint8_t *a, uint8_t *b, int32_t prio);

/* Plays a queued sound slot (Sim_1000e737 with the numbers of its names) or shows its text with Sim_1007df6c;
 * returns the sound slot (or 1/0 when only the text is shown). */
int32_t __cdecl Sim_1001fc18(uint8_t *slot);

#endif
