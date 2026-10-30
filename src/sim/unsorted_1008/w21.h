/*
 * Round 21: SimMain, the sim's entry point and frame loop (DECISIONS.md, "Round 21").
 */
#ifndef HG_SIM_UNSORTED_1008_W21_H
#define HG_SIM_UNSORTED_1008_W21_H

#include <stdint.h>

#ifndef _WIN32
#define __stdcall
#endif

/*
 * The exported SimMain(instance, unused, command line, network block, unused, window) of HGSIM.DLL, called by the
 * launcher: the heaps and memory set-up, the command line (mission name and switches), the display mode, about sixty
 * subsystem starts (each followed by the network poll 0x1003e291), then, inside a __try / __finally, the frame loop
 * (until [0x1017b8bc] reaches 3: the per-frame systems, the render loop through the display driver's slots, the
 * pause and mission-end handling) and the shutdown, and finally the heaps freed. Returns 0xff when [0x1015ebf8] is
 * set, else 0. The __finally shows the error box (when [0x1015dca0] is set) only when an exception unwinds through
 * SimMain.
 *
 * Every Windows call goes through src/platform/sim_platform.h. Every callee is called through its original address.
 * The function is compiled without optimisation so that each call pushes its arguments and pops them right after,
 * as the original does: the stack frames of the functions it calls then lie exactly where they did relative to
 * SimMain's stack pointer, so the leftovers that still-original code reads below it keep their layout.
 *
 * Deviation (DECISIONS.md, round 21): the display driver's start record (0x50 bytes, built only at 640 x 480 with
 * /3DFX or /D3D) gets 0 in the fields the original never sets.
 */
uint32_t __stdcall SimMain(uint32_t instance, uint32_t unused1, char *cmdline, uint8_t *net, uint32_t unused2,
                           uint32_t window);

#endif
