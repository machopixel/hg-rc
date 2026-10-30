/*
 * Round 8, letter z: functions of HGSIM.DLL at 0x100e122f.
 *
 * Callees that are not converted (or patched elsewhere) are called through their original address (SIM_FN):
 *   0x100278c0 (name record, out record of 0x20 bytes) -> value
 *   0x100dcc12 (value) -> result
 *   0x100bc280 (code, ...)   the error report (patched by hooks_sim.c)
 */
#ifndef HG_SIM_UNSORTED_100E_W8Z_H
#define HG_SIM_UNSORTED_100E_W8Z_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

typedef uint32_t(__cdecl *W8zNameRecordFn)(void *record, void *out);
typedef int(__cdecl *W8zValueFn)(uint32_t value);
typedef void(__cdecl *W8zErrorFn)(int code, ...);

#define W8Z_CALL_100278c0 SIM_FN(W8zNameRecordFn, 0x100278c0)
#define W8Z_CALL_100dcc12 SIM_FN(W8zValueFn, 0x100dcc12)
#define W8Z_CALL_100bc280 SIM_FN(W8zErrorFn, 0x100bc280)

/* ---- w8z_load.c ---- */
/*
 * Resets the state (Sim_10052530, 150 dwords of -1 at 0x1017eec0, the dwords 0x1017e6a0 / 0x1017e6a4,
 * Sim_100e2756(0, 0, 0), Sim_100db77e(0)) and builds a name record from s (word: the number s starts with, or
 * -1 when it does not start with a digit; then up to 12 bytes of s, ended by a 0 at +0xe). When 0x100278c0 finds
 * it and Sim_100b35b0 agrees: 0x100dcc12 of the find, Sim_10027a8f, Sim_100db351, Sim_100b372b, and the error
 * 0x4b when Sim_100b487e says 0. Returns 1 when 0x100dcc12 gave a value that is not 0; otherwise reports
 * error 0xb with the name and returns 0.
 */
int __cdecl Sim_100e122f(const char *s);

#endif
