/*
 * Round 12: game functions of HGSIM.DLL around 0x100e0000..0x100effff that read uninitialised stack in the original;
 * converted with the start values recorded in DECISIONS.md ("Round 12: uninitialised stack").
 */
#ifndef HG_SIM_UNSORTED_100E_W12_H
#define HG_SIM_UNSORTED_100E_W12_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * ReadWTB: one object of a WTB model at base + *offset (magic [0x1012bdc0]; else [0x1017e838] = -2, return -1).
 * Its name (+8, 16 bytes, negated bytes when the first is above 0x80) is copied and upper-cased; it sets the solid /
 * special flags. The model *out is created (Sim_10008f1c, named) or extended (Sim_10008cbc); its vertices
 * (0x2c bytes from +0x20) are added with Sim_10009136 and summed into the checksum; each polygon (0x38 or 0x3e
 * bytes) gets its material (a 3-entry cache at 0x1017e860, else Sim_100e2837 of 0x1017e740 / 0x1017e6d0, with
 * "Missing ..." lines to the log file [0x1017ed8c] when [0x1017b954] is set), its flags from the material kinds,
 * a polygon (Sim_100091f7) with its normal and corners (Sim_10009278), colours for kind 4 and the edge flag for the
 * kinds 0xb000..0xe000. *offset moves past the object; a checksum different from +4 reports error 0x58 and returns
 * -1; else Sim_100094ba(*out), [0x1017e838] = 0, return 0. A name of exactly 16 characters ends at its 16th
 * character (the original runs on into the uninitialised stack after its buffer, DECISIONS.md).
 */
int __cdecl Sim_100e2aa3(uint32_t base, uint32_t *offset, uint32_t *out);

#endif