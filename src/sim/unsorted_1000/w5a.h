/*
 * Round 5, letter a: functions of 0x10007c45..0x1000ed77 that call helpers which are not converted yet.
 * Those helpers are called through their address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1000_W5A_H
#define HG_SIM_UNSORTED_1000_W5A_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: the C library's sprintf (destination, format, ...). */
#define W5A_SPRINTF SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* Not converted: twelve dwords (the third is the name index from Sim_1000df9c). */
#define W5A_CALL_1000e737 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x1000e737)
/* Not converted: a lookup in the sine table at 0x103719a0 (float in, result in st(0)). */
#define W5A_SINTAB SIM_FN(double(__cdecl *)(float), 0x10084766)

/*
 * When the dword [0x10a3ade8] is not zero: writes 14 blanks at column 0x2f of row 0 of the text screen
 * (attribute 0x78). Then clears the dword [0x1015d028].
 */
void __cdecl Sim_10007c45(void);
/*
 * Nothing for a null or empty name. Otherwise Sim_1000df9c(name), then
 * 0x1000e737(0, 0, index, 0, a, [0x101321b4], b, table[which], -1, c, 0, 0) with the dword table at
 * 0x1012df30; returns what that returns.
 */
int __cdecl Sim_1000ebfb(const char *name, uint32_t a, uint32_t b, int which, uint32_t c);
/*
 * trunc(sin-table lookup(float at [[0x1014cae0] + 0xc] - v) * the double at 0x101110d8), limited to
 * -7500..7500.
 */
int __cdecl Sim_1000ed77(float v);

#endif
