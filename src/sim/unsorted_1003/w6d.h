/*
 * Round 6, letter d: functions in 0x10031ed3..0x10034dc2 that call helpers which are not converted yet (called
 * through their original address with SIM_FN, prototypes below) besides converted ones.
 */
#ifndef HG_SIM_UNSORTED_1003_W6D_H
#define HG_SIM_UNSORTED_1003_W6D_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* Not converted: (dword, dword, segment of 0x38 bytes it may change); non-zero for a hit. */
#define W6D_CALL_10031ce0 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, void *), 0x10031ce0)
/* Not converted: stores a code address and the code segment through its two pointers, returns a byte count. */
#define W6D_CALL_100f9eb7 SIM_FN(uint32_t(__cdecl *)(uint32_t *, uint32_t *), 0x100f9eb7)
/* Patched by other hook files: (message number, 0) and (0x400, 0xbb8). */
#define W6D_CALL_100bc280_2 SIM_FN(void(__cdecl *)(uint32_t, uint32_t), 0x100bc280)
#define W6D_CALL_100a8816 SIM_FN(void(__cdecl *)(uint32_t, uint32_t), 0x100a8816)

/* Windows imports used here (KERNEL32). */
typedef uint32_t(__stdcall *W6dGetVersionFn)(void);
typedef int(__stdcall *W6dVirtualProtectFn)(void *address, uint32_t size, uint32_t protect, uint32_t *old_protect);
#define W6D_IAT_GETVERSION 0x10ac55d4u
#define W6D_IAT_VIRTUALPROTECT 0x10ac55d0u

/*
 * Tries the four dwords obj[0x1c..0x28] with 0x10031ce0(dword, b, copy of seg): of the hits the one whose length
 * (Sim_1009cbde) is smallest is copied back to seg (0x38 bytes) and 1 is returned; 0 without a hit shorter than
 * 3.3999999521e38 (0x101112ec).
 */
int __cdecl Sim_10031ed3(void *obj, uint32_t b, void *seg);
/* Clears the characters of row 24 of the text screen at 0xb0000 (bytes 0xb0f00..0xb0f9f), keeping the attributes. */
void __cdecl Sim_100345fc(void);
/*
 * Makes the code block 0x100f9eb7 reports writable (VirtualProtect with PAGE_READWRITE when GetVersion has the top
 * bit set, else PAGE_EXECUTE_READWRITE; error 0x4d through 0x100bc280 when that fails), calls 0x100a8816(0x400,
 * 0xbb8), sets [0x1017d140] = 0xb54, Sim_100883a2(), stores five function addresses at 0x1014cb3c, [0x10138c84] =
 * 0xff, clears [0x1014cb18] when [0x1014cb04] or [0x1014cb08] is set and sets bit 8 of [0x1014caf8].
 */
void __cdecl Sim_10034cca(void);
/*
 * For every cell listed by Sim_10087c86 (when [0x1017c8cc] is set): walks its objects (+8 list): kind 0x90 (word
 * +2 & 0xf0) goes through Sim_100aa4a2 of its tree (+0x28, kept in [0x1015e544]) and the walk restarts at the cell's
 * first object; kind 0x70 gets Sim_1008a40a and Sim_1008a37f; any other with +0x34 == 4 gets Sim_1008a37f.
 */
void __cdecl Sim_10034dc2(void);

#endif
