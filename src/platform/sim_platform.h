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

int32_t __cdecl SimPlat_GetSystemMetrics(int32_t index);
uint32_t __cdecl SimPlat_GlobalAlloc(uint32_t flags, uint32_t bytes);
uint32_t __cdecl SimPlat_GlobalSize(uint32_t mem);
uint32_t __cdecl SimPlat_GlobalFree(uint32_t mem);
void __cdecl SimPlat_GetSystemTime(void *systemTime);
void __cdecl SimPlat_GlobalMemoryStatus(void *status);
uint32_t __cdecl SimPlat_HeapCreate(uint32_t options, uint32_t initial, uint32_t maximum);
uint32_t __cdecl SimPlat_HeapAlloc(uint32_t heap, uint32_t flags, uint32_t bytes);
int32_t __cdecl SimPlat_HeapDestroy(uint32_t heap);
int32_t __cdecl SimPlat_SetEnvironmentVariable(const char *name, const char *value);
int32_t __cdecl SimPlat_SendMessage(uint32_t window, uint32_t message, uint32_t wParam, uint32_t lParam);
int32_t __cdecl SimPlat_ShowCursor(int32_t show);
uint32_t __cdecl SimPlat_GetDC(uint32_t window);
int32_t __cdecl SimPlat_ReleaseDC(uint32_t window, uint32_t dc);
int32_t __cdecl SimPlat_GetDeviceCaps(uint32_t dc, int32_t index);
int32_t __cdecl SimPlat_PeekMessage(void *msg, uint32_t window, uint32_t first, uint32_t last, uint32_t remove);
int32_t __cdecl SimPlat_MessageBox(uint32_t window, const char *text, const char *caption, uint32_t type);

#endif
