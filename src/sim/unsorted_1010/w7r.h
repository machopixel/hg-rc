/*
 * Round 7, letter r: C runtime code of the DLL around 0x10100000..0x10104000: environment lookup, the heap
 * front end (resize and zeroed allocation), the fatal-error exit, the three double-to-text layouts of the
 * formatter and two math library helpers.
 *
 * Callees that are not converted are called through their original address (SIM_FN):
 *   0x10107910 (no arguments)                   0x101069c0 (paragraphs -> small block, or 0)
 *   0x10108430 (code)                           0x10100f00 (value, buffer, digits -> buffer)
 *   0x10103ad0 (new control word, mask)
 */
#ifndef HG_SIM_UNSORTED_1010_W7R_H
#define HG_SIM_UNSORTED_1010_W7R_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

typedef int(__cdecl *W7rVoidFn)(void);
typedef uint32_t(__cdecl *W7rSmallAllocFn)(uint32_t paragraphs);
typedef void(__cdecl *W7rCodeFn)(int code);
typedef int(__cdecl *W7rExitFn)(int code);
typedef char *(__cdecl *W7rFormatFn)(const uint32_t *value, char *buf, int ndig);
typedef int(__cdecl *W7rCtrlFpFn)(uint32_t newWord, uint32_t mask);

#define W7R_FN_10107910 SIM_FN(W7rVoidFn, 0x10107910)
#define W7R_FN_101069C0 SIM_FN(W7rSmallAllocFn, 0x101069c0)
#define W7R_FN_10108430 SIM_FN(W7rCodeFn, 0x10108430)
#define W7R_FN_10100F00 SIM_FN(W7rFormatFn, 0x10100f00)
#define W7R_FN_10103AD0 SIM_FN(W7rCtrlFpFn, 0x10103ad0)

/* ---- w7r_env.c ---- */
/*
 * Looks name up in the environment table (dword 0x1017e8b0, NULL-terminated array of "NAME=value" texts).
 * When the table is NULL and the dword 0x1017e8b8 is set, 0x10107910 is called first (non-zero answer: NULL)
 * and the table is read again. An entry matches when it is longer than name, has '=' right after name's
 * length and Sim_101078d0(entry, name, length) is 0; returns the text after the '=' of the first match,
 * else NULL (also for a NULL name or table).
 */
char *__cdecl Crt_GetEnv(const char *name);
/*
 * Runs Sim_101083f0 when the dword 0x1017e8fc is 1 (or 0 with the dword 0x1017e900 at 1), then
 * 0x10108430(code), then the function in the dword 0x101596fc with 0xff; returns what that returns.
 */
int __cdecl Sim_101009b0(int code);

/* ---- w7r_heap.c ---- */
/*
 * Resizes block to size bytes: NULL block -> Heap_Malloc(size); size 0 -> Heap_FreeBlock, NULL. Sizes up to
 * 0xffffffe0 are rounded up to 16. A block of the small block heap (Sim_10106900) below the limit in dword
 * 0x1015be5c is first resized in place (Sim_10106d80), else moved to 0x101069c0(size / 16); failing that (or
 * over the limit) to HeapAlloc(heap 0x10ac3714, 0, size). On a move min(old paragraphs * 16, size) bytes are
 * copied and the old block freed with Sim_10106960. Any other block goes to HeapReAlloc(heap, 0, block, size).
 * When the result is NULL and the dword 0x1017e968 is set, Sim_10106590(size) decides whether to try again.
 */
void *__cdecl Heap_Realloc(void *block, uint32_t size);
/*
 * Zeroed block of count * size bytes (32-bit product, rounded up to 16, 0 becomes 16): from 0x101069c0 when
 * up to the limit in dword 0x1015be5c (cleared here), else (or when that gives NULL) HeapAlloc(heap, 8, n).
 * Retries like Heap_Realloc while Sim_10106590 answers non-zero.
 */
void *__cdecl Heap_Calloc(uint32_t count, uint32_t size);

/* ---- w7r_fmt.c ---- */
/*
 * The three text layouts of a double (*value, two dwords). They use the digit record of Sim_10108f00: dword
 * +0 sign ('-' for negative), +4 decimal exponent, +0xc pointer to the digit text. While the byte 0x1017e908 is
 * set (0x10100ed0 / 0x10100f00 set it around their call) they work on the record and digits already left by
 * Fmt_DoubleG (record pointer in the dword 0x1017e904, exponent - 1 in the dword 0x1017e90c).
 */
/* d.ddde+XXX layout with ndig digits after the point; caps gives 'E'. Returns buf. */
char *__cdecl Fmt_DoubleE(const uint32_t *value, char *buf, int ndig, int caps);
/* ddd.ddd layout with ndig digits after the point. Returns buf. */
char *__cdecl Fmt_DoubleF(const uint32_t *value, char *buf, int ndig);
/*
 * Shortest layout for ndig significant digits: Sim_10100ed0 (the E layout) when the exponent is below -4
 * or not below ndig, else 0x10100f00 (the F layout). Records the record pointer and the exponent for those two,
 * and in the byte 0x1017e910 whether rounding raised the exponent; in that case the F layout gets the digit
 * text without its last digit.
 */
char *__cdecl Fmt_DoubleG(const uint32_t *value, char *buf, int ndig, int caps);

/* ---- w7r_math.c ---- */
/*
 * Error exit of the math functions: restores the x87 control word cw through 0x10103ad0(cw, 0xffff) and,
 * unless op is a known function (Sim_10103800 gives its name) and Sim_101091f0 handles it, sets the error
 * code with Sim_101037d0(type). Returns retval (in st(0)). arg1 and arg2 only go into the record the original
 * builds for Sim_101091f0, which does not look at it.
 */
double __cdecl Math_ReportError(int type, uint32_t op, double arg1, double arg2, double retval, uint32_t cw);
/*
 * Splits x into a mantissa with magnitude in [0.5, 1) (same sign, returned in st(0)) and a power of two
 * stored at *expo. 0 and NaN give +0.0 and 0; infinities give +-0.5 and 0x401; denormals are normalised.
 */
double __cdecl Math_FrExp(double x, int32_t *expo);

#endif
