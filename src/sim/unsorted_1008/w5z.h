/*
 * Round 5 (letter z): the function at 0x1008663f, which calls Windows.
 * Calls of functions that are not converted yet go through their original
 * address (SIM_FN); the others call the converted C functions by name.
 */
#ifndef HG_SIM_UNSORTED_1008_W5Z_H
#define HG_SIM_UNSORTED_1008_W5Z_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * When the original 0x1010026a() answers non-zero and dword 0x1015dca0 is not 0: SendMessageA(dword 0x1017b8f0, 0x41e, 0, 0)
 * and MessageBoxA(0, text at 0x1014b658, caption at 0x1014b64c, 0x10). Has no stack frame of its own; leaves eax as the last call does.
 */
void __cdecl Sim_1008663f(void);

#endif
