/*
 * Round 9, letter b, unsorted_1005: a unit's weapon-state reset at respawn and the joystick profile name.
 */
#ifndef HG_SIM_UNSORTED_1005_W9B_H
#define HG_SIM_UNSORTED_1005_W9B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Clears the dwords +0x8c, +0x90, +0x94 of the unit entry, sets bit 0x10 of +0x88, puts -1 into
 * 0x109b8b60[entry+4] and ors Sim_1004ee50(entry) into +0x8c. Returns the entry (what the original leaves in eax).
 */
uint8_t *__cdecl Sim_100500ab(uint8_t *entry);
/*
 * Writes the profile name of a joystick into dev+0x4c from the device name at dev+0xc and the capability
 * record caps (word +0x60 flag bits, dword +0x3c): "tracker", "sidewind", "fltstick", "tmaster" or "joystick".
 * Returns dev+0x4c (the Str_Copy result the original leaves in eax).
 */
char *__cdecl Joy_PickProfileName(uint8_t *dev, const uint8_t *caps);

#endif
