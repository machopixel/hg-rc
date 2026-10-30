#include "w8w.h"
#include "w5b.h"
#include "../util/w3o.h"
#include "../win/w4u.h"

#define SIGN_BIT 0x80000000u

// FUNCTION: HGSIM 0x10084766
float __cdecl Sim_10084766(float a)
{
    int negative = 0;
    int32_t index;
    uint32_t value;

    if (!(W8W8_FABS((double)a) >= (double)GFL(0x1012b870)))
        return GFL(0x1012b874);
    if (Bits_OfFloat(a) & SIGN_BIT) {
        a = -a;
        negative = 1;
    }
    while ((double)a > (double)GFL(0x1012b878))
        a = (float)((double)a - (double)GFL(0x1012b878));
    if ((double)a > (double)GFL(0x1012b87c)) {
        a = (float)((double)a - (double)GFL(0x1012b87c));
        negative ^= 1;
    }
    if (!((double)a > (double)GFL(0x1012b880)))
        index = Render_Ftol((double)a * (double)GFL(0x1012b884) + (double)GFL(0x1012b888));
    else
        index = Render_Ftol(((double)GFL(0x1012b87c) - (double)a) * (double)GFL(0x1012b884) +
                            (double)GFL(0x1012b888));
    value = SIM_PTR(u32a, 0x103719a0)[index];
    if (negative)
        value ^= SIGN_BIT;
    return Float_OfBits(value);
}

// FUNCTION: HGSIM 0x1008486e
float __cdecl Sim_1008486e(float x, float y)
{
    int negativeY = 0;
    int negativeX = 0;
    int32_t index;
    float result;

    if (!(W8W8_FABS((double)x) >= (double)GFL(0x1012b870))) {
        if (!(W8W8_FABS((double)y) >= (double)GFL(0x1012b870)))
            return GFL(0x1012b88c);
        if (Bits_OfFloat(y) & SIGN_BIT)
            return GFL(0x1012b87c);
        return GFL(0x1012b874);
    }
    if (!(W8W8_FABS((double)y) >= (double)GFL(0x1012b870))) {
        if (Bits_OfFloat(x) & SIGN_BIT)
            return GFL(0x1012b890);
        return GFL(0x1012b880);
    }
    if (Bits_OfFloat(x) & SIGN_BIT) {
        x = -x;
        negativeX = 1;
    }
    if (Bits_OfFloat(y) & SIGN_BIT) {
        y = -y;
        negativeY = 1;
    }
    if (!(W8W8_FABS((double)x - (double)y) >= (double)GFL(0x1012b870))) {
        result = Float_OfBits(0x42340000u);
    } else if (!((double)x >= (double)y)) {
        index = Render_Ftol((double)x / (double)y * (double)GFL(0x1012b894) + (double)GFL(0x1012b888));
        result = Float_OfBits(SIM_PTR(u32a, 0x103721c0)[index]);
    } else {
        index = Render_Ftol((double)y / (double)x * (double)GFL(0x1012b894) + (double)GFL(0x1012b888));
        result = (float)((double)GFL(0x1012b880) - (double)SIM_PTR(f32o, 0x103721c0)[index]);
    }
    if (negativeY)
        result = (float)((double)GFL(0x1012b87c) - (double)result);
    if (negativeX)
        result = -result;
    return result;
}

// FUNCTION: HGSIM 0x10084bcf
int32_t __cdecl Sim_10084bcf(float ax, float ay, float az, float bx, float by, float bz, float cx, float cy, float cz,
                             float *nx, float *ny, float *nz)
{
    float x, y, z, length;
    double p, q;

    p = ((double)by - (double)ay) * ((double)cz - (double)bz);
    q = ((double)cy - (double)by) * ((double)bz - (double)az);
    x = (float)-(p - q);
    p = ((double)bz - (double)az) * ((double)cx - (double)bx);
    q = ((double)cz - (double)bz) * ((double)bx - (double)ax);
    y = (float)-(p - q);
    p = ((double)bx - (double)ax) * ((double)cy - (double)by);
    q = ((double)cx - (double)bx) * ((double)by - (double)ay);
    z = (float)-(p - q);
    length = (float)W8W8_SQRT((double)x * (double)x + (double)y * (double)y + (double)z * (double)z);
    *nx = (float)((double)x / (double)length);
    *ny = (float)((double)y / (double)length);
    *nz = (float)((double)z / (double)length);
    return Render_Ftol((double)length);
}

// FUNCTION: HGSIM 0x100840dc
int32_t __cdecl Sim_100840dc(uint8_t *target, int32_t steps)
{
    double delta[0x100][3];
    uint8_t *current = GBYTES(0x109b8c00);
    const uint8_t *source = GBYTES(0x1015e6a0);
    int32_t left;
    int i, k;

    if (target == 0)
        return -1;
    steps >>= 1;
    for (i = 0; i < 0x100; i++) {
        current[3 * i] = source[3 * i];
        current[3 * i + 1] = source[3 * i + 1];
        current[3 * i + 2] = source[3 * i + 2];
        for (k = 0; k < 3; k++)
            delta[i][k] = (double)((int32_t)target[3 * i + k] - (int32_t)current[3 * i + k]) / (double)steps;
    }
    left = steps;
    while (left-- != 0) {
        for (i = 0; i < 0x100; i++) {
            for (k = 0; k < 3; k++)
                target[3 * i + k] = (uint8_t)(current[3 * i + k] +
                                              (Render_Ftol((double)(steps - left) * delta[i][k]) & 0xff));
        }
        Gdi_SetPalette(0, 0x100, target);
        Sim_10083c45();
    }
    G32(G32(0x1015e698)) = G32(0x1015e9a0);
    return 0;
}
