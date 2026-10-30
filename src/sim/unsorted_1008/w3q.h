/*
 * Round 3 (letter q) functions of HGSIM.DLL at 0x1008e489.
 * Names carry the original address because the code alone does not say what
 * the fields mean. Floating point is done as the original's x87 did it
 * (see util/w3q.h).
 */
#ifndef HG_SIM_UNSORTED_1008_W3Q_H
#define HG_SIM_UNSORTED_1008_W3Q_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * size = two ints (w, h); scale = float. With ratio = h / w (0 when w is 0), `mode` picks how the
 * pair (out[0], out[1]) is derived: modes 0..3 compare scale with ratio (modes 1 and 2 with -ratio)
 * and give one coordinate as 0 or 2w / 2h and the other as w or h plus or minus
 * Render_Ftol(h / scale) or Render_Ftol(w * scale); 5 = (w, 2h), 7 = (w, 0); any other mode
 * (4, 6, above 7) = (0, 0). Returns out.
 */
int *__cdecl Sim_1008e489(const int *size, unsigned mode, float scale, int *out);

#endif
