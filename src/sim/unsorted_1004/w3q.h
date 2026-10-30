/*
 * Round 3 (letter q) functions of HGSIM.DLL at 0x1004c5c1 and 0x1004e6a2.
 * Names carry the original address because the code alone does not say what
 * the fields mean. Floating point is done as the original's x87 did it
 * (see util/w3q.h).
 */
#ifndef HG_SIM_UNSORTED_1004_W3Q_H
#define HG_SIM_UNSORTED_1004_W3Q_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Let n_k = Sim_1004c565 / c573 / c584 / c595 / c5a6 (word) for k = 0..4 and R(i) the 0x98-byte
 * records at 0x10124eb8 (int +0, float +4). sum = dword at 0x101116b2 + 0xaa * a, plus the int
 * of R(n_k) for each k; the result is Render_Ftol of (float)sum multiplied, one float rounding
 * after each step, by the float of R(n_0) .. R(n_4).
 */
int __cdecl Sim_1004c5c1(int a, unsigned word);
/*
 * Callback-driven walker over rows and columns: while [0x1015f224] (row) is below [0x1015f330] it calls
 * the function at [0x1015f334] with ([0x1015f338] + [0x1015f228], [0x1015f220] + row)
 * (the sums are its two arguments, in that order), then advances the column, and at [0x1015f33c] columns
 * wraps it to 0 and advances the row. Returns the callback's result, or -1 when the rows are done.
 */
int __cdecl Sim_1004e6a2(void);

#endif
