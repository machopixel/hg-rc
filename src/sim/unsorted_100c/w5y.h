/*
 * Round 5 (letter y) functions of HGSIM.DLL in 0x100cxxxx. Plain C, original cdecl convention.
 * They call helpers that are not converted yet through their original address (SIM_FN), so the
 * original code still runs for those. Names carry the original address because the code alone
 * does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_100C_W5Y_H
#define HG_SIM_UNSORTED_100C_W5Y_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Sim_1001403a(obj, the word at obj+0x1aa), clears the dword at obj[0x54] + 8, then passes the float
 * Sim_10015266(obj) to Sim_1000ae00(obj, x).
 */
void __cdecl Sim_100c03d8(void *obj);
/* Sim_1001403a(obj, value), then Sim_1000ae00(obj, Sim_10015266(obj)). Returns 0. */
int __cdecl Sim_100c0d9d(void *obj, short value);
/*
 * Seeks in a file (fseek through Sim_100feb20, origin 0): obj[0] is the file, the record idx (12 bytes each, from
 * obj + 4) holds the offset at +4 and a value at +8. Returns that value, or -1 for idx -1 or when the seek fails.
 */
int __cdecl Sim_100c6f39(void *obj, int idx);
/* idx = Sim_100c6eae(obj, name); returns -1 when idx is -1, else Sim_100c6f39(obj, idx). */
int __cdecl Sim_100c6f82(void *obj, const char *name);
/* Sim_100ca670(the object in pointer table slot [0x1017b8c8], arg, 2). */
void __cdecl Sim_100cbe13(uint32_t arg);
/* The same with 0x20008 as the last argument. */
void __cdecl Sim_100cbe3c(uint32_t arg);

#endif
