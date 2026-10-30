/*
 * Wave 2, letter j: leaf functions of HGSIM.DLL around 0x10010000..0x10020000.
 */
#ifndef HG_SIM_UNSORTED_1001_W2J_H
#define HG_SIM_UNSORTED_1001_W2J_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * When arg is non-zero stores 0x10000 in the dword at +2 of the 0x13f-byte
 * record number arg in the block of 0x4fc3a bytes number dword 0x1013bb40 (base
 * 0x1047383a).
 */
void __cdecl Sim_100107ce(int arg);
/*
 * Fixes up a mode: probes Sim_1000fe60(1) and Sim_1000fe60(2). A mode of 0
 * becomes 1 when only probe 1 passes, 2 when only probe 2 passes and stays 0
 * when both do; -1 when neither does. A mode of 1 (2) gives -1 when probe 1
 * (2) fails. Any other mode is returned as it is.
 */
int __cdecl Sim_100109e7(int mode);
/* Does nothing (an empty function). */
void __cdecl Sim_100125d4(void);
/*
 * Dword +4 of the 0x6c-byte group record (table 0x109b7d40) selected by the
 * dword +8 of the table-0x109e8b40 record that dword 0x1017b8c8 indexes.
 */
uint32_t __cdecl Sim_10017892(void);

#endif
