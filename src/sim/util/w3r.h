/*
 * Float access at a byte offset (round 3, letter r), next to util/access.h.
 * The converted floating-point code keeps the original's evaluation order:
 * the x87 code computed in double (53-bit precision, Windows default) and
 * rounded to float only when a result was stored, so the C does the same.
 */
#ifndef HG_SIM_UTIL_W3R_H
#define HG_SIM_UTIL_W3R_H

#include "access.h"

#ifdef __GNUC__
#define HG_W3R_MAY_ALIAS __attribute__((may_alias))
#else
#define HG_W3R_MAY_ALIAS
#endif

typedef float HG_W3R_MAY_ALIAS f32a;
typedef double HG_W3R_MAY_ALIAS f64a;

#define F32(p, off) (*(f32a *)(BYTES(p) + (off)))
#define GF32(addr) (*SIM_PTR(f32a, addr))
#define GF64(addr) (*SIM_PTR(f64a, addr))

#endif
