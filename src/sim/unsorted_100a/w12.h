/*
 * Round 12: game functions of HGSIM.DLL around 0x100a0000..0x100affff that read uninitialised stack in the original;
 * converted with the start values recorded in DECISIONS.md ("Round 12: uninitialised stack").
 */
#ifndef HG_SIM_UNSORTED_100A_W12_H
#define HG_SIM_UNSORTED_100A_W12_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Sky and ground settings of the mission from skygnd.par (0x1014cb58, mode 0x1014cb54; the file is not closed, as
 * in the original). [0x1017d108] = 0; the mission key is Str_CopyUpper(mission) (32 bytes). Lines are read with
 * Sim_1009ff33 (512 bytes; none left: return), cut at '#', empty ones skipped, each parsed with sscanf(format
 * 0x1014cb64) into the locals and the globals the format names, until the first 4 characters of its mission match
 * the key (Sim_1010d240). The locals keep their values from line to line; the intensities start at 0x80, the
 * hardware ambient at 0, everything else at 0 / empty (the original reads stack leftovers there, DECISIONS.md).
 * Then the light, colour, horizon and fog globals are set as the original does, and Sim_10058549(intensity).
 */
void __cdecl Sim_100a0063(const char *mission);

#endif