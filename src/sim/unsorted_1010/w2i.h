/*
 * Wave 2 (letter i) additions to unsorted_1010: C runtime code of the DLL (0x10101000..0x10108000): a lookup, a date text, a heap block resize.
 * Plain C, original cdecl convention. Names carry the original address
 * because the code alone does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1010_W2I_H
#define HG_SIM_UNSORTED_1010_W2I_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * *cursor is advanced by 4. Returns the 16-bit word at the old position in the
 * low half and the high half of the NEW cursor in the high half.
 */
uint32_t __cdecl Sim_10101a70(uint32_t *cursor);
/* The second dword of the entry whose first dword is key in the table of 27 entries of 8 bytes at 0x101598b0; 0 if none. */
uint32_t __cdecl Sim_10103800(uint32_t key);
/*
 * MSVC small block heap: grows or shrinks a block inside a region. hdr is the
 * heap header (dword +0x10 = base address of the regions, 0x18 + 8 * region number =
 * per-region counters), region the region header (dword +0 = rover, +4 = its
 * run length, 0xf8 bytes of block sizes after it), block the address of the
 * block's size byte inside the region and newSize the wanted size in the same
 * units. Returns 1 when done and 0 when not (equal size, would pass the
 * region end or the bytes in between are not all zero).
 */
int __cdecl Sim_10106d80(void *hdr, void *region, uint8_t *block, uint32_t newSize);
/*
 * Writes the two decimal digits of n / 10 and n % 10 (each plus '0', low byte) to
 * dst[0] and dst[1]; returns dst + 2.
 */
char *__cdecl Text_PutTwoDigits(char *dst, int n);
/*
 * Formats the broken-down time tm (dwords: seconds, minutes, hours, day of
 * month, month, years since 1900, weekday) as "Www Mmm dd hh:mm:ss yyyy\n" in
 * the static buffer at 0x1017e978 using the name tables at 0x1012c440
 * (weekdays) and 0x1012c458 (months), and returns the buffer.
 */
char *__cdecl Time_FormatTm(const int32_t *tm);
/* Code page 932, 936, 949 or 950 -> 0x411, 0x804, 0x412 or 0x404; 0 for any other. */
uint32_t __cdecl Sim_10107f70(uint32_t codePage);
void __cdecl Sim_101083b0(uint32_t value); /* stores it in the dword at 0x1017e900 */

#endif
