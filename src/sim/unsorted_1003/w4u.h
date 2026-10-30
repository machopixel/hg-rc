/* Round 4 (letter u), originals in 0x10030150..0x1003913b: block lists and small heap users. */
#ifndef HG_SIM_UNSORTED_1003_W4U_H
#define HG_SIM_UNSORTED_1003_W4U_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
/*
 * One step of the refresh-mode test (strings "Refresh mode %d start=..."), driven by the counter dword 0x1015e9cc:
 * at 1 it reads the high-resolution counter into the qword 0x1015e680, at 5 it reads it again, stores the low-word
 * difference in dword +0xc of the current mode object (dword 0x1015e68c) and moves on to the next mode object whose
 * function at +0x10 succeeds (marking the ones that fail as unavailable, +8 = 0), finally calling Sim_10037e8f when the
 * active flag 0x1015e694 is 0; any other value of the counter just increments it. The mode numbers are compared with
 * the dword 0x1013a2c4. A failing counter read clears the active flag 0x1015e694.
 */
void __cdecl Sim_10037cab(void);
/*
 * The name is the one the DLL prints ("ToggleFullScreen(1): pause_timer(TRUE)"). Does nothing while the flag 0x1015e694 is
 * set, or while the window already covers the display (dwords 0x1015e9c0 == 0 and 0x1015e9c4/0x1015e9c8 not above the
 * size at 0x109e3d14/0x109e3d18). Otherwise pauses the timer (Timer_Pause(0x80, 1)), picks the mode object to switch to,
 * calls the old object's function +0x14 and the new one's +0x10, and applies the palette through the object 0x1015e688
 * (+0x14); when it finds no usable mode it only prints a message. Un-pauses the timer on every path except the first one.
 */
void __cdecl ToggleFullScreen(void);

#endif

/* Allocates an 8-byte list node (not zeroed) from the game heap; returns it or NULL. */
void *__cdecl ListNode_Alloc(void);
/*
 * The owner record keeps a singly linked list of nodes {next, value}: head pointer at +0x1c, a counter
 * at +0x18. List_FreeNodes frees every node and clears the head (nothing for a NULL owner or an empty
 * list; the counter is left alone).
 */
void __cdecl List_FreeNodes(void *owner);
/* Adds a node holding `value` at the front of the owner's list and increments the counter. Returns the node, or 0 for a NULL owner or value or when the allocation fails. */
void *__cdecl List_PushFront(void *owner, void *value);
/* Allocates 0x644 bytes (not zeroed) and sets the six dwords at +0, +8, +0x10 to 0x7f7fc99e and at +4, +0xc, +0x14 to 0xff7fc99e. Returns the block, or NULL. */
void *__cdecl Sim_1003058f(void);
/* For a non-NULL owner: List_FreeNodes on each of the 7 x 7 list records (32 bytes each, row stride 0xe0) from +0x24, then frees the owner. */
void __cdecl Sim_100305f8(void *owner);
/* When the flag dword 0x1015e570 is set: FreeLibrary of the module handle at 0x1015e568. Returns 1. */
int __cdecl Sim_1003693c(void);
/*
 * Searches the records of the doubly linked list whose last record is the dword 0x109e3760 (links: previous at
 * +0x690, next at +0x694; dword 0x109e3770 is the first) from the last backwards for the one whose first dword equals
 * `key`. That record is unlinked (the heads are fixed up), passed to Sim_10038ad5(other, record) and freed.
 */
void __cdecl Sim_1003913b(uint32_t key, void *other);

/*
 * One step of the refresh-mode test (strings "Refresh mode %d start=..."), driven by the counter dword 0x1015e9cc:
 * at 1 it reads the high-resolution counter into the qword 0x1015e680, at 5 it reads it again, stores the low-word
 * difference in dword +0xc of the current mode object (dword 0x1015e68c) and moves on to the next mode object whose
 * function at +0x10 succeeds (marking the ones that fail as unavailable, +8 = 0), finally calling Sim_10037e8f when the
 * active flag 0x1015e694 is 0; any other value of the counter just increments it. The mode numbers are compared with
 * the dword 0x1013a2c4. A failing counter read clears the active flag 0x1015e694.
 */
void __cdecl Sim_10037cab(void);
/*
 * The name is the one the DLL prints ("ToggleFullScreen(1): pause_timer(TRUE)"). Does nothing while the flag 0x1015e694 is
 * set, or while the window already covers the display (dwords 0x1015e9c0 == 0 and 0x1015e9c4/0x1015e9c8 not above the
 * size at 0x109e3d14/0x109e3d18). Otherwise pauses the timer (Timer_Pause(0x80, 1)), picks the mode object to switch to,
 * calls the old object's function +0x14 and the new one's +0x10, and applies the palette through the object 0x1015e688
 * (+0x14); when it finds no usable mode it only prints a message. Un-pauses the timer on every path except the first one.
 */
void __cdecl ToggleFullScreen(void);

#endif
