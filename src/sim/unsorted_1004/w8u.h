/*
 * Round 8, letter u: functions in 0x1004170e..0x10046fe5 (unit position packet, camera path and modes, data
 * files). Helpers that are not converted (or that another round-8 list converts) are called through their
 * original address with SIM_FN; prototypes below. Floating point follows the original x87 code with the game's
 * control word (53-bit precision): every intermediate is a double and only a store to a float rounds to float
 * (see util/w3o.h for the FCOM idioms); _ftol is Render_Ftol (render/w2n.h). The camera is the 0x40-byte record
 * that [0x1014cae0] points to (+0 x, +4 y, +8 z, +0xc..+0x14 angles, +0x3c field of view).
 */
#ifndef HG_SIM_UNSORTED_1004_W8U_H
#define HG_SIM_UNSORTED_1004_W8U_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* The C library of the DLL: fclose 0x100fd760, fseek 0x100feb20, fread 0x100fe830, _close 0x101049c0,
 * _write 0x10105750, _read 0x10105b50, fabs 0x100fcb70 (double in, st(0) out). */
#define W8U_FCLOSE_1004 SIM_FN(int(__cdecl *)(void *), 0x100fd760)
#define W8U_FSEEK_1004 SIM_FN(int(__cdecl *)(void *, int32_t, int), 0x100feb20)
#define W8U_FREAD_1004 SIM_FN(uint32_t(__cdecl *)(void *, uint32_t, uint32_t, void *), 0x100fe830)
#define W8U_CLOSE_1004 SIM_FN(int(__cdecl *)(uint32_t), 0x101049c0)
#define W8U_WRITE_1004 SIM_FN(int(__cdecl *)(uint32_t, const void *, uint32_t), 0x10105750)
#define W8U_READ_1004 SIM_FN(int(__cdecl *)(uint32_t, void *, uint32_t), 0x10105b50)
#define W8U_FABS_1004 hg_fabs
/* Not converted: table lookup (an angle as float bits); the result is in st(0). */
#define W8U_CALL_10084766 SIM_FN(double(__cdecl *)(uint32_t), 0x10084766)
/* Not converted: (unit a +0x28, unit b +0x28). */
#define W8U_CALL_1006788c SIM_FN(void(__cdecl *)(uint32_t, uint32_t), 0x1006788c)
/* Not converted: (a, b, c, d as float bits). */
#define W8U_CALL_10044d0b SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t), 0x10044d0b)
/* Not converted: no arguments. */
#define W8U_CALL_100456fc SIM_FN(void(__cdecl *)(void), 0x100456fc)
/* Not converted: (name, [0x1014cee4], [0x1014cf50], 3, &out, "ANFLOBJICID"); returns the loaded data or 0. */
#define W8U_CALL_100b8d6c SIM_FN(uint8_t *(__cdecl *)(const void *, uint32_t, uint32_t, int, uint32_t *, const char *), 0x100b8d6c)
/* HeapAlloc / HeapFree through the DLL's import slots. */
/*
 * Position packet of unit `index` (not the own one, [0x1017b8c8]): six ints at +2.. scaled by [0x10111528] into
 * the floats 0x1015dd58.. and the unit's +0x28 record +0x554..; a sound (Sim_1000ebfb "AFXHGCOO") whose volume
 * follows the distance, and 0x1006788c(own unit +0x28, unit +0x28).
 */
void __cdecl Sim_1004170e(const uint8_t *pkt, int index);

/* Camera path step: advances the key timer / index (0x1015ed38, 0x1015ed20) and sets the six camera values from
 * the six key interpolators (Sim_10047bcd) plus the offsets of Sim_10045308. 0 when no path runs. */
int __cdecl Sim_10043f1a(void);

/* Switches the camera mode (Sim_100448ff, or 2) and runs it for this frame. */
void __cdecl Sim_100445d6(void);

/* Camera mode 0 / 6: the key-driven turn rates, then the path (Sim_10043f1a) or Sim_10045308 on the camera. */
void __cdecl Sim_100451a2(void);

/* Camera mode 2 (and default): moves and turns the camera by (forward, turn, side, pitch, lift as float bits). */
void __cdecl Sim_1004548f(uint32_t f8, uint32_t fc, uint32_t f10, uint32_t f14, uint32_t f18);

/* Reads the whole file (mode "r") into buffer, or into a new HeapAlloc block when buffer is 0; 0 if it does not
 * open. */
void *__cdecl File_LoadWhole(const char *name, void *buffer);

/* Loads the "ANFLOBJICID" record table of name (0x100b8d6c) into the slots at 0x10a1d6c0 (base Sim_10019d8c);
 * 1 on success. The original keeps at most 32 ids in its frame. */
int __cdecl Sim_100463bf(const void *name);

/* Writes the 0x20-byte cache item 0xd, 0x300 bytes of data and [0x109e3de4] bytes of [0x109e3e20] to a new file;
 * 1 on success. */
int __cdecl Sim_100469f2(const char *name, const void *data);

/* Opens name (_open 0x8000), reads it whole into *buffer (HeapAlloc, or Heap_AllocRaw when alloc is set); returns
 * the open handle with *size set, or -1. */
int32_t __cdecl File_ReadAll(const char *name, uint32_t *size, uint32_t *buffer, const uint32_t *alloc);

/* File_ReadAll of the full name (Sim_100474db), closes it and sets *out to the buffer; returns the handle or -1. */
int32_t __cdecl Sim_10046ba1(const char *name, uint32_t *out);

/* Writes the kill statistics of rec to the text file name ("wt"); 0, or -1 when it does not open. */
int __cdecl Stats_WriteKills(const char *name, const uint8_t *rec);

/* Writes 0x1d4 bytes of data to the file (full name by Sim_100474db); 0, or -1 when it does not open. */
int __cdecl Sim_10046f7f(const char *name, const void *data);

/* *out = a zeroed 0x17-byte block, filled from the file when it reads, then reset by [0x1015ec58]; 1, or -1. */
int __cdecl Sim_10046fe5(const char *name, uint8_t **out);

#endif
