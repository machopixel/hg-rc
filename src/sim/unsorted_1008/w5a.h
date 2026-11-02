/*
 * Round 5, letter a: functions of 0x1008349a..0x1008fce3 that call helpers which are not converted yet.
 * Those helpers are called through their address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1008_W5A_H
#define HG_SIM_UNSORTED_1008_W5A_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: a debug print (format, four doubles). */
#define W5A_CALL_100348d8 SIM_FN(int(__cdecl *)(const char *, double, double, double, double), 0x100348d8)
/* Sine and arc tangent from OpenLibm (hg_sin, hg_atan), in place of the C library's 0x100ff644 / 0x10100044. */
#define W5A_SIN hg_sin
#define W5A_ATAN hg_atan
/* Not converted: stores the 16-bit value at index (the word table at 0x10156dcc). */
#define W5A_CALL_100f01cc SIM_FN(void(__cdecl *)(int, uint32_t), 0x100f01cc)
/* Not converted: the C library's fprintf-like print (destination, text). */
#define W5A_CALL_100fd7e0 SIM_FN(int(__cdecl *)(uint32_t, const char *), 0x100fd7e0)
/* Not converted: one dword. */
#define W5A_CALL_100a0def SIM_FN(int(__cdecl *)(uint32_t), 0x100a0def)
#define W5A_CALL_100a4f03 SIM_FN(int(__cdecl *)(uint32_t), 0x100a4f03)

/*
 * Prints the 3 x 4 floats of m as three rows with the format "    %9.2f %9.2f %9.2f %9.2f\n": row i is
 * m[i], m[i + 3], m[i + 6], m[i + 9] (0x100348d8 does the printing).
 */
void __cdecl Sim_1008349a(const float *m);
/*
 * Fills the float tables of 0x201 entries at 0x103719a0 (entry i = sin(i * 0.003067961572265625), the last one
 * 1.0) and at 0x103721c0 (entry i = atan(i / 512.0) * 57.29577957855229, the last one 45.0). Returns 1.
 */
int __cdecl Trig_InitTables(void);
/*
 * For the 256 entries of three bytes (r, g, b) at rgb: 0x100f01cc(i, ((r >> 1) << 11) | (g << 5) | (b >> 1)).
 * Returns 0.
 */
int __cdecl Sim_1008799c(const uint8_t *rgb);
/*
 * Prints the lines "Collidable Parents\n" and "Collidable ALL\n" with 0x100fd7e0(out, line) and walks the
 * lists at [0x1017b9a0] (next pointer at +0x18) and [0x1017b998] (next pointer at +0x10), counting their nodes;
 * returns the number of nodes of the second list (the first count is not used).
 */
int __cdecl Sim_10088c1c(uint32_t out);
/*
 * When bit 0 of flags is set: 0x100a0def([0x1014cae0]). Then Sim_10087c86() and 0x100a4f03(p) for each
 * non-zero pointer p of the table at 0x1036f2a0 (up to the first 0), then Sim_100071f6().
 */
void __cdecl Sim_1008fce3(uint32_t flags);

#endif
