/*
 * Round 14: game functions of HGSIM.DLL around 0x10040000..0x1004ffff that were left original for a stack overflow,
 * settled by the shipped data (DECISIONS.md, "Round 14").
 */
#ifndef HG_SIM_UNSORTED_1004_W14_H
#define HG_SIM_UNSORTED_1004_W14_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * The sim's command dispatcher: runs the game command `cmd`. In a network game ([0x1015ec58]) without bit 0 of
 * [0x1017b94c] every command but 0x76, 0x4b and 0x4c only shows "Waiting for players...Function deactivated."
 * ([0x101c45c0]) with Sim_1007df6c. Otherwise by command: view, pane and HUD switches (Sim_10077fd8 of the view
 * objects 0x109e88e8.., the pane mode +0x30 of [0x109e88e8], flags in 0x1015fd50..0x1015fdd0), the camera modes
 * (Sim_10010951 .. Sim_10010b92, single player only), the map cycle [0x10144fa0], the lance orders 0x5d..0x63
 * (Sim_10015c61 / Sim_10015cd2 for the target group, Sim_10074c3d or the order [0x10151d50] in a network game),
 * auto-eject (0x54) and time compression (0xb0) with their messages, and the HUD sound / sensor commands. The
 * commands 0x32..0x3b set the throttle [0x1015fd40] = (cmd - 0x32) * [0x101115f0] and [0x1015fd5d] = 1. Any other
 * command goes to Sim_1008b922 (its low 16 bits) when [0x1015f1e8] is set. Each switch that the player's unit
 * (0x109e8b40[[0x1017b8c8]], word +0x1c bit 0x2000) reports gets its HUD sound with Sim_1000ebfb.
 */
void __cdecl Sim_1004954e(uint32_t cmd);

#endif
