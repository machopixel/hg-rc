/*
 * Round 3 (letter o) functions around 0x10090e23..0x1009cd36: the sweep over
 * the 0x57e-byte records at 0x10217c20 and the cached length / unit
 * direction of a segment.
 *
 * Segment (the structure the three Sim_1009cXXX functions work on):
 *   +0x00 start (3 floats)  +0x0c end (3 floats)  +0x18 end - start (3 floats)
 *   +0x24 (end - start) / length (3 floats)  +0x30 length  +0x34 dword: 1 once
 *   the length and the unit direction are valid (0 means not yet computed).
 */
#ifndef HG_SIM_UNSORTED_1009_W3O_H
#define HG_SIM_UNSORTED_1009_W3O_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Calls the function stored at +0x556 of every record (argument: the record)
 * whose index is in the list at 0x1017cc28 (nodes: next at +4, index at +8);
 * when it returns 0, the record is released with Sim_10090513(index) and
 * [0x1017cc24] drops by one. Does nothing without the list, without
 * [0x1015dc80] or while [0x1017b92c] is set. Copies the indices to the array at
 * 0x10215be0 (count in [0x10217c00]) first.
 */
void __cdecl Sim_10090e23(void);

/* If [0x1017cc24] is non-zero, index < [0x1017cc20] and bit 0 of the word at record+0x10 is set: calls the function stored at record+0x55a with the record. */
void __cdecl Sim_10090f17(unsigned index);

/* The segment's length (computed first when +0x34 is 0), returned in st(0) like the original. */
float __cdecl Sim_1009cbde(void *seg);

/* Computes length, unit direction and the valid flag when the flag is 0 and the length is greater than the float at 0x1012ba04. */
void __cdecl Sim_1009cbfe(void *seg);

/* Sets the end point to start + t * unit direction, recomputes the delta, stores t as the length. Does nothing if the segment has no length. */
void __cdecl Sim_1009cd36(void *seg, float t);

#endif
