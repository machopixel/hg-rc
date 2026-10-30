#include "w2l.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10037670
void __cdecl Sim_10037670(void)
{
}

// FUNCTION: HGSIM 0x1003775f
void __cdecl Sim_1003775f(uint32_t *p)
{
    uint32_t m[4];
    int i;

    for (i = 0; i < 4; i++)
        p[4 + i] = 0;
    for (i = 0; i < 4; i++)
        m[i] = p[i];
    do {
        for (i = 0; i < 4; i++) {
            if ((m[i] & 1) != 0)
                p[4 + i] = p[4 + i] + 1;
            m[i] >>= 1;
        }
    } while (m[0] != 0 || m[1] != 0 || m[2] != 0 || m[3] != 0);
    for (i = 0; i < 4; i++)
        p[8 + i] = 0;
    for (i = 0; i < 4; i++)
        m[i] = p[i];
    for (i = 0; i < 4; i++) {
        while ((m[i] & 1) == 0 && p[8 + i] < 0x20) {
            p[8 + i] = p[8 + i] + 1;
            m[i] >>= 1;
        }
    }
}

// FUNCTION: HGSIM 0x1003772b
void __cdecl Sim_1003772b(uint32_t *p, uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    p[0] = d;
    p[1] = a;
    p[2] = b;
    p[3] = c;
    Sim_1003775f(p);
}
