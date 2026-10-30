#include "w3q.h"
#include "../render/w2n.h"
#include "../util/w3q.h"

// FUNCTION: HGSIM 0x1008e489
int *__cdecl Sim_1008e489(const int *size, unsigned mode, float scale, int *out)
{
    int w = size[0];
    int h = size[1];
    int w2 = (int)((uint32_t)w << 1);
    int h2 = (int)((uint32_t)h << 1);
    float ratio = (w == 0) ? 0.0f : (float)((double)h / (double)w);
    float negRatio = -ratio;
    int ox, oy;

    switch (mode) {
    case 0:
        if (X87_NOT_ABOVE(scale, ratio)) {
            ox = w2;
            oy = (int)((uint32_t)h + (uint32_t)Render_Ftol((double)w * (double)scale));
        } else {
            oy = h2;
            ox = (int)((uint32_t)w + (uint32_t)Render_Ftol((double)h / (double)scale));
        }
        break;
    case 1:
        if (X87_BELOW(scale, negRatio)) {
            oy = h2;
            ox = (int)((uint32_t)w + (uint32_t)Render_Ftol((double)h / (double)scale));
        } else {
            ox = 0;
            oy = (int)((uint32_t)h - (uint32_t)Render_Ftol((double)w * (double)scale));
        }
        break;
    case 2:
        if (X87_BELOW(scale, negRatio)) {
            oy = 0;
            ox = (int)((uint32_t)w - (uint32_t)Render_Ftol((double)h / (double)scale));
        } else {
            ox = w2;
            oy = (int)((uint32_t)h + (uint32_t)Render_Ftol((double)w * (double)scale));
        }
        break;
    case 3:
        if (X87_NOT_ABOVE(scale, ratio)) {
            ox = 0;
            oy = (int)((uint32_t)h - (uint32_t)Render_Ftol((double)w * (double)scale));
        } else {
            oy = 0;
            ox = (int)((uint32_t)w - (uint32_t)Render_Ftol((double)h / (double)scale));
        }
        break;
    case 5:
        oy = h2;
        ox = w;
        break;
    case 7:
        oy = 0;
        ox = w;
        break;
    default:
        ox = 0;
        oy = 0;
        break;
    }
    out[0] = ox;
    out[1] = oy;
    return out;
}
