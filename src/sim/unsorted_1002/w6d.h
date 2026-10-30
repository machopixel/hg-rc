/*
 * Round 6, letter d: functions in 0x10024548..0x1002fd4c that call helpers which are not converted yet (called
 * through their original address with SIM_FN, prototypes below) besides converted ones.
 */
#ifndef HG_SIM_UNSORTED_1002_W6D_H
#define HG_SIM_UNSORTED_1002_W6D_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: twelve dwords. */
#define W6D_CALL_100246b3 SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100246b3)
/* Not converted: (dword, object, float, three floats passed as their bits). */
#define W6D_CALL_10025aee SIM_FN(void(__cdecl *)(uint32_t, uint32_t, float, uint32_t, uint32_t, uint32_t), 0x10025aee)
/* Not converted: three dwords. */
#define W6D_CALL_100d3510 SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x100d3510)
/* Patched by another hook file: five dwords here (a message number, a text and its arguments). */
#define W6D_CALL_100bc280_5 SIM_FN(void(__cdecl *)(uint32_t, const void *, const void *, int, uint32_t), 0x100bc280)
/* Not converted: one pointer. */
#define W6D_CALL_10027fb8 SIM_FN(void(__cdecl *)(void *), 0x10027fb8)
#define W6D_CALL_100aa8cb SIM_FN(void(__cdecl *)(uint32_t), 0x100aa8cb)
#define W6D_CALL_1002a2f6 SIM_FN(void(__cdecl *)(void *), 0x1002a2f6)
/* Not converted: no arguments. */
#define W6D_CALL_1009bd47 SIM_FN(void(__cdecl *)(void), 0x1009bd47)

/*
 * Releases the 0x156-byte record index of 0x1096fc80: [0x10134890] = -1 when its +0x60 is set, Sim_100aa351 and
 * Sim_100aa406 on its tree +0x3c, Sim_100585b3(+0x68), Sim_100ac58e(+0x6c), both set to -1; when flag is set
 * 0x100246b3(+0x50, a, +8, b, c, d, e, f, g, 0, 0, 0) (+8 read before the releases); then Sim_10021c3c(index).
 */
void __cdecl Sim_10024548(uint32_t a, int index, int flag, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f,
                          uint32_t g);
/*
 * For every entry of the 0x50-byte table 0x109e9ba0 (count [0x1015dc68], last first) without bit 4 in its word +0
 * whose object Sim_100b53bb(+8) exists: when the distance (Sim_10084a94) from (x, y, z) to the object's centre
 * (Sim_1000946e) is below its radius + extra, calls 0x10025aee(a, object, [0x1015dc80] * scale, centre).
 */
void __cdecl Sim_100257ec(uint32_t a, float x, float y, float z, float extra, float scale);
/*
 * [0x1015dbd0] = 10000.0, 0x100d3510(0x12, 2 * [0x101480a8], 2), copies the point obj + 0x58 to 0x1015dbf8,
 * [0x1015dc0c] = 2 * [0x101480a8], [0x1015dbf4] = 0 and returns Sim_10024616(-2, 0x16, point, point). Before that it
 * looks for an entry of the 0x2c-byte table 0x109a5a40 (from the last of 256) with +0x24 == 0x16 and an object whose
 * +0x8c is set (only to store 10000.0 again).
 */
uint32_t __cdecl Sim_10025f6b(void *obj);
/*
 * Steps the stream obj to its next block: the block obj[0x1b] is 0xc bytes long when its first dword is [0x101349e8],
 * else its second dword says the length. When the next block lies before obj[0x17] + obj[0xf] it becomes obj[0x1b]
 * and is returned if its length (+4) is from 8 to obj[0x13]; otherwise error 0x2c is reported with 0x100bc280
 * (text 0x10134fd8, obj + 6, the word obj[4]). Returns 0 except in the first case.
 */
void *__cdecl Sim_100279f1(void *obj);
/*
 * When the hash table [0x1015dc30] exists: 0x10027fb8 on every node of its 0x3f1 chains (next at +8), clears
 * 0x109f13e0, 0x109f13f0, 0x109f13f4 and frees the table with HeapFree([0x1017b8f8], 1, table).
 */
void __cdecl Sim_10027e6a(void);
/*
 * Finds the 0x30-byte slot of 0x109eeb00 (128) whose +4 is key, else the first one whose +4 is 0; -1 when key is 0 or
 * there is none. Clears it (Sim_1002990f), counts [0x1015dc40] up, 0x100aa8cb(key), and sets +0 = value, +4 = key,
 * +0x20 = -[0x1013abc0]. Returns the slot number.
 */
int __cdecl Sim_10028b90(uint32_t key, uint32_t value);
/*
 * For every used entry (+0 and +4 set) of the 0x14-byte table 0x109f09e0 (last of 128 first): when the object
 * Sim_100a9b71(+4) has its centre (Sim_1000946e) within its radius + extra of (x, y, z) (squared distances
 * compared), Sim_100291a2(entry number, [0x1015dc80] * scale).
 */
void __cdecl Sim_100299c8(float x, float y, float z, float extra, float scale);
/*
 * Clears PTRTAB(index) and allocates the record (Sim_1002a29f); then 0x1002a2f6(record), record[4] = index,
 * record[0x54] = 0x1015fd38 for the current index ([0x1017b8c8]) or record + 0x2a0, 0x5c bytes there cleared, and
 * callback(index, record) when callback is not null.
 */
void __cdecl Sim_1002a1f2(int index, void(__cdecl *callback)(int, void *));
/*
 * 0x1009bd47() when byte 9 of [0x1017b8c4] is set or [0x1015ec58] is 0; then for the 30 objects at 0x109e88e0 the
 * function at +0x108 (when set) on the object, and HeapFree([0x1017b8f8], 1, object); Sim_100481dd(-1); returns
 * HeapFree([0x1017b8f8], 1, [0x1015dc2c]).
 */
int __cdecl Sim_1002c63a(void);
/*
 * Sim_1002fa3f(obj), then with the box b = obj[0x98]: inside = x in [b0, b4] and z in [b10, b14] (x87 compares: a
 * NaN bound at +4 or +0x14 lets it pass); *in = inside, *value = b[0xc] when inside, and *above = inside and y in
 * [b8, bc]. All 0 without a box.
 */
void __cdecl Sim_1002ce45(void *obj, float x, float y, float z, int *above, int *in, uint32_t *value);
/*
 * The bounding box of the 0x58-byte entries (count: word +4) at +0x18 of the block obj[0x2c], from the floats at
 * +0xc, +0x10, +0x14 of each entry (Sim_1002fd4c: +0, +4, +8); refreshed with Sim_100aa307 first when obj[0x9c]
 * differs from block[0x10]. All six results 0 without a block; start values +-3.3999999521e38.
 */
void __cdecl Sim_1002fba5(void *obj, float *min_x, float *max_x, float *min_y, float *max_y, float *min_z,
                          float *max_z);
void __cdecl Sim_1002fd4c(void *obj, float *min_x, float *max_x, float *min_y, float *max_y, float *min_z,
                          float *max_z);

#endif
