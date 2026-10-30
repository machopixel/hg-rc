/*
 * Round 13: game functions of HGSIM.DLL around 0x10070000..0x1007ffff whose last original callee was converted in
 * round 12.
 */
#ifndef HG_SIM_UNSORTED_1007_W13_H
#define HG_SIM_UNSORTED_1007_W13_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Draw method (+0x13c) of a view pane: nothing unless the word +4 and the mode +0x30 are set; +0xf8 = [0x109e89d8].
 * Modes other than 1, 2, 9..13 hand over to Sim_100788ba: always when the word +6 is above 2; when it is 1, while
 * the flicker flag [0x10179d24] is set (cleared again when Sim_1009c929(10) < 7), which a roll below 3 sets. Then by
 * mode: 3 rear view (camera Sim_10044992 copy, heading = the player's object +0x68 plus 180, or plus its +0x74 in
 * the replay [0x1096fbd0]; label "VP_RER" or a plain frame), 4 the replay camera (Sim_10044b2c), 5 the view down
 * ("VP_DWN", angles 90 / 0 at the player's position +0x58), each rendered with Sim_100619a0 between Sim_100aa351 /
 * Sim_100aa39f of the object's node +0x48 and the render state saved (Sim_100787f4 / Sim_10078857) and put back;
 * 6 the map (Sim_10099ef3, when byte +9 of [0x1017b8c4] is set); 7 the selected weapon (Sim_100258d7, picked with
 * Sim_1002590c, else the frame is cleared with 0x100eb070, label "VP_WPN"); 8 Sim_100c7deb; 9 Sim_100cf96c;
 * 1 and any other mode the damage display Sim_100b66a0.
 */
void __cdecl Sim_100782a9(uint8_t *pane);

#endif
