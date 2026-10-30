/*
 * Helpers for the floating-point functions of round 3 (letter q). The original
 * did this work on the x87 stack with a 53-bit precision control word (the
 * Windows default): every operation is rounded to double and only a store to
 * a dword rounds to float. The converted code therefore writes every
 * arithmetic step with explicit doubles.
 */
#ifndef HG_SIM_UTIL_W3Q_H
#define HG_SIM_UTIL_W3Q_H

#include "access.h"

/* Float at a byte offset (the original reads and writes these as plain dwords). */
typedef float HG_MAY_ALIAS f32a;
#define F32A(p, off) (*(f32a *)(BYTES(p) + (off)))
#define GF32(addr) (*SIM_PTR(f32a, addr))

/*
 * The three outcomes of the original's FCOMP + FNSTSW + TEST AH, mask, in terms of
 * C comparisons. An unordered pair (NaN) sets all of C0, C2 and C3, so it counts
 * as "below" and as "equal", but never as "above".
 */
#define X87_BELOW(a, b) (!((a) >= (b)))                 /* test ah,1 non-zero  (a < b or unordered) */
#define X87_ABOVE(a, b) ((a) > (b))                     /* test ah,0x41 zero   (a > b) */
#define X87_NOT_ABOVE(a, b) (!((a) > (b)))              /* test ah,0x41 non-zero (a <= b or unordered) */
#define X87_EQUAL(a, b) (!((a) < (b)) && !((a) > (b)))  /* test ah,0x40 non-zero (a == b or unordered) */

#endif
