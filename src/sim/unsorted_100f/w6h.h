/*
 * Round 6, letter h: C runtime code of HGSIM.DLL at 0x100fc640..0x100fd9a0 (the Visual C++ library the DLL
 * carries). Helpers that are not converted yet are called through their original address (SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_100F_W6H_H
#define HG_SIM_UNSORTED_100F_W6H_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: (string, end pointer, base, flags); 0x100fd290 passes flags 0, 0x100fd540 flags 1. */
#define SIM_CALL_100fd2b0 SIM_FN(int32_t(__cdecl *)(const char *, char **, int, int), 0x100fd2b0)
/* Not converted: (name, mode, 0x40); returns a pointer. */
#define SIM_CALL_100fd820 SIM_FN(void *(__cdecl *)(const char *, const char *, int), 0x100fd820)
/* Not converted: (stream structure, format, pointer to the variable arguments); the scanning engine. */
#define SIM_CALL_10103ba0 SIM_FN(int(__cdecl *)(void *, const char *, void *), 0x10103ba0)
/* Not converted: (info type, source bytes, count, out words, code page, locale); 0 when it fails. */
#define SIM_CALL_10104fd0 SIM_FN(int(__cdecl *)(uint32_t, const char *, int, uint16_t *, uint32_t, uint32_t), 0x10104fd0)

/* Stores six code addresses of the DLL (0x10100f30 twice, 0x10100af0, 0x10100b80, 0x10100a90, 0x10100b60) in the dwords 0x10159700..0x10159714. */
void __cdecl Sim_100fc640(void);
/* strstr: the first place where `needle` occurs in `hay` (hay itself for an empty needle), or null. */
char *__cdecl Crt_StrStr(const char *hay, const char *needle);
/*
 * The character class word of c (table pointer at 0x10159450, indexed from -1 to 255) masked with `mask`.
 * Other values of c go to 0x10104fd0 with the one or two bytes of c (two when the high byte of the class word of
 * the byte (c >> 8) has bit 0x80); 0 when that fails.
 */
int __cdecl Crt_IsCType(int c, int mask);
/*
 * atol: skips bytes of class 8 (space), takes one '-' or '+', then adds decimal digits (class 4) with wrapping
 * 32-bit arithmetic. When the dword 0x1015965c is above 1 the classes come from Crt_IsCType.
 */
int32_t __cdecl Crt_AToL(const char *s);
/* atoi: Crt_AToL. */
int32_t __cdecl Crt_AToI(const char *s);
/* 0x100fd2b0(s, end, base, 0). */
int32_t __cdecl Sim_100fd290(const char *s, char **end, int base);
/* 0x100fd2b0(s, end, base, 1). */
int32_t __cdecl Sim_100fd540(const char *s, char **end, int base);
/* 0x100fd820(name, mode, 0x40). */
void *__cdecl Sim_100fd850(const char *name, const char *mode);
/*
 * Scans the string s with 0x10103ba0 through a stream structure on the stack: buffer and position s, count
 * strlen(s), flags 0x49; the variable arguments are handed over as a pointer. Returns what 0x10103ba0 returns.
 */
int __cdecl Sim_100fd710(const char *s, const char *format, ...);

#endif
