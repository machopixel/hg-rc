/*
 * Round 3 (letter q) functions of HGSIM.DLL at 0x100d6e99.
 * Names carry the original address because the code alone does not say what
 * the fields mean. Floating point is done as the original's x87 did it
 * (see util/w3q.h).
 */
#ifndef HG_SIM_UNSORTED_100d_W3Q_H
#define HG_SIM_UNSORTED_100d_W3Q_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * The original leaves the result in st(0): the integer at obj+0xb4 as a double. Called from code that
 * uses the x87 stack, so a hook must return a double (declared that way: the compiler's calling
 * convention returns it in st(0) on x86).
 */
double __cdecl Sim_100d6e99(const void *obj);

#endif
