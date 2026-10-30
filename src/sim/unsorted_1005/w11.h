/*
 * Round 11: game functions of HGSIM.DLL around 0x10050000..0x1005ffff, converted one at a time.
 */
#ifndef HG_SIM_UNSORTED_1005_W11_H
#define HG_SIM_UNSORTED_1005_W11_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* Language setup: Lang_ReadIni(dll name 0x10144260, sim file 0x10144270), the string table [0x1017d594] =
 * Sim_100b69d0(sim file), Text_InitStrings; then the language DLL is loaded (handle 0x101668ec) and three of its
 * functions fetched (0x1015dc60 / 0x1015dc3c / 0x1015dc5c): the first gets the language record [0x10144230]
 * (its +0xa goes to 0x10157590), the second decides the flag 0x101668e8 when the record's +0xe is set, the third
 * gets the record's language word and three buffers; the DLL is freed (handle cleared when that succeeds).
 * Without the DLL only Sim_10083495 runs. */
void __cdecl Sim_1005f6c6(void);

/* The fatal-error box for message group code (0x6e / 0x6f / 0x70 / other) with the item text key (+ the number
 * value for 0x6f): once only (0x101653c8) and not for the key at 0x10141da4; the text is built into 0x1015f468
 * with sprintf from the string table, Sim_1005f9aa logs it, the cursor is shown (ShowCursor until visible) for
 * MessageBoxA; answer 2 (Cancel) ends the game (Sim_100bc451, exit(code)); a cursor that was hidden is hidden
 * again. */
void __cdecl Sim_1005573d(int code, uint32_t value, const char *key);

#endif
