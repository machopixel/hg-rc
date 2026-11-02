#include "w10.h"
#include "w2h.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../libm/hg_math.h"

#define GF(addr) (*SIM_PTR(float, addr))

typedef double(__cdecl *W10aFabsFn)(double);

/* The field indexes the original interpolates, in its order. */
static const int kW10aField[7] = {0, 1, 8, 9, 0xc, 0xd, 0xe};

// FUNCTION: HGSIM 0x100a6ee4
uint32_t __cdecl Sim_100a6ee4(const float *a, const float *b, float z)
{
    float v[7];
    const float *lo, *hi;
    double d53;
    float d, t;
    uint32_t out;
    int i;

    for (i = 0; i < 7; i++)
        v[i] = a[kW10aField[i]];
    /* fcomp: unordered goes the "b not above a" way */
    if (b[2] > a[2]) {
        lo = a;
        hi = b;
    } else {
        lo = b;
        hi = a;
    }
    /* x87 at 53-bit precision: the difference is tested as the 53-bit value, used as the stored float */
    d53 = (double)hi[2] - (double)lo[2];
    d = (float)d53;
    if (hg_fabs(d53) >= (double)GF(0x1012ba74)) {     /* NaN keeps a's values */
        t = (float)((double)z - (double)lo[2]);
        for (i = 0; i < 7; i++) {
            int k = kW10aField[i];

            v[i] = (float)(((double)hi[k] - (double)lo[k]) * (double)t / (double)d + (double)lo[k]);
        }
    }
    out = Sim_100a88ff();
    *(float *)BYTES(out) = v[0];
    *(float *)(BYTES(out) + 4) = v[1];
    U32(out, 8) = G32(0x10185c50);
    *(float *)(BYTES(out) + 0x20) = v[2];
    *(float *)(BYTES(out) + 0x24) = v[3];
    *(float *)(BYTES(out) + 0x30) = v[4];
    *(float *)(BYTES(out) + 0x34) = v[5];
    *(float *)(BYTES(out) + 0x38) = v[6];
    return out;
}
