/*
 * Round 6, letter i: C runtime code of the DLL around 0x100fd000..0x10100000 (sorting, copying, the heap
 * front end, streams, string compare, the exit sequence). Helpers that are not converted yet are called
 * through their original address with SIM_FN.
 */
#ifndef HG_SIM_UNSORTED_100F_W6I_H
#define HG_SIM_UNSORTED_100F_W6I_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

typedef int(__cdecl *W6iCompareFn)(const void *, const void *);
typedef void(__cdecl *W6iVoidFn)(void);

/* Not converted yet: the original helpers, called through their address. */
#define W6I_FN_100FE240 SIM_FN(int(__cdecl *)(void *), 0x100fe240)
#define W6I_FN_10103BA0 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, void *), 0x10103ba0)
#define W6I_FN_10105970 SIM_FN(int(__cdecl *)(int, int, int, int, int, int, int), 0x10105970)
#define W6I_FN_10105A60 SIM_FN(int(__cdecl *)(void *), 0x10105a60)
#define W6I_FN_100FEF50 SIM_FN(void *(__cdecl *)(uint32_t, uint32_t), 0x100fef50)
#define W6I_FN_101069C0 SIM_FN(void *(__cdecl *)(uint32_t), 0x101069c0)
#define W6I_FN_10106960 SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x10106960)
#define W6I_FN_100FF280 SIM_FN(uint32_t(__cdecl *)(uint32_t), 0x100ff280)
#define W6I_FN_101071E0 SIM_FN(int32_t *(__cdecl *)(void *), 0x101071e0)
#define W6I_FN_100FFE20 SIM_FN(void(__cdecl *)(uint32_t, char *, uint32_t, int), 0x100ffe20)

/*
 * Sorts the elements from lo to hi (both included, width bytes each) by repeatedly finding the largest
 * one (compare(p, largest) > 0; the first of equal ones wins) and swapping it to the end (Mem_Swap).
 * Pointer comparisons are unsigned.
 */
void __cdecl Sort_SelectionSort(uint8_t *lo, uint8_t *hi, uint32_t width, W6iCompareFn compare);
/* Copies n bytes from src to dst; overlapping areas are handled like memmove. Returns dst. */
void *__cdecl Mem_Copy(void *dst, const void *src, uint32_t n);
/*
 * Walks the stream pointer table (dword 0x10ac3860, count 0x10ac4ba0, both re-read every step) and calls
 * 0x100fe240 for each stream whose flags (+0xc) have a bit of 0x83 set. mode 1: returns how many calls
 * did not return -1. mode 0: only streams with flag 2; returns -1 if a call returned -1, else 0. Other
 * modes call nothing and return 0.
 */
int __cdecl Sim_100fe310(int mode);
/* Calls 0x10103ba0(a, b, address of the arguments after b) and returns its result. */
int __cdecl Sim_100fe450(uint32_t a, uint32_t b, ...);
/* FindClose(handle): 0 on success; otherwise sets dword 0x1017e888 to 0x16 and returns -1. */
int __cdecl File_FindClose(uint32_t handle);
/*
 * A FILETIME (two dwords, not both 0) -> FileTimeToLocalFileTime -> FileTimeToSystemTime -> returns
 * 0x10105970(year, month, day, hour, minute, second, -1). -1 when the time is 0 or a call fails.
 */
int __cdecl Time_FromFileTime(const uint32_t *fileTime);
/*
 * Reads characters from the stream f (dword +0 = read pointer, +4 = bytes left; 0x10105a60 refills)
 * into buf until a newline (kept), the end (-1) or n - 1 characters; adds a 0. Returns buf, or NULL when
 * n <= 0 or the end comes before the first character.
 */
char *__cdecl Stream_ReadLine(char *buf, int n, uint8_t *f);
/* Returns 0x100fef50(size, dword 0x1017e968). */
void *__cdecl Heap_Malloc(uint32_t size);
/*
 * Rounds size up to 16. Up to the limit in dword 0x1015be5c it asks the small block allocator
 * 0x101069c0(size / 16); if that gives NULL or the size is larger, HeapAlloc(heap 0x10ac3714, 0, size).
 */
void *__cdecl Heap_AllocBlock(uint32_t size);
/*
 * Frees block (nothing for NULL): a block of the small block heap (Sim_10106900 finds it) is handed to
 * 0x10106960(node, page, entry); any other goes to HeapFree(heap 0x10ac3714, 0, block).
 */
void __cdecl Heap_FreeBlock(uint32_t block);
/*
 * Case-insensitive compare of a and b: -1, 0 or 1. When dword 0x1017e950 is 0 only A-Z are folded;
 * otherwise each differing pair is folded with 0x100ff280 (a's character first). In that mode the
 * characters are put into the low byte of the previous 0x100ff280 results, and the value returned at the
 * end of the texts keeps those upper bits, like the original.
 */
int __cdecl Str_CompareNoCase(const char *a, const char *b);
/* Calls every non-NULL function pointer from begin up to (not including) end, in order. */
void __cdecl Crt_CallTable(W6iVoidFn *begin, W6iVoidFn *end);
/* Calls the function in dword 0x101593cc if set, then the tables 0x1012d208..0x1012d410 and 0x1012d000..0x1012d104. */
void __cdecl Sim_100ff380(void);
/*
 * The exit sequence. If dword 0x1017e8d0 is 1: TerminateProcess(GetCurrentProcess(), code). Sets dword
 * 0x1017e8cc to 1 and byte 0x1017e8c8 to the low byte of returnToCaller. Unless quick: calls the pointers
 * of the list from dword 0x10ac4bc8 (start, re-read after each call) to dword 0x10ac4bb8 (end) backwards,
 * then the table 0x1012d514..0x1012d724. Always the table 0x1012d828..0x1012d92c. If returnToCaller is 0:
 * sets dword 0x1017e8d0 to 1 and calls ExitProcess(code).
 */
void __cdecl Proc_DoExit(uint32_t code, int quick, int returnToCaller);
void __cdecl Proc_Exit(uint32_t code);      /* Proc_DoExit(code, 0, 0) */
void __cdecl Proc_ExitQuick(uint32_t code); /* Proc_DoExit(code, 1, 0) */
void __cdecl Sim_100ff3f0(void);            /* Proc_DoExit(0, 0, 1) */
void __cdecl Sim_100ff400(void);            /* Proc_DoExit(0, 1, 1) */
/* Time_FormatTm(0x101071e0(value)), or NULL when 0x101071e0 returns NULL. */
char *__cdecl Sim_100ff4e0(void *value);
/*
 * 0x100ffe20(value, buf, radix, negative) with negative = 1 only for radix 10 and value < 0 (the helper
 * writes the digits, with a '-' for negative). Returns buf.
 */
char *__cdecl Text_IntToRadix(int value, char *buf, int radix);

#endif
