/*
 * Float access helpers for the round 3 functions of letter o.
 *
 * About the floating point in these functions: the originals run on the x87
 * unit with the Windows default control word, i.e. every intermediate is
 * rounded to a 53 bit mantissa (a double), and only a store to a float
 * variable or field rounds to 24 bits. The C versions therefore compute in
 * double and cast to float exactly where the original stores a float.
 * The FCOM + "test ah, N" idiom of the originals is rewritten as follows
 * (a and b are the compared values, a is the one that was on the stack top):
 *   test ah, 0x41 is zero      <=>  a > b
 *   test ah, 0x41 is non-zero  <=>  !(a > b)     (a <= b, or a NaN)
 *   test ah, 1 is non-zero     <=>  !(a >= b)    (a < b, or a NaN)
 *   test ah, 0x40 is non-zero  <=>  a == b or a NaN
 * A float truncated to int goes through Render_Ftol (the runtime's __ftol,
 * see render/w2n.h).
 */
#ifndef HG_SIM_UTIL_W3O_H
#define HG_SIM_UTIL_W3O_H

#include <stdint.h>

#include "access.h"
#include "../render/w2n.h"

typedef float HG_MAY_ALIAS f32o;

/* Float at a byte offset of a structure, and float at an address of the DLL's data. */
#define FL(p, off) (*(f32o *)(BYTES(p) + (off)))
#define GFL(addr) (*SIM_PTR(f32o, addr))
typedef double HG_MAY_ALIAS f64o;
#define GF64(addr) (*SIM_PTR(f64o, addr))

/* The raw bits of a float / a float from raw bits (the originals move many floats as dwords). */
static inline uint32_t Bits_OfFloat(float f)
{
    union { float f; uint32_t u; } v;
    v.f = f;
    return v.u;
}

static inline float Float_OfBits(uint32_t u)
{
    union { float f; uint32_t u; } v;
    v.u = u;
    return v.f;
}

#endif
