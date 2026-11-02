/*
 * Round 6, letter g: small functions around 0x100c0423..0x100c0fc4 on the game objects (word +0x1aa, +0x1a8,
 * the block at dword +0x54). The floats returned in st(0) by the helpers are stored as floats by the original;
 * the x87 compares run at the 53-bit precision of the Windows control word (doubles here).
 */
#ifndef HG_SIM_UNSORTED_100C_W6G_H
#define HG_SIM_UNSORTED_100C_W6G_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: the C library function 0x100fcb70 (double in, st(0) out), and 0x100c2ae5(obj). */
#define W6G_CALL_100fcb70 hg_fabs
#define W6G_CALL_100c2ae5 SIM_FN(int(__cdecl *)(void *), 0x100c2ae5)

/* Sim_1001403a(obj, word +0x1aa); f = Sim_100144e9(obj); Sim_100c0379(obj, f); returns Sim_1000ae00(obj, f). */
int __cdecl Sim_100c0423(void *obj);

/*
 * Sim_1001403a(obj, word +0x1aa); f = Sim_10015266(obj); Sim_1000ae00(obj, f); Sim_100144e9(obj) (result dropped)
 * when 0x100fcb70(f) is greater than the float 0x1012bb20. When the float +0xd4 is greater than the float 0x1012bb24
 * and f is not greater than 0x1012bb20: Sim_100c4898(obj), the dword +8 of the block +0x54 = 0 and its byte +0x26 = 1,
 * returns 0; otherwise Sim_100c48c9(obj) and returns 1.
 */
int __cdecl Sim_100c0471(void *obj);

/*
 * Sim_1001403a(obj, value); Sim_1000ae00(obj, Sim_100144e9(obj)); the dword +8 of the block +0x54 = 1/64 (0x3c800000).
 * Returns the dword +0x4f4 of the object at +0x28 (read before that store).
 */
uint32_t __cdecl Sim_100c0ac4(void *obj, int16_t value);

/*
 * Sim_1001403a(obj, value); f = Sim_100144e9(obj), or Sim_10015266(obj) when the byte +0x59 of the block +0x54 is 0
 * and 0x100c2ae5(obj) is non-zero; Sim_1000ae00(obj, f); the dword +8 of the block +0x54 = 1/64. Returns the block
 * pointer (eax of the original).
 */
uint32_t __cdecl Sim_100c0fc4(void *obj, int16_t value);

#endif
