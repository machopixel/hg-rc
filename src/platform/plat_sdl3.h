/*
 * Inside the platform layer only: the one SDL3 window that stands for the game's window (plat_window.c), shared by
 * the window functions of plat.h and the SDL3 input (input_sdl3.c), so SDL wraps the game's window once.
 */
#ifndef HG_PLATFORM_PLAT_SDL3_H
#define HG_PLATFORM_PLAT_SDL3_H

#include <SDL3/SDL.h>

/* Starts SDL3's video once (its own count; SDL_Quit ends it). Nonzero when it runs. */
int PlatSdl_Video(void);
/* The SDL3 window for the OS window `native` (Windows: its HWND), made on first use; NULL when SDL cannot. */
SDL_Window *PlatSdl_Window(void *native);
/* Gives the window back to its owner (the wrap ends; the OS window stays). */
void PlatSdl_Release(void);

#endif
