#include "w3p.h"
#include "../util/access.h"
#include "../util/w3p.h"

// FUNCTION: HGSIM 0x10058ac2
float __cdecl Render_FogFactor_10058ac2(float z, float arg)
{
    double zero = (double)GF32(0x1012b4a8);
    double one = (double)GF32(0x1012b4ac);
    double start = (double)GF32(0x10143fc8);
    double end = (double)GF32(0x10185bf8);
    double scale = (double)GF32(0x10143fc4);
    double floorFactor = (double)GF32(0x101668cc);
    float fog;

    if (X87_EqualOrUnordered(start, zero)) {
        fog = 1.0f;
    } else {
        if (X87_LessOrUnordered((double)z, start))
            fog = 1.0f;
        else if (!X87_LessOrUnordered(start, end))
            fog = 1.0f;
        else if (!X87_LessOrUnordered((double)z, end))
            fog = 0.0f;
        else
            fog = (float)(one - ((double)z - start) / (end - start));
        if (!X87_LessEqualOrUnordered((double)fog, one))
            fog = 1.0f;
        if (X87_LessOrUnordered((double)fog, zero))
            fog = 0.0f;
    }
    if (!X87_EqualOrUnordered(scale, zero)) {
        if (!X87_LessEqualOrUnordered((double)arg, zero)) {
            float ratio;

            if (!X87_LessEqualOrUnordered(scale, zero))
                ratio = (float)(scale / (double)arg);
            else
                ratio = (float)((double)-arg / scale);
            if (X87_LessOrUnordered((double)ratio, floorFactor))
                fog = (float)((double)fog * floorFactor);
            else if (X87_LessOrUnordered((double)ratio, one))
                fog = (float)((double)fog * (double)ratio);
        } else if (X87_LessOrUnordered(scale, zero)) {
            fog = (float)((double)fog * floorFactor);
        }
    }
    return fog;
}
