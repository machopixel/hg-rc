/*
 * Round 15: the world-file reader 0x100dcc12 (DECISIONS.md, "Round 15").
 */
#ifndef HG_SIM_UNSORTED_100D_W15_H
#define HG_SIM_UNSORTED_100D_W15_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Reads the chunks of an opened world file (BWD; `file` is the reader record of Sim_100278c0, its name at +6) one by
 * one (Sim_100279f1) and acts on each tag: world settings (PLNT, CLIM, LITE, VWST, ...), sky and ground models, the
 * objects (OBJ through Sim_100db78b, ANIM, SCRG, THNG, the HGBT / HGXP pools of shots and explosions), the gears (GPS
 * from a GDF design, GP from a placed object, their cockpit files PITF / VPTF read recursively), navigation points,
 * tasks, positions, included files (INCL) and the rest. Returns 1, or 0 when a gear failed to load (or the gear table
 * is full) or a nested file returned 0. Unknown tags are reported (0x100bc280, 8).
 *
 * Deviations (DECISIONS.md, round 15): the OBJ records built for HGBT / HGXP get the dword +0x34 (the colour, which
 * the original never writes) as 0, and in the GP case the gear record pointer starts as 0 (the original's is unset
 * until the first GP chunk that finds its object; no GP chunk ships).
 */
uint32_t __cdecl Sim_100dcc12(uint8_t *file);

#endif
