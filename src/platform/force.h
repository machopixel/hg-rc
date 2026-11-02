/*
 * Force feedback (rumble) behind the sim's three requests, which used to go to FFORCE.DLL (TSFF_InitSystem,
 * TSFF_SIM_Effect, TSFF_ExitSystem; DECISIONS.md, "Force feedback kept for rumble"). The sim's own effect requests
 * are unchanged; only this end is ours. Today there is no device (force_none.c): Force_Present answers 0 and the sim
 * takes the path the original took when FFORCE.DLL was missing. The rumble work connects this to SDL3 later.
 *
 * The answers are FFORCE.DLL's: 0 for success, a negative HRESULT for a failure.
 */
#ifndef HG_PLATFORM_FORCE_H
#define HG_PLATFORM_FORCE_H

#include <stdint.h>

/* 1 when a force-feedback device can be used (FFORCE.DLL loaded with its three functions), 0 otherwise. */
int Force_Present(void);
/* TSFF_InitSystem(window, arg): opens the device for the game's window. */
int32_t Force_Init(uint32_t window, uint32_t arg);
/* TSFF_SIM_Effect(table): plays the effect the sim wrote into its request table (1.2's form, 0xc0 bytes). */
int32_t Force_Effect(const void *table);
/* TSFF_ExitSystem(): closes the device. */
int32_t Force_Exit(void);

#endif
