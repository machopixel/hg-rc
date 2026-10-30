/*
 * Round 23: the audio option setters Ghidra's index missed (reached only through the option tables 0x10149a10 /
 * 0x10149a64 and 0x10150584..; DECISIONS.md, "Round 23").
 */
#ifndef HG_SIM_UNSORTED_1001_W23_H
#define HG_SIM_UNSORTED_1001_W23_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Audio option `which` (0..4) set to value, kept both in the live settings and in the saved settings record
 * [0x1015d1b8]: 0 to [0x101321b4] / +4, 1 to [0x101321b8] / +8 (with bit 1 of the flags [0x101321c0]), 2 to
 * [0x101321bc] / +0xc (the music: bits 3 and 2 of the record's flags +0x10, the music started with Sim_1001f335
 * unless in a network game, or stopped with Sim_1001f399; the flags then copied to [0x101321c0]), 3 to [0x101321b0] /
 * +0, 4 bit 1 of the record's flags (copied to [0x101321c0]). Other values of which do nothing.
 */
void __cdecl Sim_1001efbe(int32_t which, uint32_t value);
/*
 * Applies the saved audio option `which` (0..4) from the settings record [0x1015d1b8] to the live settings, as
 * Sim_1001efbe; the music (2) also sets the CD volume (Sim_1009ebeb) before starting.
 */
void __cdecl Sim_1001f14f(int32_t which);

#endif
