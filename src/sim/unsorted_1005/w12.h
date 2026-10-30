/*
 * Round 12: game functions of HGSIM.DLL around 0x10050000..0x1005ffff that read uninitialised stack in the original;
 * converted with the start values recorded in DECISIONS.md ("Round 12: uninitialised stack").
 */
#ifndef HG_SIM_UNSORTED_1005_W12_H
#define HG_SIM_UNSORTED_1005_W12_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Input polling. Unless [0x1015ec58] is set without bit 0 of [0x1017b94c]: for each of the [0x1015fde8] axis records
 * (0x80 bytes at 0x1015fe00) reset or re-centre its control (+0x14), clear the 0x6d key flags (0x1013f164), let every
 * open device but the keyboard read (method +0x14), poll the [0x1015fdf0] key bindings (0x6e bytes at 0x101620c8,
 * Sim_1005436d) and the axis bindings (Sim_10054d98, Sim_1005436d, the step rules), and turn each axis value into its
 * float. Then the keyboard device [0x1015fdf4], when open, gives the key word (method +0x18): 0x40d toggles full
 * screen, anything else goes to the word 0x1015fd4c. Without an open keyboard the key is 0 (the original reads an
 * uninitialised local, DECISIONS.md).
 */
void __cdecl Sim_10055048(void);

#endif