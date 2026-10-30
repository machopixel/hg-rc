/*
 * Round 6, letter h: functions at 0x100d36f5..0x100db0ee that call helpers which are not converted yet (called
 * through their original address, SIM_FN, with the prototypes below) or Windows through the import slots. Floating
 * point is x87 in the originals at the 53-bit precision the Windows loader sets: intermediates are doubles, stored
 * results floats.
 */
#ifndef HG_SIM_UNSORTED_100D_W6H_H
#define HG_SIM_UNSORTED_100D_W6H_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: (name, dword, 0); returns a pointer to the loaded data or 0. */
#define W6H_CALL_100ba7d5 SIM_FN(uint8_t *(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x100ba7d5)
/* Not converted: (destination, source pointer). */
#define W6H_CALL_100a9e1a SIM_FN(void(__cdecl *)(void *, void *), 0x100a9e1a)
/* Hooked by another hook file, so always called at its original address: (code, 0). */
#define W6H_CALL_100bc280 SIM_FN(void(__cdecl *)(uint32_t, uint32_t), 0x100bc280)
/* Not converted: (the block just loaded). */
#define W6H_CALL_10078f60 SIM_FN(void(__cdecl *)(void *), 0x10078f60)

/*
 * Runs once (nothing when the dword 0x1017e65c is above 0 or 0x1017e660 is set): sets 0x1017e660, stores
 * 0x10185620 + 16 * [0x10152420] in 0x1017e664 and loads it with 0x100ba7d5(that, [0x1014ced4], 0). With data:
 * Sim_1008c550(0x1017e640, data, first, count), copies 0x300 bytes of the data to the block [0x1017e644], then
 * byte 0x1017e648 = 0x80 and the (unaligned) dword 0x1017e649 = 0x10.
 */
void __cdecl Sim_100d36f5(uint32_t first, int32_t count);
/*
 * Sim_100a77a7(name, [0x1014ceb4]) for every one of the 0x2e entries (0xd6 bytes from 0x10145508) whose text at
 * +0xb2 is not empty, then for every one of the 0x4d entries (0x3c bytes from 0x10147b88) whose first dword is
 * above 0, with the text at +0x24.
 */
void __cdecl Sim_100d62bd(void);
/*
 * *a = 0, *b = 0, *c = 1.0f; reads three floats p, q, r with Sim_100a9ca3(obj[0x4c], ...). With s = obj[0x28]:
 * when bit 4 or 0x400 of the word s[0x562] is set, p += s[0x1c]; else p = s[0x1c]. In both cases, when the
 * dword [s[0]] is 1 or 9, q += s[0xc] / 4.0f. Builds a matrix with Sim_10060656(m, p, q, r, 0, 0, 0) and turns
 * (*a, *b, *c) with it (Sim_1005fc4f).
 */
void __cdecl Sim_100d6f92(void *obj, float *a, float *b, float *c);
/*
 * With s = obj[0x28] and e = s[0x84] + 0x128 * s[0x4fc]: reads three dwords with Sim_100d709d(obj, .., e), copies
 * Sim_100aa863(s[0x498 + 4 * e[0x11c]]) to dst with 0x100a9e1a and stores the three dwords with Sim_100a9d47.
 */
void __cdecl Sim_100d7111(void *obj, void *dst);
/*
 * Shared by 0x100d8d0a (limit 1e-7f at 0x1012bd68, state 0x5e) and 0x100d9306 (limit 0.0f at 0x1012bd60,
 * state 0xdf): see Sim_100d8d0a.
 */
/*
 * With rec = obj[0x57a] and node = obj[0x552]: reads the node's position (Sim_100a9cfb). While its second float is
 * above the limit: moves the node by ((obj[0x56e] + rec[0x14]) * k, (obj[0x572] - Sim_1009c929(10) + rec[0x18]) * k,
 * (obj[0x576] + rec[0x1c]) * k) with k = [0x1015dc80] / 181.0f, Sim_100aa799(node), obj[0x566] = state, returns 1.
 * Otherwise clears bit 0 of the word obj[0x10], Sim_100aa351(node), SlotPool_Release(rec[0x20], rec[0]), returns 0.
 */
int __cdecl Sim_100d8d0a(void *obj);
int __cdecl Sim_100d9306(void *obj);
/*
 * Nothing when the distance of (x, y, z) from the camera block at [0x1014cae0] is above its float +0x40, or when
 * SlotPool_Take(owner) or Sim_10090c7d(0x10001659, 0x1000320b, slot, 0) gives nothing (the slot is then released).
 * Otherwise fills the new object o from the position and the speed (vx, vy, vz) and lays 0x20 parts (0x2a bytes
 * from o + 0x12) along the speed in steps of a sixteenth, with random offsets from Sim_1009c929.
 */
void __cdecl Sim_100d9064(float x, float y, float z, float vx, float vy, float vz, void *owner);
/*
 * Frees the block at 0x1017e688 (when set), clears 0x1017e688, 0x1017f118, 0x1017e690 and stores a copy of data
 * (its size is the dword data[4]) in a new heap block at 0x1017e688; 0x1017f118 = (size - 8) / 12. Returns 1;
 * 0 after 0x100bc280(0x3f, 0) when the allocation fails.
 */
int __cdecl Sim_100da098(const void *data);
/*
 * As Sim_100da098 for the slot data[8] of the tables 0x1017ee40 (block) and 0x1017ee80 (count (size - 8) / 0x97),
 * error code 0x40; the new block is also handed to 0x10078f60.
 */
int __cdecl Sim_100dac0a(const void *data);
/*
 * For each of the (data[4] - 8) / 0x18 entries (0x18 bytes from data + 8: two dwords, then a name): stores the
 * dwords in the 0x6c-byte records at 0x109b7d4c (+0 and +8) and the second one also at 0x109b8400 + 4 * first;
 * Sim_1005259e(i, name) with the text [0x1017e6b0] for the entry [0x1013bb40] and [0x1017e6b4] for the others
 * when set, else the entry's own name; then clears the record's dword +4.
 */
void __cdecl Sim_100db0ee(const void *data);

#endif
