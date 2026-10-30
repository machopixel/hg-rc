/*
 * Small leaf functions of HGSIM.DLL around 0x10027acf..0x1002dc46: a code to
 * name lookup, two field setters and a table test. Names carry the original
 * address because the code alone does not say what the data means.
 */
#ifndef HG_SIM_UNSORTED_1002_W2K_H
#define HG_SIM_UNSORTED_1002_W2K_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Finds code in the 97 dwords at 0x101349e8 (the four-letter codes are ASCII
 * such as 0x445742 = "BWD") and returns the string with the same index in the
 * pointer table at 0x101348b8; the string at 0x10134fe4 ("unknown") when absent.
 */
const char *__cdecl Sim_10027acf(uint32_t code);
/* obj+8 = value */
void __cdecl Sim_10028965(void *obj, uint32_t value);
/* obj+0xf0 = value */
void __cdecl Sim_10028a15(void *obj, uint32_t value);
/* Whether the dword at 0x10138bc8 + 12 * (dword obj+0x34) is non-zero. */
int __cdecl Sim_1002dc46(const void *obj);

#endif
