/* DLL removal step 1: helpers shared by the graphics library's C files (w37_*.c). */
#ifndef HG_SIM_GFX_W37_INT_H
#define HG_SIM_GFX_W37_INT_H

#include <stdint.h>

/* `jle` after `add a, b`: the true sum is <= 0 (the 32-bit result may have wrapped). */
#define SUM_LE0(a, b) ((int64_t)(int32_t)(a) + (int64_t)(int32_t)(b) <= 0)
/* `jle` after `sub a, b`: the true difference is <= 0. */
#define SUB_LE0(a, b) ((int64_t)(int32_t)(a) - (int64_t)(int32_t)(b) <= 0)
/* 32-bit sums and differences as the original computes them (wrapping). */
#define ADD(a, b) ((int32_t)((uint32_t)(a) + (uint32_t)(b)))
#define DIFF(a, b) ((int32_t)((uint32_t)(a) - (uint32_t)(b)))

#endif
