/*
 * Round 8, letter y: functions of 0x100a4f03..0x100abcb3 (the deferred scene list of the renderer, the resource
 * archive slots at 0x101c65e0, a direction value from the listener vector, object resets and builders).
 * Helpers that are not converted yet (or that another hook file patches, or that another round-8 helper works on)
 * are called through their original address.
 */
#ifndef HG_SIM_UNSORTED_100A_W8Y_H
#define HG_SIM_UNSORTED_100A_W8Y_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted (round 8, batch x): (scene object); a non-zero result ends the walk of Sim_100a4f03. */
#define W8Y_CALL_100a4933 SIM_FN(int(__cdecl *)(uint32_t), 0x100a4933)
/* Patched by hooks_view.c (SortByDepth): (first entry, last entry) of the 8-byte deferred list. */
#define W8Y_CALL_100a4700 SIM_FN(void(__cdecl *)(uint32_t, uint32_t), 0x100a4700)
/* Not converted: draws one deferred polygon record. */
#define W8Y_CALL_100a539e SIM_FN(void(__cdecl *)(uint32_t), 0x100a539e)
/* Not converted (round 8, batch z): the C library's fabs (double in, st(0) out). */
#define W8Y_CALL_100fcb70 hg_fabs
/* Not converted (round 8, batch A): _read(handle, buffer, count), _close(handle), _lseek(handle, offset, origin). */
#define W8Y_CALL_10105b50 SIM_FN(int(__cdecl *)(uint32_t, void *, uint32_t), 0x10105b50)
#define W8Y_CALL_101049c0 SIM_FN(int(__cdecl *)(uint32_t), 0x101049c0)
#define W8Y_CALL_10105fe0 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x10105fe0)
/* Not converted: builds a 3x4 matrix (12 floats) at its first argument from three positions and three angles
   (raw float words) and a mode word. */
#define W8Y_CALL_10060220 \
    SIM_FN(void(__cdecl *)(float *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x10060220)
/* Not converted: no arguments (the player's own unit). */
#define W8Y_CALL_1002bca0 SIM_FN(void(__cdecl *)(void), 0x1002bca0)
/* Not converted (round 8, batch t): (unit). */
#define W8Y_CALL_10011093 SIM_FN(void(__cdecl *)(uint32_t), 0x10011093)
#define W8Y_CALL_10011681 SIM_FN(void(__cdecl *)(uint32_t), 0x10011681)
/* Not converted: (control record). */
#define W8Y_CALL_100d4590 SIM_FN(void(__cdecl *)(void *), 0x100d4590)
#define W8Y_CALL_100d6833 SIM_FN(void(__cdecl *)(void *), 0x100d6833)
/* Not converted (round 8, batch z): (unit); 0 drops the unit's target word +0xf0. */
#define W8Y_CALL_100cb4fb SIM_FN(int(__cdecl *)(uint32_t), 0x100cb4fb)
/* Not converted (round 8, batch x): (object). */
#define W8Y_CALL_10089e91 SIM_FN(void(__cdecl *)(uint32_t), 0x10089e91)
/* Patched by hooks_sim.c (SystemError): (code, format, argument, 0). */
#define W8Y_CALL_100bc280 SIM_FN(void(__cdecl *)(int, const char *, uint32_t, int), 0x100bc280)

/*
 * Starts a deferred scene list (Sim_100a88db) for the object list: nothing more when list is 0 or its +8 is 0 or
 * equal to its +4. Otherwise the count 0x1017d138 and 0x1017d13c are cleared, 0x101c9ea4 = 1, the write pointer
 * 0x101cc2ac = the list base 0x1017d168; 0x100a4933 is called for the objects chained by +0x20 from list+0x20
 * until one answers non-zero; then more than one entry is sorted (0x100a4700) and every entry's first dword is
 * drawn by 0x100a539e.
 */
void __cdecl Sim_100a4f03(void *list);
/*
 * The same for a scene node tree (Sim_100a526b), but the entries are only collected and sorted (from the list base,
 * 0x101c9ea4 left as it is) and their number goes to 0x1017d134 (cleared first).
 */
void __cdecl Sim_100a4ff0(void *node);
/*
 * As Sim_100a4ff0, but the entries are collected from index [0x101c9ea0] of the list (no
 * Sim_100a88db), their number goes to 0x1017d130 (cleared first) and is added to 0x101c9ea0.
 */
void __cdecl Sim_100a512a(void *node);
/*
 * Walks a scene node tree (first child +4, next sibling +8) while 0x101c9ea4 is set and the count 0x1017d13c is
 * below the limit 0x1017d140 (signed). A node's object (+0x8c) whose kind word (+2) & 0xf0 is not 0x20 counts in
 * 0x101cc2b4; when its flags word (+0) has no 0x1000 and the display driver's test (slot 0x1014cb40) answers 0,
 * 0x101c7a9c counts it and Sim_100a4510(object, float 0x101c7aa0) is called. The tests come before the children.
 */
void __cdecl Sim_100a526b(void *node);

/*
 * A signed value from -127 to 127 for the direction of a point, from the vector at 0x109b16c0 (z), 0x109b16c4
 * (x), 0x109b16c8 (y). With 0x109afdcc set: the dot product of (dx, dy, dz) with that vector. Otherwise with
 * e = vector - (px, py, pz) (each a float): 0x7f when all three |e| are below the float 0x1012ba74 (fabs of the
 * C library); else (dx, dy, dz) . e (a float) divided by the length Sim_10084a94(e). The value times the float
 * 0x1012ba7c is truncated (_ftol, low 16 bits) and limited to -127..127.
 */
int __cdecl Sim_100a648c(float px, float py, float pz, float dx, float dy, float dz);

/*
 * Opens the resource archive: the archive allocators are set (0x100a77ef allocates, 0x100a7800 frees), then
 * Sim_10027f06; the archive name 0x1017d154 is made once (a copy of Sim_100474db(text 0x1014d0f8)). The slot of
 * Sim_100a79db(name, 0) goes to 0x1014cf8c; when there is one its tables are loaded (Sim_100a7e55), otherwise
 * SystemError(3, text 0x1014d104, name, 0) is reported and 0 returned. Returns 1 otherwise.
 */
int __cdecl Sim_100a76a0(void);
/*
 * Opens an archive into the free slot of the slot table 0x101c65e0 (one slot of 0x14a bytes; in use when the word
 * +4 is set). mode 2: opened with flags 0x8002, the 12-byte header must start with the 4 bytes at 0x1014d194, and
 * a header byte +8 of 0xfe closes it again and gives -2; mode 0: flags 0x8000, the 4 bytes at 0x1014d19c; other
 * modes -1. Then the dword at offset 0x10 is read, the directory (that size + 8 bytes from offset 0xc) is read into
 * a block of the archive allocator (freed again when the read is short). The slot gets the handle (+0), the block
 * (+6), in use (+4 = 1) and the name (+0x12a, 0x20 bytes cleared, then at most 0x20 copied). Returns the slot
 * (0), or -1 when there is no free slot or something fails.
 * The handle is opened with Sim_10109980(name, flags, 0): the original leaves the third word unset, which _sopen
 * only reads for _O_CREAT (not in these flags). A short header read compares the buffer's previous contents, as
 * in the original (the buffer is a local of the same size).
 */
int __cdecl Sim_100a79db(const char *name, int mode);
/*
 * Position of item `sub` of the directory entry named key (Sim_100a7da3; its tables are loaded with Sim_100a7e55
 * when missing, -1 when that fails): table[sub] + the entry's word +0x14 into *outPos and table[sub + 1 word] - that
 * word into *outEnd (when given), and the archive is positioned there (_lseek). Returns 0. The entry index is
 * compared with -1 after a 16-bit zero extension, so a missing name is not caught (as in the original).
 */
int __cdecl Sim_100a80d2(int index, const char *key, uint32_t sub, uint32_t *outPos, uint32_t *outEnd);
/*
 * Reads item `sub` (16 bits) of the directory entry named key into buffer: -1 when the entry is missing, its
 * tables cannot be loaded, the seek fails or the read is short; 0 otherwise.
 */
int __cdecl Sim_100a8238(int index, const char *key, uint32_t sub, void *buffer);
/*
 * Item `sub` (16 bits) of an archive: below 0xf000 Sim_100a8238(index, key, sub, buffer); else item sub - 0xf000 of
 * the record 0x1017d158 (Sim_100c6f39 size, Sim_100c6fbd read, Sim_100c6fb8): -1 when there is no such item or the
 * read is short, else 0.
 */
int __cdecl Sim_100a836e(int index, const char *key, uint32_t sub, void *buffer);
/*
 * Like Sim_100a8238 without reading: returns the position of the item (0 when the entry is missing or its tables
 * cannot be loaded) and stores the archive handle in *outHandle.
 */
uint32_t __cdecl Sim_100a8405(int index, const char *key, uint32_t sub, uint32_t *outHandle);

/*
 * Sets bit 0 of the node's flags (+0x94), then applies three matrices of 0x10060220 to it (Sim_100a9f6e): built
 * from (0, 0, 0, -x, -y, -z, mode), (a, b, c, 0, 0, 0, mode) and (0, 0, 0, x, y, z, mode). a, b, c and
 * the float words x, y, z are passed on unchanged (negated through the x87 for the first matrix).
 */
void __cdecl Sim_100aa016(void *node, uint32_t a, uint32_t b, uint32_t c, uint32_t mode, uint32_t x, uint32_t y,
                          uint32_t z);

/*
 * Resets the control record +0x28 of obj (nothing when there is none). The record's +0 is its unit, whose +0x48
 * is its scene node and +0x54 its state block. 0x1002bca0 when the unit (+4) is the player's (0x1017b8c8); +0x53c
 * takes the word +0x6c of the node's object; the two timers +4 and +0x14 restart with 0.2 (Sim_10047b76); the
 * words +0x4fc, +0x4e8, +0x508, +0x4f4, +0x4f0, +0x4d8, +0x4e0, +0x504, +0x50c, +0x54c, +0x544 are cleared and
 * the flags word +0x560 becomes 0x2000. The node is moved by (0, float +0x520, 0) (Sim_100a9d7c), updated
 * (Sim_100aa799), its orientation set from the unit's +0x64..+0x6c (Sim_100a9ca3) and its position copied to
 * the unit's +0x58..+0x60 (Sim_100a9cfb). The unit's +0x94 and +0x90 are cleared, +0x154 = Sim_100ac21a(1,
 * unit), Sim_100aa454(node); in the state block +0x46 and +0x31 (bytes) and +8 are cleared; then 0x10011093(unit)
 * and +0x4d4 = 0.
 */
void __cdecl Sim_100a89f0(void *obj);
/*
 * One step of a control record (unit at +0): 0x10011681(unit). In state 2 (+0x4f0): 0x100d4590(record); the
 * unit's target word +0xf0 is dropped for a unit of kind (+0x14) 2, for kind 3 of the player (+0xc ==
 * 0x1017b8c8), and when it has no 0x1000 bit and 0x100cb4fb(unit) answers 0; 0x100d6833(record); +0x50c and the
 * state block's byte +0x46 cleared, its +8 too when the flags word +0x560 has 4 and not 8. With the float +0x534
 * at least 360 the heading (state block +4) is taken: +0xc moves to heading - (heading - +0xc brought into
 * -180..180 by steps of 360) and +8 = heading; otherwise +8 is limited to -+0x534..+0x534. +0x18 = state block
 * +0. In the other states +8, +0x18 and the state block's +8, +4, +0 are cleared. Then the state moves on:
 * 0 -> 1 with the time +0x4d8 = now (0x1015dc78) + 0x43e + Sim_1009c929(0x16a); 1 -> 2 once now passes +0x4d8;
 * 3 -> 0; in 4 +0x4d8 is set to now + 0x712 (Sim_1007cbb4) when it is 0 and Sim_10061c32(record) runs while
 * now is before it.
 */
void __cdecl Sim_100a8d09(void *ctl);
/*
 * Walks the scene node tree depth first, next sibling (+8) first: the first node whose object's word +0x26
 * equals key gets Sim_10028f38(first child, 0x100020e5, key) and Sim_10028d9f(node, 0x100020e5, key), and its
 * children are not walked.
 */
void __cdecl Sim_100ab054(void *node, uint32_t key);
/*
 * Builds up to 64 flat squares: clears 0x1017d1d0..0x1017d1dc and 0x1017d1cc; each is an object of
 * Sim_10008f1c(4, 1, 0x10, &extra) (none: stop) with the corners (+-1000, 0, +-1000), one polygon of id
 * 0x9000 | i (Sim_100091f7, fields +0x10 and +0x1c = 1.0, the others 0) over the four corners, a scene node
 * Sim_100a9830(0, 0xc) linking both ways, and the usual updates; the counts 0x1017d1c8 (+1 each) and slot tables
 * 0x101c56c0 / 0x101c57e0 (0x34 bytes: index, node +0x30, cleared +4..+0x10) are filled. 0x1017d1c4 = count - 1.
 */
void __cdecl Sim_100abcb3(void);

#endif
