/*
 * Round 5, letter c: function at 0x100c00c7 that calls helpers which are not converted yet.
 * Those helpers are called through their original address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_100C_W5C_H
#define HG_SIM_UNSORTED_100C_W5C_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: takes a double on the stack, returns its absolute value in st(0). */
#define SIM_FABS_C hg_fabs
/* Not converted: the C library sprintf (destination, format, ...). */
#define SIM_CALL_100fc780 SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* Not converted: (dword, word, three out pointers (the dwords l4, lc, l8 used below), dword). */
#define SIM_CALL_100c0158 SIM_FN(void(__cdecl *)(uint32_t, int16_t, uint32_t *, uint32_t *, uint32_t *, uint32_t), 0x100c0158)

/* Looks up the entry for obj+4 with Sim_100ca350; if there is one (not -1) marks it in the table at 0x109a8648 (0x54-byte entries) and calls Sim_100159cf(obj, 0x100). */
void __cdecl Sim_100c00c7(void *obj, uint32_t a, int16_t b, uint32_t c);

/*
 * Floating point (x87 at the 53-bit precision the Windows loader sets: intermediates are doubles, stored results floats).
 * Returns 1 when fabs(f) > 5.0f (0x1012bb1c), and then replaces a float 0 (or NaN) at [obj+0x54]+8 with 0.001f; else 0.
 */
int __cdecl Sim_100c0379(void *obj, float f);
/* With Sim_1002dc46(obj) non-zero and the float at 0x1015dd5c at least 0.766f (0x1012bb84) returns 1, else whether the high nibble of the byte at obj+2 is 5. */
int __cdecl Sim_100c312c(const void *obj);
/*
 * Formats the frame count n as "%2.2d:%2.2d:%05.2f" in the buffer at 0x1017e5c0 (returned): n / 0x9f150, then
 * (n % 0x9f150) / 0x2a6c, then the rest divided by 181.0 (the qword at 0x1012bc60).
 */
char *__cdecl Sim_100cca20(int n);

#endif
