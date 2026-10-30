/*
 * Round 5, letter b: a function around 0x1009ff33 that calls a C library routine of the DLL (fread 0x100fe830)
 * through its address with SIM_FN.
 */
#ifndef HG_SIM_UNSORTED_1009_W5B_H
#define HG_SIM_UNSORTED_1009_W5B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: fread(buffer, size, count, file). */
#define SIM_FREAD SIM_FN(uint32_t(__cdecl *)(void *, uint32_t, uint32_t, void *), 0x100fe830)

/*
 * For length = 0, 1, ... while length < size - 1 (unsigned): got = fread(buf + length, 1, 1, file), stopping when got is 0 or
 * the character read is '\n'. Unless length is 0 and the last fread returned 0, length is then incremented and
 * buf[length] = 0 (so after an fread that returned 0 the zero follows one unread byte). Returns length. The original's
 * size is only ever 0x200; for 1 it reads an uninitialised local, which this version treats as 0.
 */
uint32_t __cdecl Sim_1009ff33(void *file, char *buf, uint32_t size);

#endif
