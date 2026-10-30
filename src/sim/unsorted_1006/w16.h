/*
 * Round 16: the within-range test 0x100687a3 and the unit collision and movement chain (DECISIONS.md, "Round 16").
 */
#ifndef HG_SIM_UNSORTED_1006_W16_H
#define HG_SIM_UNSORTED_1006_W16_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Whether (x, y, z) is within `radius` of the box `box` (six floats: min / max per axis): the nearest point of the box
 * (Sim_10068716) goes to *outX / *outY / *outZ, and the result is 1 when the squared distance to it is not above the
 * squared radius (or is NaN). 0 when box or an out pointer is null.
 *
 * Deviation (DECISIONS.md, round 16): the original first compares three of its own never-written locals with 0.0 and
 * returns 0 if any is 0.0 (or NaN); the leftovers it sees are the stack of the rotation helpers its callers ran just
 * before, never 0.0 in play with the original code. The C leaves that test out, so the range test always runs.
 */
int __cdecl Sim_100687a3(const float *box, float x, float y, float z, float radius, float *outX, float *outY,
                         float *outZ);

/*
 * Whether unit b's position (record +0x58 / +0x5c / +0x60) is within b's radius (+0x53c) of a's collision box (record
 * +0x284), in a's frame: the position moved by minus a's node position (Sim_100a9cfb, Sim_10068e1d) and turned by
 * minus its angles (Sim_100a9ca3, Sim_10068d76), then Sim_100687a3 with the out pointers. x / y / z are not used.
 * 0 when a or b is null.
 */
int __cdecl Sim_100689c0(const uint32_t *a, const uint32_t *b, float x, float y, float z, float *outX, float *outY,
                         float *outZ);

/*
 * Unit-against-unit collision for the unit record `self` moving to (*x, *y, *z): against every other unit of the gear
 * table (0x109e8b40, [0x1015dc64] of them; skipped: empty, own id, no record, a kind Sim_10079feb counts unless the
 * record +0x58c is 2, record flag +0x560 & 0x100, unit flag +0x1c & 0x4000) it tests the sum of the two radii (+0x53c)
 * against the distance (Sim_10084a94), or for box-collision units (+0x29c == 1) Sim_100688a3 / Sim_100689c0 /
 * Sim_100690d4. On contact *hit = the other gear, the position is pushed out of the other unit (or kept, for a box
 * unit), the push direction goes to [0x1015dd58..60] and, times half the speed difference (+0x554..), to the record
 * +0x548..+0x550. A unit in state 4 (+0x4f0) is instead handed to Sim_1006863d. The unit that is "riding" on self
 * (gear +0x84 == own id, lower index) and the plain sphere case end the search; the box cases go on. Returns 1 on
 * contact.
 */
int __cdecl Sim_10066907(uint8_t *self, float *x, float *y, float *z, uint32_t *hit);

/*
 * The move of the unit record `rec` by (dx, dy, dz) from its unit's position (+0x58..): the target goes to *x / *y /
 * *z, then it is checked against the other units (Sim_10066907: the unit's +0x84 = the gear's id), the ground objects
 * (Sim_1006754e: *hitObject) and, for a move that is not tiny, the world along the way (Sim_1002e362 on a segment
 * stretched by the radius, Sim_1009cab0 / Sim_1009cbde / Sim_1009cd36): on a hit the target is pulled back to the
 * contact and slid along it, and the record's push +0x548..+0x550 set from the speed (+0x554..). A hit on a "TL1M"
 * object by a kind 4 / 10 unit does not count. Units other than the own one move only in states 2 / 4 (+0x4f0); kinds 5
 * and 8 skip the tests unless +0x58c is 2. Returns the record's contact counter +0x4f4 (reset to 0, with *hitObject,
 * when nothing was hit; a tiny move keeps it, counting on, or returns 0).
 */
int __cdecl Sim_100663c0(uint8_t *rec, uint32_t *hitObject, uint32_t *hitGear, float dx, float dy, float dz,
                         float *x, float *y, float *z);

/*
 * The per-frame movement of the unit record `rec` (the unit at +0): the record's timers (Sim_10047bcd), the stun and
 * burn markers, the drive or air braking, the jump jets and side-steps, the speeds (+0x548..) integrated over the
 * frame ([0x1015dc80]) into a move, the collision tests (Sim_100663c0), the ground height (Sim_1002d961) and landing
 * with its fall damage (Sim_1006c161), the crash sounds and collision damage (0x1006788c / 0x100680dd), the body
 * height and angles, the crouch / prone / dodge states, the turret / head / aim nodes and the heading vectors
 * (+0xcc / +0xd0).
 *
 * Deviation (DECISIONS.md, round 16): the drive speed the side-step reads is 0 when the drive branch did not set it
 * (the original reads its unset local [ebp-0x2c] there, 0 in every call seen with the original code).
 */
void __cdecl Sim_1006e22c(uint8_t *rec);

#endif
