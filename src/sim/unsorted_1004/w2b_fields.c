#include "w2b.h"
#include "../simmem.h"

#define ROW1 0xAAu
#define ROW2 0x98u

/* Address arithmetic wraps at 32 bits like the original. */
#define T1(addr, row) (*(uint32_t *)(SIM_PTR(unsigned char, addr) + (uint32_t)(row) * ROW1))
#define T2(addr, idx) (*(uint32_t *)(SIM_PTR(unsigned char, addr) + (uint32_t)(idx) * ROW2))

static int Sum3(int row, unsigned word, uint32_t a1, uint32_t a2)
{
    uint32_t sum = T1(a1, row);

    sum += T2(a2, Sim_1004bf79(word));
    sum += T2(a2, Sim_1004bf87((int)word));
    sum += T2(a2, Sim_1004bf98((int)word));
    return (int)sum;
}

static int Sum1(int row, int word, uint32_t a1, uint32_t a2)
{
    uint32_t sum = T1(a1, row);

    sum += T2(a2, Sim_1004c30d(word));
    return (int)sum;
}

// FUNCTION: HGSIM 0x1004c085
int __cdecl Sim_1004c085(int row, unsigned word) { return Sum3(row, word, 0x101116B6, 0x10124EC0); }

// FUNCTION: HGSIM 0x1004c161
int __cdecl Sim_1004c161(int row, unsigned word) { return Sum3(row, word, 0x101116C2, 0x10124EC8); }

// FUNCTION: HGSIM 0x1004c1c1
int __cdecl Sim_1004c1c1(int row, unsigned word) { return Sum3(row, word, 0x101116C6, 0x10124ECC); }

// FUNCTION: HGSIM 0x1004c297
int __cdecl Sim_1004c297(int row, unsigned word) { return Sum3(row, word, 0x101116AA, 0x10124EB0); }

// FUNCTION: HGSIM 0x1004c3ad
int __cdecl Sim_1004c3ad(int row, int word) { return Sum1(row, word, 0x101116C6, 0x10124ECC); }

// FUNCTION: HGSIM 0x1004c4a1
int __cdecl Sim_1004c4a1(int row, int word) { return Sum1(row, word, 0x101116A6, 0x10124EAC); }

// FUNCTION: HGSIM 0x1004c4d1
int __cdecl Sim_1004c4d1(int row, int word) { return Sum1(row, word, 0x101116AA, 0x10124EB0); }
