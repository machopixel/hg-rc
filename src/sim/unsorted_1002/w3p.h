/*
 * Round 3 function of HGSIM.DLL in 0x1002xxxx (letter p).
 */
#ifndef HG_SIM_UNSORTED_1002_W3P_H
#define HG_SIM_UNSORTED_1002_W3P_H

#include <stdint.h>

#include "../simmem.h"

/*
 * Does nothing unless dword [[p] + 4] equals dword 0x1017b8c8. Then for each of the 30 object pointers at
 * 0x109e88e0, if Sim_1009c929(10) is below 5 (when flag != 0) or 2 (when flag == 0), calls the function stored
 * in the object at +0x12c with (object, signed word object+6 plus 1). Sim_1009c929 is called once per object
 * whether or not the function is called.
 */
void __cdecl Sim_1002c6e2(const void *p, int flag);

#endif
