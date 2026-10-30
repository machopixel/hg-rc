/*
 * Round 5, letter a: functions of 0x10040a37..0x1004ed37 that call helpers which are not converted yet.
 * Those helpers are called through their address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1004_W5A_H
#define HG_SIM_UNSORTED_1004_W5A_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: the C library's sprintf (destination, format, ...). */
#define W5A_SPRINTF SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* Not converted: (byte value, 16-bit value); the original passes the upper half of a pointer with the word. */
#define W5A_CALL_10016de7 SIM_FN(int(__cdecl *)(uint32_t, uint32_t), 0x10016de7)
/* Not converted: one dword. */
#define W5A_CALL_10016632 SIM_FN(int(__cdecl *)(uint32_t), 0x10016632)
/* Not converted: (x, y, width, height, callback address). */
#define W5A_CALL_1004deb3 SIM_FN(int(__cdecl *)(int, int, int, int, uint32_t), 0x1004deb3)

/*
 * p is an 8-byte record: byte +2, word +3 (the kind), word +5. A kind of 0 calls 0x10016632(byte); a kind of 7
 * calls 0x10016de7(byte, word); any other kind does nothing.
 */
void __cdecl Sim_10040a37(const uint8_t *p);
/*
 * Nothing when [0x1017b8c8] == idx. Otherwise formats "'%s' %s" with the name at slot idx + 0x124 (pointer
 * table 0x109e8b40) and the string [0x101c49f8], passes the text to Sim_1007df6c(text, [0x109b98c0], 0x712,
 * 0x50), sets [0x101655ac] to idx & 0xff and [0x1015ec74] to 1 when it was 0. The first argument is not used.
 */
void __cdecl Sim_10041c4c(uint32_t unused, int idx);
/*
 * The float at [[0x1014cae0] + 0x18] is set to 1.0 in [0x1013aecc] and [0x1013aed0] first when flag is not
 * 0, then to [0x1013aecc] when Sim_100448ff() is 0, else to [0x1013aed0]. When that changed the float
 * (compared as floats): [0x1013a014] = 1 and Sim_1000ebfb("AFXHDZMI", -2500, 0, 5, 0x50).
 */
void __cdecl Sim_10044909(int flag);
/*
 * Switches the mode word [0x1013aed8]: while [0x1013aed4] is not 3 it takes that value, the record
 * [0x1014cae0] is saved to 0x109b9a20 (Sim_10044992), [0x1013aed0] becomes 2.0 and Sim_10044909(0) runs.
 * Then Sim_100258d7() gives a record to copy into [0x1014cae0] (Sim_100449fc); without one the mode
 * [0x1013aed8] is applied with Sim_100448cd and, when it is 2, the saved record is copied back.
 */
void __cdecl Sim_10044c7a(void);
/*
 * p: rectangle with the left, top at +4, +8 and the right, bottom at +0xc, +0x10 (inclusive). When p is not
 * null and [0x1013b9a8] is 1: [0x109b8be0] = p, [0x1013b9a8] = 2 and the result of 0x1004deb3(left, top,
 * width, height, 0x10002955) is returned; otherwise 2.
 */
int __cdecl Sim_1004ed37(const uint8_t *p);

#endif
