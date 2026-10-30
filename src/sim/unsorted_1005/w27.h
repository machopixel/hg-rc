/*
 * Round 27: joystick support removed (DECISIONS.md, "Round 27").
 */
#ifndef HG_SIM_UNSORTED_1005_W27_H
#define HG_SIM_UNSORTED_1005_W27_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * The joystick driver's name (table 0x101426f0 +4, Input_AddDevice): the original asks Windows for the capabilities
 * of joystick id (joyGetDevCapsA) and on success fills the device record dev ("joystick<id + 1>", axis and button
 * names +0x60.., +0x70 the state block) and returns 0.
 * Joystick support is removed: this is the original's "no joystick" path, it returns 1 and changes nothing.
 */
uint32_t __cdecl Sim_1005621b(uint32_t id, void *dev);
/*
 * The joystick driver's open (table +8): the original reads the joystick of the state block dev+0x70
 * (joyGetDevCapsA, joyGetPosEx), sets up its axis ranges and buttons and returns 0. Joystick support is removed: this is the
 * original's "no joystick" path (no state block, or Windows reports no joystick), it returns 1 and changes nothing.
 */
uint32_t __cdecl Sim_100567cb(void *dev);

#endif
