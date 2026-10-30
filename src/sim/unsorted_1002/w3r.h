/* Round 3 (letter r), original 0x1002986c. */
#ifndef HG_SIM_UNSORTED_1002_W3R_H
#define HG_SIM_UNSORTED_1002_W3R_H

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Resets the 0x80 entries (0x14 bytes each) of the table at 0x109f09e0: when an entry has a
 * value at +4 and a function at +0x10 the function is called with that value, then the
 * entry is overwritten with the five dwords at 0x1015dc48. Afterwards Sim_1002990f(i) is
 * called for i = 0..0x7f.
 */
void __cdecl Sim_1002986c(void);

#endif
