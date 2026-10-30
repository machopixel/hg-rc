/*
 * Round 5, letter b: small functions in 0x100104fd..0x10016d83 that call helpers which are not converted
 * yet (called through their address with SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1001_W5B_H
#define HG_SIM_UNSORTED_1001_W5B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted. */
#define SIM_CALL_10016117 SIM_FN(int(__cdecl *)(int, int), 0x10016117)
#define SIM_CALL_100200d1 SIM_FN(void(__cdecl *)(int, int), 0x100200d1)
/* The original does not say what it returns (void); its eax is kept by Sim_1001393e, which returns it with a word replaced. */
#define SIM_CALL_10020256 SIM_FN(int(__cdecl *)(int, int), 0x10020256)
#define SIM_CALL_10017b3e SIM_FN(void(__cdecl *)(void *, float), 0x10017b3e)
/* The second argument is only ever read as a word (all its uses in the callee are 16 bits wide). */
#define SIM_CALL_10016de7 SIM_FN(void(__cdecl *)(int, uint16_t), 0x10016de7)
#define SIM_CALL_10016632 SIM_FN(void(__cdecl *)(int), 0x10016632)

/*
 * The three functions below do the same with different numbers: when value < 8 (signed), Sim_10015c61(value) is not -1
 * and the record it names has a dword +0x28 that points to a block whose dword +0x4f0 is 2, then
 * dword [0x1012ff58 + 4 * value] = N and 0x10016117(value, M) runs. The second argument is not used.
 */
/* N = 2, M = 0xf. */
void __cdecl Sim_100104fd(int value, int unused);
/* N = 3, M = 5. */
void __cdecl Sim_10010632(int value, int unused);
/* N = 6, M = 0xb. */
void __cdecl Sim_10010767(int value, int unused);
/*
 * mode 0 picks 1 when only slot 1 is usable (Sim_1000fe60), 2 when only slot 2 is, and returns when neither is;
 * mode 1 and 2 return when their slot is not usable. Otherwise [0x1015d100] = mode and 0x100200d1(mode, 0) runs.
 */
void __cdecl Sim_10010951(int mode);
/*
 * Runs Sim_100152fc(obj[8], value, 2); when that is 0 the result is 0, else when Sim_10052aaf(obj[8]) equals [0x1017b8c8]
 * 0x10020256(4, obj[0x10]) runs. Returns the word `value` (or 0) in the low half of eax; the top half is what the
 * last call left in eax (0 when Sim_100152fc returned 0), as in the original.
 */
int __cdecl Sim_1001393e(void *obj, uint16_t value, uint16_t unused);
/* Clears dword (obj[0x54])[8] and obj[0xd4]; when Sim_10079feb(obj[0]) is non-zero: Sim_10017aa9(obj) if the word obj[0x1a6] is not 0, else 0x10017b3e(obj, 4000.0f). The second argument is not used. */
void __cdecl Sim_10013999(void *obj, int16_t unused);
/* Sim_100409be(kind, words) when [0x1015ec58] is set; then, for words[0] (signed word) not 0: 0x10016de7(kind, words[1]) when it is 7; for NULL or 0: 0x10016632(kind). */
void __cdecl Sim_10016d83(int kind, const uint16_t *words);

#endif
