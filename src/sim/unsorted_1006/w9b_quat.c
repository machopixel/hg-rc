#include "w9b.h"
#include "w3q.h"
#include "w5a.h"

// FUNCTION: HGSIM 0x1006123b
void __cdecl Quat_FromAxisAngles(float a, float b, float c, int order, float *out)
{
    float inner[4];
    float middle[4];
    float outer[4];
    float t[4];

    switch (order) {
    case 1:
        Sim_100610ae(1.0f, 0.0f, 0.0f, a, inner);
        Sim_100610ae(0.0f, 1.0f, 0.0f, b, middle);
        Sim_100610ae(0.0f, 0.0f, 1.0f, c, outer);
        break;
    case 0:
        Sim_100610ae(1.0f, 0.0f, 0.0f, a, middle);
        Sim_100610ae(0.0f, 1.0f, 0.0f, b, inner);
        Sim_100610ae(0.0f, 0.0f, 1.0f, c, outer);
        break;
    case 2:
        Sim_100610ae(1.0f, 0.0f, 0.0f, a, inner);
        Sim_100610ae(0.0f, 1.0f, 0.0f, b, outer);
        Sim_100610ae(0.0f, 0.0f, 1.0f, c, middle);
        break;
    default:
        out[2] = 0.0f;
        out[1] = 0.0f;
        out[0] = 0.0f;
        out[3] = 1.0f;
        return;
    }
    Quat_Multiply(middle, inner, t);
    Quat_Multiply(outer, t, out);
}
