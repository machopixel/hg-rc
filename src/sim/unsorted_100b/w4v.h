/*
 * Round 4, letter v: four wrappers of HGSIM.DLL around 0x100b4200 for the 0x88-byte records of the table at
 * 0x101a2100 (count [0x1017d218]); the callback list of a record is the pointer at +0x84 (see unsorted_100c/w4v.h).
 */
#ifndef HG_SIM_UNSORTED_100B_W4V_H
#define HG_SIM_UNSORTED_100B_W4V_H

#include <stdint.h>
#include "../unsorted_100c/w4v.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Sim_100cc6ea(list of record `index`, callback, interval, arg); returns the new node or NULL. */
void *__cdecl Sim_100b41dc(int index, TimerCallback callback, uint32_t interval, uint32_t arg);
/* Sim_100cc8c5 on the list of every record whose flags (+0x18) lack 0x800 and whose list pointer (+0x84) is not NULL. */
void __cdecl Sim_100b4213(void);
/* Sim_100cc79a(list of record `index`, node). */
void __cdecl Sim_100b427b(int index, void *node);
/* Sim_100cc830(list of record `index`). */
void __cdecl Sim_100b42aa(int index);

#endif
