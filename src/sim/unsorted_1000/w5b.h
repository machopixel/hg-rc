/*
 * Round 5, letter b: small functions in 0x10008123..0x1000c7b4 that call helpers which are not converted
 * yet (called through their address with SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1000_W5B_H
#define HG_SIM_UNSORTED_1000_W5B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* The C library sprintf of the DLL (0x100fc780): writes the formatted text and a terminating zero. */
#define SIM_SPRINTF SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* Not converted. The second argument is a float passed as its 32 bits. */
#define SIM_CALL_1000b070 SIM_FN(int(__cdecl *)(void *, uint32_t), 0x1000b070)
#define SIM_CALL_1000b587 SIM_FN(int(__cdecl *)(void *, uint32_t), 0x1000b587)

/* Prints a 50-character line (two sprintf of a 50.50 format over a blank) at row 1 and again at row 2 of the text screen when [0x10a3ade8] is set; then [0x1015d048] = 0. */
void __cdecl Sim_10008123(void);
/* Like Sim_10008123 with a 12.12 format, one line at column 0x40, row 0, attribute 0x78; then [0x1015d050] = 0. */
void __cdecl Sim_100082be(void);
/* Runs 0x1000b070(obj, arg) when the dword obj[0x1f4] is not 0, else 0x1000b587(obj, arg) (arg is a float, passed on as bits). When that result is not 0 and Sim_1000afe7(word obj[0x1aa]) is true the word [0x101654f2] is incremented. Returns the result. */
int __cdecl Sim_1000ae00(void *obj, uint32_t arg);
/* 1 when value is 0, 1, 9 or 12 (unsigned compare), else 0. */
int __cdecl Sim_1000c7b4(uint32_t value);

#endif
