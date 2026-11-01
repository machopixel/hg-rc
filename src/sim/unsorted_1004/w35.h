#ifndef W35_H
#define W35_H
#include <stdint.h>
#include "../simmem.h"

/* Round 33, round 3: the mission flow in 1.2's form (w35_menu.c). */

/*
 * The in-mission / mission-end menu, every frame (1.0 0x10048f02, 1.2 0x1003ad72). Unless [0x1017b92c]: the
 * mission-end timers ([0x1015f1d4] music, [0x1015f1d0] end), the end message, the "continue / quit" phase
 * ([0x1015f1e4], [0x1015f208], [0x1015ef10] pending), then the frame's key (Sim_1005572c of [0x1015fd4c]) to the
 * key handler Sim_1004954e or, in the end phase, to its own choices: 0x4b, 0x6b (continue: the player's gear back
 * through the respawn), 0x76 (quit). 1.2's changes: byte +8 of the game settings [0x1017b8c4] is no longer cleared;
 * the end phase's main choices are skipped while [0x1015ef10] is set (its own pending branch takes the key); the
 * continue path resets three view globals (Sim12_10089c60) before the respawn. The arguments are not read; the
 * third one's stack slot holds the key.
 */
void __cdecl Sim_10048f02(uint32_t a, uint32_t b, uint32_t c);

/* 1.2 0x10089c60 (new): [0x1015d1ac] = 0, [0x1015d1cc] = 1, [0x1015d098] = 0 (1.2's 0x1018ea7c / 84 / 74). */
void __cdecl Sim12_10089c60(void);

#endif
