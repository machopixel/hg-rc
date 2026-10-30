#include "w2j.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10006a2f
void __cdecl Sim_10006a2f(void)
{
    uint32_t now = G32(0x1015dc78);
    int i;

    for (i = 0; i < 0x200; i++) {
        uint8_t *entry = GBYTES(0x10ac1b00) + i * 0xe;
        int32_t elapsed, count, remainder, n;
        int16_t frame;
        uint32_t period;

        if (S16(entry, 8) < 0 || U16(entry, 6) == 0 || U16(entry, 4) == 0)
            continue;
        if (U32(entry, 0xa) == 0)
            U32(entry, 0xa) = now;
        elapsed = (int32_t)(now - U32(entry, 0xa));
        period = U16(entry, 4);
        count = elapsed / (int32_t)period;
        remainder = (int32_t)((uint32_t)elapsed - (uint32_t)count * period);
        frame = S16(entry, 2);
        if (count <= 0)
            continue;
        for (n = 0; n < count; n++) {
            int32_t v, sign;

            frame = (int16_t)(frame + 1);
            v = frame;
            sign = v >> 31;
            v = ((((v ^ sign) - sign) & 0x1f) ^ sign) - sign; /* keeps the sign, the magnitude is taken modulo 32 */
            frame = (int16_t)v;
            if ((int8_t)GBYTES(0x10a51ac0)[(int32_t)S16(entry, 0) * 0x300 + (int32_t)frame * 0x18] == 0)
                frame = 0;
        }
        if (U16(entry, 6) == 2 && frame < S16(entry, 2)) {
            U16(entry, 6) = 0;
            frame = 0;
        }
        S16(entry, 2) = frame;
        U32(entry, 0xa) = now - (uint32_t)remainder;
    }
}
