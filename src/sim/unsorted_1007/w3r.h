/*
 * Round 3 (letter r), original 0x10077ecd and 0x10077fff: both call the method at +0x11c of
 * the same seven objects (pointers at 0x109e88e8, 0x109e890c, 0x109e8928, 0x109e8938,
 * 0x109e894c, 0x109e8950, 0x109e8954) with (object, value), through the pointer stored in the
 * object, then set three shared values and call Sim_1009b30f.
 */
#ifndef HG_SIM_UNSORTED_1007_W3R_H
#define HG_SIM_UNSORTED_1007_W3R_H

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * When [0x10179d20] is set: gives the objects the saved values at 0x1096fc40..0x1096fc58, copies
 * 0x1096fbec, 0x1096fc1c and 0x1096fc20 to 0x101355b8, 0x10144fac and 0x1015d1cc and calls
 * Sim_1009b30f(0x1096fc20). Returns [0x10179d20].
 */
int __cdecl Sim_10077ecd(void);
/* Gives the objects the values 9, 10, 8, 1, 0, 12, 13, sets 0x101355b8, 0x10144fac and 0x1015d1cc to 1 and calls Sim_1009b30f(1). */
void __cdecl Sim_10077fff(void);

#endif
