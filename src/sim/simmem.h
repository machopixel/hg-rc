/*
 * Access to the data of the original HGSIM.DLL from converted code. Addresses
 * are the ones the DLL has at its preferred base (0x10000000); the hook layer
 * sets g_simBase to where Windows really loaded it.
 */
#ifndef HG_SIM_SIMMEM_H
#define HG_SIM_SIMMEM_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
/*
 * A Windows function the original DLL imports, called through the DLL's own
 * import slot (the address of the slot at its preferred base). `type` is the
 * function pointer type, with the __stdcall convention. The test checker
 * answers these calls with stubs and compares the call log.
 */
#define SIM_API(type, slot) (*SIM_PTR(type, slot))

#endif

extern unsigned char *g_simBase;

#define SIM_PREFERRED_BASE 0x10000000u
#define SIM_PTR(type, addr) ((type *)(g_simBase + ((addr) - SIM_PREFERRED_BASE)))
/* A function of the original DLL, called with the cdecl convention. */
#define SIM_FN(type, addr) ((type)(void *)(g_simBase + ((addr) - SIM_PREFERRED_BASE)))

/*
 * A Windows function the original DLL imports, called through the DLL's own
 * import slot (the address of the slot at its preferred base). `type` is the
 * function pointer type, with the __stdcall convention. The test checker
 * answers these calls with stubs and compares the call log.
 */
#define SIM_API(type, slot) (*SIM_PTR(type, slot))

#endif
