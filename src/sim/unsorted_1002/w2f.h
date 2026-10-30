/*
 * Wave 2 (batch f) functions of HGSIM.DLL in 0x1002xxxx. Plain C, original
 * cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1002_W2F_H
#define HG_SIM_UNSORTED_1002_W2F_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Unlinks a node from the doubly linked list made of the pointers at +0xc and
 * +0x10 (each one's +0x10 / +0xc points back). When the node is the dword
 * 0x109f13f0 that dword takes the node's +0xc link; when it is 0x109f13f4 that
 * one takes the +0x10 link. Sets the word at +2 to 1 and clears both links.
 * Does nothing when the word at +2 is already 1.
 */
void __cdecl Sim_10027cce(void *node);
/* Does nothing. */
void __cdecl Sim_10027f2e(void);
/* *(p+0x30) = v. */
void __cdecl Sim_10028a07(void *p, uint32_t v);
/* Sets the words at +0xfc, +0xfe, +0x100 and +0x102 of p. */
void __cdecl Sim_10028a37(void *p, uint16_t a, uint16_t b, uint16_t c, uint16_t d);

#endif
