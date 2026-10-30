#include "w5a.h"
#include "w2e.h"
#include "../unsorted_1003/w2e.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x10007c45
void __cdecl Sim_10007c45(void)
{
    char format[16];
    char text[16];

    W5A_SPRINTF(format, SIM_PTR(char, 0x1012dcf0), 0xe, 0xe);
    W5A_SPRINTF(text, format, SIM_PTR(char, 0x1012dc50));
    if (G32(0x10a3ade8) != 0)
        Mono_PutText(text, 0x2f, 0, 0x78);
    G32(0x1015d028) = 0;
}

// FUNCTION: HGSIM 0x1000ebfb
int __cdecl Sim_1000ebfb(const char *name, uint32_t a, uint32_t b, int which, uint32_t c)
{
    int index;

    if (name == 0)
        return 0;
    if (*name == 0)
        return 0;
    index = Sim_1000df9c(name);
    return W5A_CALL_1000e737(0, 0, (uint32_t)index, 0, a, G32(0x101321b4), b,
                             SIM_PTR(u32a, 0x1012df30u)[which], 0xffffffffu, c, 0, 0);
}

// FUNCTION: HGSIM 0x1000ed77
int __cdecl Sim_1000ed77(float v)
{
    float t = (float)((double)FL(G32(0x1014cae0), 0xc) - (double)v);
    int r = Render_Ftol(W5A_SINTAB(t) * GF64(0x101110d8));

    if (r < -7500)
        r = -7500;
    else if (r > 7500)
        r = 7500;
    return r;
}
