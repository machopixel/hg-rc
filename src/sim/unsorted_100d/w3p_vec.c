#include "w3p.h"
#include "../render/w2n.h"
#include "../util/w3p.h"

static float AbsLike(float v)
{
    return (double)v > (double)GF32(0x1012bd8c) ? v : -v;
}

// FUNCTION: HGSIM 0x100db3f7
void __cdecl Sim_100db3f7(float x, float y, float z, int32_t *outX, int32_t *outY, int32_t *outZ)
{
    float a = AbsLike(x);
    float b = AbsLike(y);
    float sum = (float)((double)a + (double)b);
    float c = AbsLike(z);
    double total = (double)sum + (double)c;

    sum = (float)total;
    if (total > GF64(0x1012bd90)) {
        sum = (float)((double)sum / GF64(0x1012bd98));
        *outX = Render_Ftol((double)x / (double)sum);
        *outY = Render_Ftol((double)y / (double)sum);
        *outZ = Render_Ftol((double)z / (double)sum);
    }
}

// FUNCTION: HGSIM 0x100db5c6
void __cdecl Sim_100db5c6(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2,
                          int32_t *outX, int32_t *outY, int32_t *outZ)
{
    float ax = (float)((double)x1 - (double)x0);
    float ay = (float)((double)y1 - (double)y0);
    float az = (float)((double)z1 - (double)z0);
    float bx = (float)((double)x2 - (double)x0);
    float by = (float)((double)y2 - (double)y0);
    float bz = (float)((double)z2 - (double)y0);
    float cx = (float)((double)ay * (double)bz - (double)by * (double)az);
    float cy = (float)((double)ax * (double)bz - (double)bx * (double)az);
    float cz = (float)((double)ax * (double)by - (double)bx * (double)ay);

    Sim_100db3f7(cx, cy, cz, outX, outY, outZ);
}
