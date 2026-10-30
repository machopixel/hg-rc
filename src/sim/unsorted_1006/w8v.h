/*
 * Round 8, letter v: functions at 0x10060b76..0x10061bc5: quaternion <-> rotation matrix conversions, a view
 * render pass with a temporary camera, and a resource reload. Helpers that are not converted yet (or that
 * another round-8 helper converts at the same time, or that hooks_view.c patches) are called through their
 * original address with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1006_W8V_H
#define HG_SIM_UNSORTED_1006_W8V_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* C library _assert(expression, file, line), fabs(x) and sqrt(x) (results in st(0)). */
#define W8V_ASSERT SIM_FN(void(__cdecl *)(const char *, const char *, int), 0x100ff860)
#define W8V_FABS SIM_FN(double(__cdecl *)(double), 0x100fcb70)
#define W8V_SQRT SIM_FN(double(__cdecl *)(double), 0x100fca04)
/* Patched by hooks_view.c: steps of the camera (camera). */
#define W8V_CALL_100d2306 SIM_FN(void(__cdecl *)(void *), 0x100d2306)
#define W8V_CALL_100d1f90 SIM_FN(void(__cdecl *)(void *), 0x100d1f90)
#define W8V_CALL_10087d83 SIM_FN(void(__cdecl *)(void *), 0x10087d83)
/* Not converted: (camera). */
#define W8V_CALL_100a0def SIM_FN(void(__cdecl *)(void *), 0x100a0def)
/* Not converted: (the fourth argument of Sim_100619a0). */
#define W8V_CALL_100a512a SIM_FN(void(__cdecl *)(uint32_t), 0x100a512a)
/* Not converted: (an entry of the list at 0x1036f2a0). */
#define W8V_CALL_100a4f03 SIM_FN(void(__cdecl *)(uint32_t), 0x100a4f03)
/* Not converted: (the fifth argument of Sim_100619a0, 0, 0, 0). */
#define W8V_CALL_10062b7c SIM_FN(void(__cdecl *)(uint32_t, int, int, int), 0x10062b7c)
/* Not converted: finds a resource (name, extension, out size or NULL); upper-cases both texts in place. */
#define W8V_CALL6_100ba7d5 SIM_FN(void *(__cdecl *)(char *, char *, uint32_t *), 0x100ba7d5)
/* Not converted: (slot). */
#define W8V_CALL_100d3427 SIM_FN(uint32_t(__cdecl *)(int), 0x100d3427)

/* w8v_quat.c */
void __cdecl Quat_ToMatrix(const float *q, float *m);
void __cdecl Matrix_ToQuat(float *m, float *q);

/* w8v_misc.c */
void __cdecl Sim_100619a0(int preset, uint32_t value, uint8_t *view, uint32_t target, uint32_t extra);
void __cdecl Sim_10061bc5(int second);

#endif
