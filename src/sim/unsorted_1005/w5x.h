/*
 * Round 5, letter x: the functions at 0x1005502a and 0x1005ae58, which call helpers that are not converted yet
 * (called through their address, SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1005_W5X_H
#define HG_SIM_UNSORTED_1005_W5X_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted. 0x1010d180 turns the lower-case letters of a string into capitals in place (one argument). */
#define SIM_CALL_1010d180 SIM_FN(void(__cdecl *)(char *), 0x1010d180)
#define SIM_CALL_1005ae79 SIM_FN(void *(__cdecl *)(void *, char *), 0x1005ae79)
#define SIM_CALL_100548aa SIM_FN(int(__cdecl *)(void), 0x100548aa)
#define SIM_CALL_10054a79 SIM_FN(int(__cdecl *)(void), 0x10054a79)

/* Runs 0x1010d180(b) and then 0x1005ae79(a, b), which looks the string b up in the binary tree a (children at +4 and +8), and returns what it finds (round 10: the result was lost before). */
void *__cdecl Sim_1005ae58(void *a, char *b);
/*
 * Runs 0x100548aa() and 0x10054a79(), then calls the function stored at +0x1c of the structure the pointer table
 * 0x1015fd20 holds at index [0x1015fdf4] (the entries are the operation tables of the input devices, see
 * 0x1013f150). The function takes no arguments; its result is dropped.
 */
void __cdecl Sim_1005502a(void);

#endif
