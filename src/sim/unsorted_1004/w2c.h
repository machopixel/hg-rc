/*
 * Wave 2 functions of HGSIM.DLL around 0x1004c6f5..0x1004cc49 that call the
 * nibble accessors of fns.h: each adds one entry of a 0xaa-stride table row to
 * five entries of a 0x98-stride table, the entries chosen by the five fields
 * of a packed word.
 */
#ifndef HG_SIM_UNSORTED_1004_W2C_H
#define HG_SIM_UNSORTED_1004_W2C_H

#include "fns.h"

/* Group using Sim_1004c565/73/84/95/a6. */
int __cdecl Sim_1004c6f5(int row, unsigned word);
int __cdecl Sim_1004c785(int row, unsigned word);
int __cdecl Sim_1004c815(int row, unsigned word);
/* Group using Sim_1004c909/17/28/39/4a. */
int __cdecl Sim_1004ca99(int row, unsigned word);
int __cdecl Sim_1004cb29(int row, unsigned word);
int __cdecl Sim_1004cbb9(int row, unsigned word);
int __cdecl Sim_1004cc49(int row, unsigned word);

#endif
