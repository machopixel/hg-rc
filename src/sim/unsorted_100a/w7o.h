/*
 * Round 7, letter o, functions at 0x100axxxx: scene nodes (matrices local +0xc, +0x3c, world +0x4c, flags +0x94,
 * parent +0, first child +4, next sibling +8, object +0x8c), the object that follows the camera on a grid
 * (0x100a9300 / 0x100a94fa), the attachments list (0x100ac001), archive records of 0x14a bytes at 0x101c65e0,
 * the deferred draw list flush (0x100a4c45), the screen layout refresh (0x100ab920) and a file opener.
 *
 * Floating point: computed in double and stored to float where the original stores a float, truncations by
 * Render_Ftol (util/w3o.h); bit-exact. Callees that are not converted (or patched by another hook file) are
 * called through their original address.
 */
#ifndef HG_SIM_UNSORTED_100A_W7O_H
#define HG_SIM_UNSORTED_100A_W7O_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: from the local matrix +0xc of a node to its +0x3c block (src, dst). */
#define W7O_CALL_10060d2e SIM_FN(void(__cdecl *)(const void *, void *), 0x10060d2e)
/* Not converted: takes one entry of the deferred list (the dword at [0x1017d168] + 8 * i). */
#define W7O_CALL_100a539e SIM_FN(void(__cdecl *)(uint32_t), 0x100a539e)
/* Not converted: C library close and seek of a file handle. */
#define W7O_CALL_101049c0 SIM_FN(int(__cdecl *)(uint32_t), 0x101049c0)
#define W7O_CALL_10105fe0 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x10105fe0)
/* Patched by another hook file (0x100d2306) / not converted (0x1007dd90). */
#define W7O_CALL_100d2306 SIM_FN(void(__cdecl *)(void *), 0x100d2306)
#define W7O_CALL_1007dd90 SIM_FN(void(__cdecl *)(void), 0x1007dd90)
/* Not converted: C library function of a double, result in st(0). */
#define W7O_CALL_100fcb70_A hg_fabs

/*
 * Turns the local matrix +0xc of the node by m (Sim_100a9c25 first; Mat3_Multiply for 0x100a9ec6,
 * Mat3x4_Multiply for 0x100a9f6e, both as +0xc = +0xc * m), refreshes +0x3c (0x10060d2e), marks the node
 * changed (bit 0 of +0x94) and counts down +0x98: at 0 it is reloaded from [0x1014d1a8] (which steps
 * to ((v + 7) & 0x7f) + 0x40) and the matrix is renormalised (Sim_100607d5).
 */
void __cdecl Sim_100a9ec6(void *node, const float *m);
void __cdecl Sim_100a9f6e(void *node, const float *m);
/*
 * For the node and all below it: clears bit 0 of +0x94, +0x4c = parent +0x4c * local +0xc (Mat3x4_Multiply;
 * without a parent a copy of +0xc), sets bit 0x8000, hands +0x4c to the object +0x8c (Sim_10009a53) and stores
 * the node in the object's +0x28.
 */
void __cdecl Sim_100aa64a(void *node);
/*
 * Moves the node under `parent` (detaching it first with Sim_100aa8cb when it has a parent) keeping where it is:
 * local +0xc = inverse(parent +0x4c) * node +0x4c, +0x3c refreshed, bit 0x4000 set and bit 0 cleared in +0x94,
 * the parent marked changed; the object's word +0x24 is taken from the parent's object; Sim_100aa799(parent).
 */
void __cdecl Sim_100aaa08(void *node, void *parent);
/*
 * Walks the siblings (+8) and children (+4) of the node: a node whose object +0x8c has the word +0x26 equal to
 * key is removed with everything below it (Sim_100aaf9b(children), Sim_100aaf32(node)).
 */
void __cdecl Sim_100aafe4(void *node, uint32_t key);

/*
 * Places the follower node: the largest extent of its model's vertices in x and z (truncated) gives the grid
 * step [0x1017d1a4] (a third of it for a node without children; rounded up to a multiple of 0x4000) and the
 * distance [0x1017d1a0] = (step + step / 8) / 2; stores the node in [0x1017d1b8] and resets the state.
 */
void __cdecl Sim_100a9300(void *node);
/*
 * When the follower is on: the grid cell of the camera (truncated position / step, rounded to the nearest cell
 * once the camera is more than [0x1017d1a0] away from the last place when [0x1017d1b4] is set); when it
 * changed, moves the follower node to cell * step (x, 0, z).
 */
void __cdecl Sim_100a94fa(void);
/*
 * The attachments list [0x1017d1d0] (next +0x10, kind +4, source +8, node +0x30): kind 1 follows the source's
 * position (+0x58, +0x7c + the float at 0x1012baa0, +0x60), kind 2 the ground height (Sim_1002d961) under the
 * source object's node position, kind 6 counts its time +0x28 up by [0x1015dc80] and fades +0x2c, and at the
 * end of its time (+0x20) its index +0 is removed afterwards (Sim_100ac58e, at most 64 per call).
 */
void __cdecl Sim_100ac001(void);

/* Frees (callback [0x1017d160]) the buffers of archive record `index`, clears it and closes its file. */
void __cdecl Sim_100a7c4c(int32_t index);
/*
 * Loads the entries of archive record `index` (header +6: count word +0xc, entries of 0x18 bytes from +0xe with
 * the file offset +4 and size +8): for each entry with an offset a buffer from the callback [0x1017d15c], a seek
 * and a read (File_ReadChunked); the buffer goes to the record's table at +0x2a. Returns -1 when a buffer is
 * not given or a read is short (that buffer is freed), else 0.
 */
int32_t __cdecl Sim_100a7e55(int32_t index);
/*
 * Flushes the deferred list (entries at [0x1017d168], counts [0x1017d134], [0x101c9ea0], [0x1017d130]) through
 * 0x100a539e; the last [0x1017d130] entries with the render state 0x1014cae8 saved (Sim_100787f4) and changed,
 * then restored. Clears the three counts.
 */
void __cdecl Sim_100a4c45(void);
/* Refreshes the screen rectangles and points for the display size (0x10185400 table, [0x1014bfe4], [0x1014cb50]),
   the view (Sim_100d32e5(0), 0x100d2306 and Sim_100d255e on the camera) and the fixed points (Sim_1000856d). */
void __cdecl Sim_100ab920(void);
/* Opens a file "rb" (Sim_100fd850) behind a 12-byte handle {file, 0, 0} from Heap_Malloc; 0 on failure. */
uint32_t *__cdecl File_OpenRead(const char *name);

#endif
