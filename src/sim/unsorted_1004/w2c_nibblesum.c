#include "w2c.h"
#include "../simmem.h"

static uint32_t Dword(uint32_t addr)
{
    return *SIM_PTR(uint32_t, addr);
}

/* Row entry plus the five entries of the 0x98-stride table at tab. */
static int SumFirst(int row, unsigned word, uint32_t rowAddr, uint32_t tab)
{
    uint32_t sum = Dword(rowAddr + (uint32_t)row * 0xaa);

    sum += Dword(tab + (uint32_t)Sim_1004c565(word) * 0x98);
    sum += Dword(tab + (uint32_t)Sim_1004c573((int)word) * 0x98);
    sum += Dword(tab + (uint32_t)Sim_1004c584((int)word) * 0x98);
    sum += Dword(tab + (uint32_t)Sim_1004c595((int)word) * 0x98);
    sum += Dword(tab + (uint32_t)Sim_1004c5a6((int)word) * 0x98);
    return (int)sum;
}

static int SumSecond(int row, unsigned word, uint32_t rowAddr, uint32_t tab)
{
    uint32_t sum = Dword(rowAddr + (uint32_t)row * 0xaa);

    sum += Dword(tab + (uint32_t)Sim_1004c909(word) * 0x98);
    sum += Dword(tab + (uint32_t)Sim_1004c917((int)word) * 0x98);
    sum += Dword(tab + (uint32_t)Sim_1004c928((int)word) * 0x98);
    sum += Dword(tab + (uint32_t)Sim_1004c939((int)word) * 0x98);
    sum += Dword(tab + (uint32_t)Sim_1004c94a((int)word) * 0x98);
    return (int)sum;
}

// FUNCTION: HGSIM 0x1004c6f5
int __cdecl Sim_1004c6f5(int row, unsigned word)
{
    return SumFirst(row, word, 0x101116b6, 0x10124ec0);
}

// FUNCTION: HGSIM 0x1004c785
int __cdecl Sim_1004c785(int row, unsigned word)
{
    return SumFirst(row, word, 0x101116a6, 0x10124eac);
}

// FUNCTION: HGSIM 0x1004c815
int __cdecl Sim_1004c815(int row, unsigned word)
{
    return SumFirst(row, word, 0x101116aa, 0x10124eb0);
}

// FUNCTION: HGSIM 0x1004ca99
int __cdecl Sim_1004ca99(int row, unsigned word)
{
    return SumSecond(row, word, 0x101116c2, 0x10124ec8);
}

// FUNCTION: HGSIM 0x1004cb29
int __cdecl Sim_1004cb29(int row, unsigned word)
{
    return SumSecond(row, word, 0x101116c6, 0x10124ecc);
}

// FUNCTION: HGSIM 0x1004cbb9
int __cdecl Sim_1004cbb9(int row, unsigned word)
{
    return SumSecond(row, word, 0x101116a6, 0x10124eac);
}

// FUNCTION: HGSIM 0x1004cc49
int __cdecl Sim_1004cc49(int row, unsigned word)
{
    return SumSecond(row, word, 0x101116aa, 0x10124eb0);
}
