/*
 * Round 6, letter g: functions around 0x100b5ad7..0x100bf186; helpers that are not converted (or that another
 * hook file patches) are called through their original address.
 */
#ifndef HG_SIM_UNSORTED_100B_W6G_H
#define HG_SIM_UNSORTED_100B_W6G_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* Patched by src/hooks/hooks_sim.c (TapSystemError): (code, pointer). */
#define W6G_CALL_100bc280 SIM_FN(void(__cdecl *)(int, const void *), 0x100bc280)
/* Not converted. */
#define W6G_CALL_1010d180 SIM_FN(void(__cdecl *)(const void *), 0x1010d180)
#define W6G_CALL_100fefe0 SIM_FN(void(__cdecl *)(uint32_t), 0x100fefe0)
#define W6G_CALL_100ff3b0 SIM_FN(void(__cdecl *)(int), 0x100ff3b0)
/* The string comparison 0x100b971d, handed to Sim_100bda9f by its original address like the original does. */
#define W6G_CMP_100b971d SIM_FN(int(__cdecl *)(uint32_t, uint32_t), 0x100b971d)

/* ShowCursor, ShowWindow, MessageBoxA (import slots 0x10ac5768, 0x10ac57dc, 0x10ac57e4). */
typedef int(__stdcall *W6gShowCursorFn)(int);
typedef int(__stdcall *W6gShowWindowFn)(uint32_t, int);
typedef int(__stdcall *W6gMessageBoxFn)(uint32_t, const char *, const char *, uint32_t);

/*
 * Runs itself on every record i < dword 0x1017d218 (signed) of the 0x88-byte records at 0x101a2100 whose dword +0x14
 * equals id; then, for id < 0x402: Sim_1002a16b(dword +0x24) when that is below 0xfe, Sim_100b4073(id), returns 1.
 * Returns 0 for a larger id.
 */
int __cdecl Sim_100b5ad7(uint32_t id);

/*
 * Finds key in the 0x14-byte entries at 0x1017d278 (count dword 0x1017d4fc), appending it (with 0 at +4) when it is
 * missing and reporting 0x100bc280(0x44, 0) when the count then exceeds 0x20; adds amount to the entry's dword +4
 * and returns 1 while that is at most 0xffdc (signed), else 0.
 */
int __cdecl Sim_100b65b7(int amount, uint32_t key);

/*
 * 0x1010d180 on both arguments; table = Sim_100bda9f(dword 0x1017d604, name, 0x100b971d); searches the chain
 * (key[0] ^ key[1]) % table[0] (signed) of the bucket array table+0x10, linked through +0x1c, for an entry whose
 * first 8 bytes equal key. Found: its dword +8 is decremented and 1 is returned; else 0.
 */
int __cdecl Sim_100bb2b0(const uint32_t *key, const char *name);

/* The same lookup with an increment; when nothing is found Sim_100d9fc0 (empty log function) is called and 0 returned. */
int __cdecl Sim_100bb350(const uint32_t *key, const char *name);

/*
 * Fatal error: Sim_100354ff(); text = Sim_100bc4f7(dword 0x10151414, code); Sim_1002ab26(); Sim_100556c2();
 * ShowCursor(1) until the count is not negative; ShowWindow(dword 0x1017b8f0, 7); MessageBoxA(0, text, 0x1015146c,
 * 0x12010); Sim_10083495 (empty log function); 0x100ff3b0(0xff).
 */
void __cdecl Sim_100bc47e(uint32_t code);

/* text = Sim_100bc4f7(dword 0x10151418, code); dword 0x1017de18 = 0; returns text. */
char *__cdecl Sim_100bc535(uint32_t code);

/*
 * Frees an array object (count at +4, element pointers at +8): for each non-NULL element calls fn(element) when fn
 * is given, else 0x100fefe0(element); then 0x100fefe0 on the pointer array (when set) and on the object.
 */
void __cdecl Sim_100bd69c(void *obj, void(__cdecl *fn)(uint32_t));

/*
 * 16-bit result (callers use ax; the upper half of eax is whatever the original left there and is reproduced):
 * 0xffff unless dword obj+0 is 1 and the dword +0xbdb of the object at +0x28 is not 5. Sim_100c4da3(obj) == 1: the
 * pointer +0x27c becomes 0x10151b40 (when it was not; the words +0x1f8 and +0x1fa become 0xffff), 0xffff. Result 0:
 * +0x27c becomes 0x10151ae0 + 8 * dword obj+0 (when it was not) and, when the word +0x1fa is -1, the result is 0
 * without bit 0x200 in the word +0x1aa, else 2 or 1 by Sim_10079feb(dword +0 of pointer-table entry (low byte of
 * the word +0x1aa)). Every other case gives 0xffff.
 */
uint32_t __cdecl Sim_100beced(void *obj);

/*
 * Sim_100beced(obj) when that is not -1; otherwise, until one of them gives a value other than -1:
 * Sim_1001403a(obj, word +0x1a8), Sim_100bee33, Sim_100beee4, Sim_100bf026, Sim_100bf0b8. Returns the 16-bit value
 * sign-extended.
 */
int __cdecl Sim_100bf186(void *obj);

#endif
