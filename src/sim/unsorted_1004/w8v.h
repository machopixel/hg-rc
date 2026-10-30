/*
 * Round 8, letter v: functions at 0x100470c7..0x1004e886: small record files (low-level _open/_write), copies
 * between plain files and scrambled record files, the screenshot TGA writer and parts of the GIF writer, the chat line editor,
 * an animation-frame step and the 16 x 64 brightness table. Helpers that are not converted yet (or that another
 * round-8 helper converts at the same time) are called through their original address with the prototypes
 * below.
 */
#ifndef HG_SIM_UNSORTED_1004_W8V_H
#define HG_SIM_UNSORTED_1004_W8V_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* C library low-level I/O: _write(handle, buffer, count) and _close(handle). */
#define W8V_WRITE SIM_FN(int(__cdecl *)(int, const void *, uint32_t), 0x10105750)
#define W8V_CLOSE SIM_FN(int(__cdecl *)(int), 0x101049c0)
/* Opens a record file (path, out size, out block, 0); returns a handle or -1. */
#define W8V_CALL_10046ac4 SIM_FN(int(__cdecl *)(const char *, uint32_t *, uint32_t *, int), 0x10046ac4)
/* C library streams: fwrite(buffer, size, count, stream), fread(...), fclose(stream). */
#define W8V_FWRITE SIM_FN(uint32_t(__cdecl *)(const void *, uint32_t, uint32_t, void *), 0x100febc0)
#define W8V_FREAD SIM_FN(uint32_t(__cdecl *)(void *, uint32_t, uint32_t, void *), 0x100fe830)
#define W8V_FCLOSE SIM_FN(int(__cdecl *)(void *), 0x100fd760)
/* C library sprintf(buffer, format, ...). */
#define W8V_SPRINTF SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* C library pow(x, y); result in st(0). */
#define W8V_POW SIM_FN(double(__cdecl *)(double, double), 0x100fed29)
/* Not converted: (a dword of 0x1014cf8c, an offset, a dword of 0x1014cec0, 0); returns a handle or 0. */
#define W8V_CALL_100282d4 SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t, uint32_t, int), 0x100282d4)
/* Not converted: the frame count of the handle. */
#define W8V_CALL_100ed8c3 SIM_FN(uint32_t(__cdecl *)(uint32_t), 0x100ed8c3)
/* Not converted: (first argument, handle, frame as a word, third argument, fourth argument). */
#define W8V_CALL_100e91dc SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100e91dc)

/* Windows HeapAlloc(heap, flags, size), called through the DLL's import slot 0x10ac5658. */
typedef void *(__stdcall *W8vHeapAllocFn)(uint32_t, uint32_t, uint32_t);

/* The low-level I/O open mode used for the record files: _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY. */
#define W8V_O_WRITE_NEW 0x8101u

/* w8v_files.c */
int __cdecl Sim_100470c7(const char *name, const void *data);
int __cdecl Sim_1004712a(const char *name, uint32_t *out);
int __cdecl Sim_1004724c(const char *name, const void *data);
int __cdecl FileRec_CopyFromFile(const char *name, const char *path);
int __cdecl FileRec_CopyToFile(const char *path, const char *name);

/* w8v_shots.c */
int __cdecl Screenshot_WriteTga(const char *name, int32_t width, int32_t height, int32_t stride, const uint8_t *pixels);
int __cdecl Gif_WriteImage(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t pixelFn);
int __cdecl Gif_CloseFile(void);

/* w8v_misc.c */
void __cdecl Sim_10048287(uint32_t a, int32_t index, uint32_t c, uint32_t d);
int __cdecl Chat_HandleKey(uint32_t key);
void __cdecl Gamma_BuildTable(void);

#endif
