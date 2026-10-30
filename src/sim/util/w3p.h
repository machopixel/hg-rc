/*
 * Float access and x87-compare helpers for the round 3 functions of letter p.
 *
 * The originals compare with FCOM and then test bits of the FPU status word:
 *   test ah, 1     is true for  a < b  and for unordered (a NaN involved)
 *   test ah, 0x40  is true for  a == b and for unordered
 *   test ah, 0x41  is true for  a <= b and for unordered
 * The helpers below give exactly those three results on doubles (every float
 * converts to double without loss).
 */
#ifndef HG_SIM_UTIL_W3P_H
#define HG_SIM_UTIL_W3P_H

#include <stdint.h>
#include "access.h"

typedef float HG_MAY_ALIAS f32a;
typedef double HG_MAY_ALIAS f64a;

#define F32(p, off) (*(f32a *)(BYTES(p) + (off)))
#define GF32(addr) (*SIM_PTR(f32a, addr))
#define GF64(addr) (*SIM_PTR(f64a, addr))

static inline int X87_LessOrUnordered(double a, double b) { return !(a >= b); }
static inline int X87_EqualOrUnordered(double a, double b) { return !(a < b) && !(a > b); }
static inline int X87_LessEqualOrUnordered(double a, double b) { return !(a > b); }

#endif
