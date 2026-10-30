/*
 * Round 7, letter p: functions of 0x100b39c3..0x100bd85b (the entry table at 0x101a2100, a name lookup, freeing a
 * cache object, opening a file along a directory list, a shutdown sequence and the pointer arrays
 * { int capacity; int count; uint32_t *items } of PtrArray_Insert). Helpers that are not converted yet (or that
 * another hook file patches) are called through their original address.
 */
#ifndef HG_SIM_UNSORTED_100B_W7P_H
#define HG_SIM_UNSORTED_100B_W7P_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: no arguments (walks the entries of the table 0x101a2100). */
#define W7P_CALL_100b469d SIM_FN(void(__cdecl *)(void), 0x100b469d)
/* Not converted: (dword 0x1014cf8c, key, dword 0x1014ced8, 1); returns a pointer to a name table or 0. */
#define W7P_CALL_100282d4 SIM_FN(uint8_t *(__cdecl *)(uint32_t, int, uint32_t, int), 0x100282d4)
/* Not converted: _assert(expression, file, line) of the DLL's C library. */
#define W7P_CALL_100ff860 SIM_FN(void(__cdecl *)(const char *, const char *, int), 0x100ff860)
/* Patched by hooks_mem.c (src/sim/mem/syscache.c): (list, extension); returns a record or 0. */
#define W7P_CALL_100ba1c9 SIM_FN(uint8_t *(__cdecl *)(uint32_t, const char *), 0x100ba1c9)
/* Not converted: sprintf(buffer, format, ...) of the DLL's C library. */
#define W7P_CALL_100fc780 SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* Not converted: no arguments, returns a flag the caller ignores. */
#define W7P_CALL_1005f949 SIM_FN(int(__cdecl *)(void), 0x1005f949)
/* Not converted: fclose(stream) of the DLL's C library. */
#define W7P_CALL_100fd760 SIM_FN(int(__cdecl *)(uint32_t), 0x100fd760)
/* The callback 0x100bd134 hands to Sim_100bd69c (Sim_100bd1f3, by its original address so that it stays hookable). */
#define W7P_FN_100bd1f3 SIM_FN(void(__cdecl *)(uint32_t), 0x100bd1f3)

/*
 * Index of key: Sim_100b38c5(key), or a new one from Sim_100b391c(key); -1 (returned) when neither gives one.
 * Otherwise the entry 0x101a2100 + 0x88 * index gets: +0x54.. the 0x30 bytes Sim_10060656(buffer, r0..r5) builds
 * (copied with Sim_10060209), +0x30.. the nine dwords m0..m8, +0 the name (Str_Copy), +0x14 = parent,
 * +0x10 = value, +0x28 = 0, +0x2c = 0 for parent -2, else Sim_100a9830(0 for parent -1 or else the dword +0x2c of
 * the entry parent, 10); +0x18 |= low & 0x1ff and (high << 12) & 0xf000; +0x1c = last. Then 0x100b469d().
 * The argument `unused` is not read. r0..r5 are the last six of the nine dwords.
 */
int __cdecl Sim_100b39c3(int key, const char *name, uint32_t m0, uint32_t m1, uint32_t m2, uint32_t r0, uint32_t r1,
                         uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t value, int32_t parent,
                         uint32_t unused, uint32_t low, uint32_t high, uint32_t last);
/*
 * Looks name up (Str_CompareNoCase) in the table 0x100282d4 answers for key: a signed word count at +8 and 12-byte
 * records from +0xc whose first 10 bytes are the name with every byte negated, then a signed word at +0xa.
 * Returns that word of the record that matches (the record after the last one is tested too), -1 when there is
 * no table or no match; Sim_1002828e(key, dword 0x1014ced8) is called whenever there is a table. The decoded name
 * is kept in a 10-byte stack buffer: the names must hold their terminating zero within the 10 bytes (the
 * original compares past the buffer otherwise).
 */
int __cdecl Sim_100b8c30(int key, const char *name);
/*
 * Frees a cache object (_assert(obj, "C:\projx\Code\sim\Mem\syscache.c", 0xd3) when it is null). When its block
 * +0x18 is set: fn(block) when fn is given, else the dword 0x1017b8f8 takes the dword 0x1017b914 when that is
 * set; then the block is freed (Heap_FreeBlock) and 0x1017b8f8 takes the dword 0x1017b900. Then the object is
 * freed.
 */
void __cdecl Sim_100b9675(void *obj, void(__cdecl *fn)(uint32_t));
/*
 * When the dword 0x1017d600 is set and 0x100ba1c9(it, ext) gives a record: for each of the count (+4) texts in the
 * array at its +8: opens "%s\%s.%s%c" (text, name, ext, the signed byte 0x1017d5f8) and then "%s\%s.%s" with mode
 * "rb" (Sim_100fd850) and returns the first stream that opens. 0 otherwise.
 */
void *__cdecl Sim_100bae0a(const char *name, const char *ext);
/*
 * Shutdown sequence: Sim_10053132, Sim_100db351, Sim_10027e6a, Sim_1003edda, Sim_100556c2, 0x1005f949,
 * Sim_100354ff, Sim_1002ab26.
 */
void __cdecl Sim_100bc451(void);
/*
 * Sim_100bd69c(dword 0x1017e1d0, 0x100bd1f3) and clears it when it is set; frees the dword 0x1017e1c8 and clears it
 * when it is set; fclose(dword 0x1017e1c0) when that is set; then Sim_100bd1b6().
 */
void __cdecl Sim_100bd134(void);
/* Sim_100bd69c(obj[4], 0) and obj[4] = 0 when obj[4] is set (obj is read first), then frees obj. */
void __cdecl Sim_100bd1f3(void *obj);
/*
 * New pointer array of capacity 16 (Heap_Malloc(0xc) and Heap_Malloc(0x40)), count 0, items cleared. Returns 0
 * (after freeing the array) when the item allocation fails; a failed first allocation writes through a null pointer
 * as in the original.
 */
void *__cdecl PtrArray_Create(void);
/* The same with capacity n (items Heap_Malloc(n * 4)). */
void *__cdecl PtrArray_CreateSized(int n);
/*
 * Removes the first item equal to value (items before count, signed compare): fn(item) when fn is given, else
 * Heap_FreeBlock(item) when it is not 0; the slot is cleared, the later items move down by one and the count
 * drops by one. Returns 1 when an item was removed, else 0. items[0] is read first in every case.
 */
int __cdecl PtrArray_Remove(void *arr, uint32_t value, void(__cdecl *fn)(uint32_t));

#endif
