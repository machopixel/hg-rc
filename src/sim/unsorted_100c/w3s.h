/*
 * Wave 3S, original 0x100c3f0c..0x100c4323: comparisons of distances to the entries of the table at
 * 0x109e4cc4 (see unsorted_1003/w3s.h). Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_100C_W3S_H
#define HG_SIM_UNSORTED_100C_W3S_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * -1 when a, b or entry is NULL. Otherwise, with pa and pb the float triples at a+0x58 and b+0x58, 1 when
 * the float-rounded squared distance of the entry to pa is below both the squared distance between pa and
 * pb and that of the entry to pb, and Sim_1003366e(entry, pa, pb) is 1 (a NaN counts as below); else 0.
 */
int __cdecl Sim_100c3f0c(const void *a, const void *b, const void *entry);
/*
 * a (entry number) when it is not -1 and the entry a is nearer (float-rounded Entry_DistSq, a NaN counts as
 * nearer) to the float triple at obj+0x58 than entry b; otherwise b. unused is ignored.
 */
int __cdecl Sim_100c4265(int a, int b, int unused, const void *obj);
/* Like Sim_100c4265, but a is returned only when its entry is strictly farther (a NaN gives b). */
int __cdecl Sim_100c4323(int a, int b, int unused, const void *obj);

#endif
