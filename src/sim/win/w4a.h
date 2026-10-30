/*
 * Round 4 (letter a): small functions of HGSIM.DLL that call Windows through
 * the DLL's import slots. Plain C, original cdecl convention. These are the
 * game's memory heap and library loading wrappers.
 */
#ifndef HG_SIM_WIN_W4A_H
#define HG_SIM_WIN_W4A_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Frees a block of the game's heap (the handle at 0x1017b8f8). Does nothing for NULL. */
void __cdecl Heap_Free(void *block);

/* Allocates a zero-filled block of `size` bytes from the game's heap. Returns the block (or NULL). */
void *__cdecl Heap_AllocZero(unsigned int size);

/* Allocates a zero-filled block of 0xfc4 bytes and stores it in the pointer at 0x1015dc30. */
void __cdecl Sim_10027d5a(void);

/* Loads a DLL by name, remembers the module in 0x1015eb68 and returns it. */
void *__cdecl Lib_Load_1003c569(const char *name);

#endif
