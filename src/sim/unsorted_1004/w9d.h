/*
 * Round 9, letter d: the file-name builder of HGSIM.DLL at 0x10047569.
 */
#ifndef HG_SIM_UNSORTED_1004_W9D_H
#define HG_SIM_UNSORTED_1004_W9D_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9d_path.c ---- */
/*
 * Clears the 0x50-byte text at 0x1015ef50, then puts `name` there: through sprintf with the format at 0x1013b2f0
 * and the text at 0x1015f050 first when that text is not empty, else as a plain copy. Returns 0x1015ef50.
 */
char *__cdecl Sim_10047569(const char *name);

#endif
