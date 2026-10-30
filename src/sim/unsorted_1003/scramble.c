#include "fns.h"

/*
 * Three byte-wide running keys derived from the seed: k1 starts at seed*41 and
 * grows by 41 per byte; k2 starts at seed/70+0x35; a counter starts at
 * seed%70 and bumps k2 each time it reaches 70. Per byte: if bit 0 is set add
 * 0x6c, then xor k1, then add k2. Called with the buffer of a data block and
 * the seed read from its header (caller 0x100ae4bc).
 */
// FUNCTION: HGSIM 0x1003c7b0
void __cdecl Sim_1003c7b0(unsigned char *buf, int count, uint32_t seed)
{
    unsigned char k1 = (unsigned char)(seed * 0x29);
    unsigned char k2 = (unsigned char)(seed / 0x46 + 0x35);
    unsigned char n = (unsigned char)(seed % 0x46);

    while (count-- != 0) {
        unsigned char b = *buf;

        if (b & 1)
            b = (unsigned char)(b + 0x6c);
        b ^= k1;
        b = (unsigned char)(b + k2);
        *buf++ = b;
        k1 = (unsigned char)(k1 + 0x29);
        n = (unsigned char)(n + 1);
        if (n == 0x46) {
            n = 0;
            k2 = (unsigned char)(k2 + 1);
        }
    }
}
