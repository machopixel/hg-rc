/*
 * Round 5, letter x: small functions around 0x10010aa0..0x1001f47d that call helpers which are not
 * converted yet (called through their address, SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1001_W5X_H
#define HG_SIM_UNSORTED_1001_W5X_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted. */
#define SIM_CALL_10010700 SIM_FN(void(__cdecl *)(int, int), 0x10010700)
#define SIM_CALL_100105cb SIM_FN(void(__cdecl *)(int, int), 0x100105cb)
#define SIM_CALL_10016117 SIM_FN(int(__cdecl *)(int, int), 0x10016117)
#define SIM_CALL_1001274b SIM_FN(void(__cdecl *)(void *), 0x1001274b)
#define SIM_CALL_10017b3e SIM_FN(void(__cdecl *)(void *, float), 0x10017b3e)
/* The second argument is only ever read as a word (all 19 reads of [ebp+0xc] in the callee are 16 bits wide). */
#define SIM_CALL_100be0f0 SIM_FN(void(__cdecl *)(void *, int), 0x100be0f0)
#define SIM_CALL_10039c00 SIM_FN(int(__cdecl *)(void), 0x10039c00)
#define SIM_CALL_1003a052 SIM_FN(int(__cdecl *)(void), 0x1003a052)
#define SIM_CALL_1004724c SIM_FN(int(__cdecl *)(const void *, uint32_t), 0x1004724c)

/* v = Sim_100109e7([0x1015d100]); when v >= 0 (signed) runs 0x10010700(v, 0). */
void __cdecl Sim_10010aa0(void);
/* When Sim_1000fe60(4) is non-zero runs 0x10016117(4, 5). */
void __cdecl Sim_10010aec(void);
/* Like Sim_10010aa0 but with 0x100105cb(v, 0). */
void __cdecl Sim_10010b65(void);
/* Runs 0x1001274b(p) when the first dword of p is 8. */
void __cdecl Sim_100125a3(void *p);
/*
 * When Sim_10079feb(first dword of obj) is non-zero runs 0x10017b3e(obj, 2000.0f); then clears the bytes +0x26 and +0x28
 * of the record the dword at obj+0x54 points at and runs 0x100be0f0(obj, value). The original pushes the low word
 * of value together with the top half of that record pointer; the callee only ever reads the low word.
 */
void __cdecl Sim_10013dbd(void *obj, int16_t value);
/* Runs 0x10039c00(); then, when [0x1015dc78] >= [0x109f4228] (signed), sets [0x109f4228] = [0x1015dc78] + 0x389. */
void __cdecl Sim_1001f454(void);
/*
 * Runs Sim_1001f399(), Cd_Shutdown() and 0x1003a052(), then 0x1004724c(0x10132220, [0x1015d1b8]) and finally
 * HeapFree(game heap [0x1017b8f8], 1, [0x1015d1b8]).
 */
void __cdecl Sim_1001f47d(void);

#endif
