/*
 * Round 29: the kind-0 shot update (DECISIONS.md, "Round 29").
 */
#ifndef HG_SIM_UNSORTED_1002_W29_H
#define HG_SIM_UNSORTED_1002_W29_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * One frame of the shot idx of kind 0 (record 0x1096fc80 + idx * 0x156; every projectile weapon but the thrown
 * grenades, which are kind 3, Sim_100225c7): the path from the last position (+0xc..) to the node's position
 * (Sim_100a9cfb) as a segment (Sim_1009cab0, Sim_1009cc82); the hit (forced by flag 0x8000 of +0x48 for a unit /
 * building target, else Sim_1002dda9). A unit hit (flag 0x100) is scored, damaged (Sim_1006c161) and told
 * (Sim_10015afd); a building gets Sim_10025aee; other hits mark +8. With no hit, a position below height 0 and the
 * ground (Sim_1002d961) ends the shot at the point where its path meets the ground (Sim_1009cf67). A hit or that
 * ends it through Sim_10024548, as does running out of time (+0x34). Otherwise: rockets / missiles (types 0x3e /
 * 0x3f) look for type-5 shots within 500, the shot moves by velocity +0x18.. and acceleration +0x24.. (unless
 * +0x152), steers (Sim_100241ed), moves its node, marks the camera target and draws its trail by type.
 * Deliberate fix (round 29): the ground impact with no hit uses the shot's own path; the original cut a segment it
 * never built.
 */
void __cdecl Sim_100232d7(int idx);

#endif
