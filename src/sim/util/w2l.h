/*
 * Wave 2l additions to util: a second consumer of the ring of 128 table values
 * used by Sim_1009c929 (see util.h). Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UTIL_W2L_H
#define HG_SIM_UTIL_W2L_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Like Sim_1009c929: the table value at 0x101cffc0 + 4 * [0x1017cd40], modulo m (signed), and the index in
 * dword 0x1017cd40 advances to (index + 1) % 0x7f. The index is a separate one from Sim_1009c929's.
 */
int __cdecl Sim_1009c9a5(int m);

#endif
