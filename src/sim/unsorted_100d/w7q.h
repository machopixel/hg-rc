/*
 * Round 7, letter q: functions of HGSIM.DLL at 0x100d191d..0x100db259.
 *
 * Callees that are not converted (or that another hook file patches) are called through their original
 * address (SIM_FN):
 *   0x100c398d (record, block)
 *   0x100bc280 (error code, format)            the game's error report; patched by hooks_sim.c
 *   0x100278c0 (name record, out buffer) -> object
 *   0x100599a0 (name) -> object
 */
#ifndef HG_SIM_UNSORTED_100D_W7Q_H
#define HG_SIM_UNSORTED_100D_W7Q_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

typedef void(__cdecl *W7qRecordFn)(void *record, void *block);
typedef void(__cdecl *W7qErrorFn)(int code, const void *format);
typedef void *(__cdecl *W7qFindNamedFn)(const void *name, void *out);
typedef void *(__cdecl *W7qLoadNamedFn)(const char *name);
typedef int(__cdecl *W7qVisitFn)(void *object);

#define W7Q_CALL_100c398d SIM_FN(W7qRecordFn, 0x100c398d)
#define W7Q_CALL_100bc280 SIM_FN(W7qErrorFn, 0x100bc280)
#define W7Q_CALL_100278c0 SIM_FN(W7qFindNamedFn, 0x100278c0)
#define W7Q_CALL_100599a0 SIM_FN(W7qLoadNamedFn, 0x100599a0)

/* ---- w7q_tree.c ---- */
/* Clears the dword +0 of every node of the binary tree (children at +0xc and +8, visited in that order). */
void __cdecl Sim_100d191d(void *node);
/*
 * Sets one option: 0x2b -> dword 0x10144fa0, 0x54 -> 0x101452d0, 0x58 -> 0x1015f1d8 (= value);
 * 0xc4 -> Sim_1003cbb3(0, value); 0xc5 -> dword 0x1014cb1c = (value != 0), with Sim_100203cf(1, 1) when set.
 * Other numbers do nothing.
 */
void __cdecl Sim_100d1b91(int option, uint32_t value);

/* ---- w7q_records.c ---- */
/*
 * Walks the records of the block (0x128 bytes each from [block+0x84]) in the order Sim_100d5cc1(block, 0)
 * selects them (selection dword +0x4fc, restored at the end), starting with the record after the selected one,
 * until the selection comes round again; each record with state (+8) 1 is set up as in Sim_100d666d.
 */
void __cdecl Sim_100d650f(void *block);
/*
 * For records 0..[block+0x4f8]-1: a record with state (+8) 1 whose type entry (0xd6 bytes at 0x10145508,
 * number +4) has kind (+0x10) 0x3d while its +0x20 is not -2 sets flag 0x800 in the block's word +0x562;
 * other such records get state 2. Then +0x2c comes from the type entry, +0x10 is cleared, 0x100c398d(record,
 * block) runs, and +0x18/+0x1c get the low byte / bits 0xf00 of the dword +0xf0 of [block] when the type has
 * bit 1 at +0x86 and the block's word +0x560 has 0x80 (else 0). Nothing for a null block.
 */
void __cdecl Sim_100d666d(void *block);

/* ---- w7q_names.c ---- */
/*
 * Makes sure a name group is known: for kind (+0x40) 3 looks the name (+0x20) up among the first [0x1017e6cc]
 * groups of the table 0x10ac1b00 (0xe bytes each, group number +0; names 0x300 bytes apart at 0x10a51ac0),
 * else among groups 0x100..[0x10152480]-1 with Str_CompareN. A found index goes to [out+0x40]. Otherwise a new
 * group is registered (kind 3: textures name+number for the numbers +0x4c..+0x50, Sim_10006db8, fields from
 * 0xb5 / +0x54; other kinds: Sim_10006ba7 of the name) and its index goes to [out+0x40]; the counters stop at
 * 0xff / 0x1ff. Returns 0 for a null info or an empty name, else 1.
 */
int __cdecl Sim_100da14c(void *info, void *out);
/*
 * Builds a name record (word +8 and the 12 characters +0xa of src); a record whose word is -1 or -2 and whose
 * name starts with '^' takes the next name of the list [0x1017e688] (12-byte entries, names at +8; position
 * 0x1017e690, count 0x1017f118) or reports error 0x39 (no list) / 0x3b (list used up) through 0x100bc280 and gives up. Then
 * finds the object with 0x100278c0 (error 0x34 when missing), calls visit(object), releases it with
 * Sim_10027a8f and returns visit's answer & 1 (0 when nothing was visited).
 */
int __cdecl Sim_100dafa8(void *src, W7qVisitFn visit);
/*
 * Loads the object named at +0x24 (copied to a 256-byte buffer) with 0x100599a0 (error 0x55 when missing),
 * counts it in the words 0x109f1448 and 0x109f144a, adds its word +0x84 to the words 0x109f144c..0x109f1450
 * and frees it from the heap [0x1017b8f8] with HeapFree. Returns 1.
 */
int __cdecl Sim_100db259(void *src);

#endif
