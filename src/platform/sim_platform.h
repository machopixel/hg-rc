/*
 * The Windows calls the converted sim start-up and frame loop (SimMain, round 21) makes, behind one thin interface so
 * that a later backend (SDL3) can replace them. The current implementation (src/sim/platform/sim_platform_iat.c)
 * makes exactly the original calls, in the original order, through HGSIM.DLL's own import slots. Handles and
 * structures are passed through as the Windows types' raw values (uint32_t / pointers).
 */
#ifndef HG_PLATFORM_SIM_PLATFORM_H
#define HG_PLATFORM_SIM_PLATFORM_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

uint32_t __cdecl SimPlat_HeapCreate(uint32_t options, uint32_t initial, uint32_t maximum);
uint32_t __cdecl SimPlat_HeapAlloc(uint32_t heap, uint32_t flags, uint32_t bytes);
int32_t __cdecl SimPlat_HeapDestroy(uint32_t heap);

#endif
