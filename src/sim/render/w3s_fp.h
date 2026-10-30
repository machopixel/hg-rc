/*
 * Helpers for the floating-point functions of wave 3S (letter s).
 *
 * The original does its arithmetic on the x87 stack with the Windows default
 * control word (53-bit mantissa), and rounds to 32 bits only where it stores a
 * float. The converted code reproduces that by doing every operation in double
 * and casting to float exactly where the original stored one: a result written
 * with `fstp dword` is (float)(expression in double); a value the original
 * leaves in st(0) is returned as double.
 */
#ifndef HG_SIM_RENDER_W3S_FP_H
#define HG_SIM_RENDER_W3S_FP_H

#include "../util/access.h"
#include "w2n.h"

typedef float HG_MAY_ALIAS f32a;
typedef double HG_MAY_ALIAS d64a;

/* A float / double at a byte offset of a structure, and in the DLL's data. */
#define F32(p, off) (*(f32a *)(BYTES(p) + (off)))
#define GF32(addr) (*SIM_PTR(f32a, addr))
#define GD64(addr) (*SIM_PTR(d64a, addr))

#endif
