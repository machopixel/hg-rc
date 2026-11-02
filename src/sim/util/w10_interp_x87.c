/*
 * Bit-exact reference for the interpreted functions (DECISIONS.md): the shared body w10_interp_body.h with the math
 * done the way the original's C runtime does it on the x87 (precision control 53 bits, asin / acos through fpatan
 * of sqrt((1 + x)(1 - x)), sin / cos through fsin / fcos). Proven equal to the original by the checker
 * (tools/verify/spec_w10_<address>_x87.py) and used by tools/verify/interp_measure.c to measure the platform-math
 * versions in w10_interp.c. Not hooked and not in the game build: gcc inline assembly, 32-bit x86 only.
 */
#include "w10_interp.h"
#include "../simmem.h"
#include "../crt/hg_winheap.h"
#include "access.h"
#include "util.h"
#include "w3o.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_1008/w5x.h"
#include "../unsorted_1008/w8w.h"
#include "../unsorted_100a/w3t.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100d/w2d.h"
#include "../unsorted_1004/w3s.h"
#include "../unsorted_1004/w6e.h"
#include "../unsorted_1004/w7m.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100d/fns.h"
#include "../unsorted_100f/w2k.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"

#define W10I_D(addr) (*SIM_PTR(const double, addr))

/* Each kernel is the C runtime's instruction sequence, run under its control word 0x27f (53-bit precision, round
 * to nearest) and back to the caller's afterwards. The 53-bit result converts to double without rounding. */
#define W10X_KERNEL(name, code)                                                                     \
    static double name(double x)                                                                    \
    {                                                                                               \
        unsigned short old, cw = 0x27f;                                                             \
        long double r;                                                                              \
                                                                                                    \
        __asm__ volatile("fnstcw %1\n\tfldcw %2\n\t" code "\n\tfldcw %1"                            \
                         : "=t"(r), "=m"(old)                                                       \
                         : "m"(cw), "0"((long double)x));                                           \
        return (double)r;                                                                           \
    }

/* asin: atan2(x, sqrt((1 + x)(1 - x))) */
W10X_KERNEL(W10x_Asin, "fld1\n\tfadd %%st(1), %%st\n\tfld1\n\tfsub %%st(2), %%st\n\tfmulp\n\tfsqrt\n\tfpatan")
/* acos: atan2(sqrt((1 + x)(1 - x)), x) */
W10X_KERNEL(W10x_Acos,
            "fld1\n\tfadd %%st(1), %%st\n\tfld1\n\tfsub %%st(2), %%st\n\tfmulp\n\tfsqrt\n\tfxch %%st(1)\n\tfpatan")
W10X_KERNEL(W10x_Sin, "fsin")
W10X_KERNEL(W10x_Cos, "fcos")

#define W10I_FN(name) name##_x87
#define W10I_K_ASIN(x) W10x_Asin(x)
#define W10I_K_ACOS(x) W10x_Acos(x)
#define W10I_K_SIN(x) W10x_Sin(x)
#define W10I_K_COS(x) W10x_Cos(x)

#include "../libm/hg_math.h"
#include "w10_interp_body.h"
