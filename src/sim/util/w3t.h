/*
 * Float access at a byte offset (wave 3, letter t), next to the integer
 * accessors of access.h, and the bit pattern of a float.
 */
#ifndef HG_SIM_UTIL_W3T_H
#define HG_SIM_UTIL_W3T_H

#include <stdint.h>

#include "access.h"

typedef float HG_MAY_ALIAS f32a;

#define FL32(p, off) (*(f32a *)(BYTES(p) + (off)))
/* Float of the DLL's data at its preferred-base address. */
#define GFL32(addr) (*SIM_PTR(f32a, addr))

static inline float BitsToFloat(uint32_t bits)
{
    union {
        uint32_t u;
        float f;
    } v;

    v.u = bits;
    return v.f;
}

#endif
