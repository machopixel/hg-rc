#include "fns.h"

// FUNCTION: HGSIM 0x1004bf79
int __cdecl Sim_1004bf79(unsigned word) { return (int)((word & 0xF) + 6); }

// FUNCTION: HGSIM 0x1004bf87
int __cdecl Sim_1004bf87(int word) { return ((word >> 4) & 0xF) + 9; }

// FUNCTION: HGSIM 0x1004bf98
int __cdecl Sim_1004bf98(int word) { return ((word >> 8) & 0xF) + 0xB; }

// FUNCTION: HGSIM 0x1004c30d
int __cdecl Sim_1004c30d(int word) { return word >> 16; }

// FUNCTION: HGSIM 0x1004c565
int __cdecl Sim_1004c565(unsigned word) { return (int)((word & 0xF) + 0xE); }

// FUNCTION: HGSIM 0x1004c573
int __cdecl Sim_1004c573(int word) { return ((word >> 4) & 0xF) + 0xE; }

// FUNCTION: HGSIM 0x1004c584
int __cdecl Sim_1004c584(int word) { return ((word >> 8) & 0xF) + 0xE; }

// FUNCTION: HGSIM 0x1004c595
int __cdecl Sim_1004c595(int word) { return ((word >> 12) & 0xF) + 0x16; }

// FUNCTION: HGSIM 0x1004c5a6
int __cdecl Sim_1004c5a6(int word) { return ((word >> 16) & 0xF) + 0x13; }

// FUNCTION: HGSIM 0x1004c909
int __cdecl Sim_1004c909(unsigned word) { return (int)((word & 0xF) + 0x18); }

// FUNCTION: HGSIM 0x1004c917
int __cdecl Sim_1004c917(int word) { return ((word >> 4) & 0xF) + 0x18; }

// FUNCTION: HGSIM 0x1004c928
int __cdecl Sim_1004c928(int word) { return ((word >> 8) & 0xF) + 0x18; }

// FUNCTION: HGSIM 0x1004c939
int __cdecl Sim_1004c939(int word) { return ((word >> 12) & 0xF) + 0x1D; }

// FUNCTION: HGSIM 0x1004c94a
int __cdecl Sim_1004c94a(int word) { return ((word >> 16) & 0xF) + 0x20; }

// FUNCTION: HGSIM 0x1004cd8d
int __cdecl Sim_1004cd8d(unsigned flags) { return (flags & 0x01) != 0; }

// FUNCTION: HGSIM 0x1004cd9e
int __cdecl Sim_1004cd9e(unsigned flags) { return (flags & 0x02) != 0; }

// FUNCTION: HGSIM 0x1004cdd1
int __cdecl Sim_1004cdd1(unsigned flags) { return (flags & 0x10) != 0; }

// FUNCTION: HGSIM 0x1004cde2
int __cdecl Sim_1004cde2(unsigned flags) { return (flags & 0x20) != 0; }
