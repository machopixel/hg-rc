/*
 * Round 7, letter m: functions at 0x10040191..0x1004ee50: the GIF file writer (header, LZW image data), loaders of
 * small resource records, a network packet handler and a few float helpers. Helpers that are not converted yet
 * are called through their original address (SIM_FN) with the prototypes below; Windows functions through the
 * DLL's import slots (SIM_API).
 */
#ifndef HG_SIM_UNSORTED_1004_W7M_H
#define HG_SIM_UNSORTED_1004_W7M_H

#include <stdint.h>
#include "../simmem.h"
#include "../util/access.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* Not converted: the C library buffer flush of a stream (character, stream); returns the character or -1. */
#define W7M_CALL_100ff030 SIM_FN(int(__cdecl *)(uint32_t, void *), 0x100ff030)
/* Not converted: (object id as a signed word, object, 2, 0). */
#define W7M_CALL_1006ad98 SIM_FN(void(__cdecl *)(int, void *, int, int), 0x1006ad98)
/* Not converted: finds a resource record (key, table name, a dword, kind, out dword, 0); returns its data or NULL. */
#define W7M_CALL_100b8d6c SIM_FN(void *(__cdecl *)(const void *, uint32_t, uint32_t, int, uint32_t *, int), 0x100b8d6c)
/* Not converted: the C library fmod (x, y) on doubles; result in st(0). */
#define W7M_FMOD SIM_FN(double(__cdecl *)(double, double), 0x100fcb30)
/* Not converted: (record, a word, a byte as word, a word); the three small values are pushed as 16-bit values. */
#define W7M_CALL_10015d30 SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t), 0x10015d30)
/* Not converted: (object id as a signed word, object record, index). */
#define W7M_CALL_1006abf6 SIM_FN(void(__cdecl *)(int, uint32_t, int), 0x1006abf6)

#ifndef HG_SIM_W7M_FLOAT_ACCESS
#define HG_SIM_W7M_FLOAT_ACCESS
/* A float of the original's data at a byte offset (may alias anything, like the U32 family in util/access.h). */
typedef float HG_MAY_ALIAS w7m_f32;
#define W7M_F(p, off) (*(w7m_f32 *)(BYTES(p) + (off)))
#define W7M_GF(addr) (*SIM_PTR(w7m_f32, addr))
static inline float W7m_Float(uint32_t bits)
{
    union {
        uint32_t u;
        float f;
    } v;
    v.u = bits;
    return v.f;
}
static inline uint32_t W7m_Bits(float f)
{
    union {
        float f;
        uint32_t u;
    } v;
    v.f = f;
    return v.u;
}
#endif

/* The pixel source of Lzw_Compress: returns the next pixel value, or -1 at the end. */
typedef int(__cdecl *W7mPixelFn)(void);

/*
 * Writes the low 16 bits of value to the stream at [0x1015f344], low byte first, like two putc calls (the C
 * library _flsbuf 0x100ff030 when the buffer is full). Returns 2 when the stream reports an error, else 0.
 */
int __cdecl Stream_PutWord(uint32_t value);
/*
 * Starts a GIF file: [0x1015f34c] = the colour count rounded up to a power of two (0 for no colours),
 * [0x1015f22c] = bits, [0x1015f21c] = height, [0x1015f218] = width; opens the file (Sim_1004db2e), writes
 * "GIF87a" and the screen descriptor (width, height, flags, 0, 0); frees [0x1015f360] and allocates 3 bytes per
 * colour there, zeroed. Returns 0, 1 (open failed), 2 (write failed) or 3 (no memory).
 */
int __cdecl Gif_Begin(const char *name, uint32_t width, uint32_t height, uint32_t colors, int bits);
/*
 * Writes the packed record rec: the byte at +0, the low words of the dwords at +1, +5, +9 and +0xd, and the byte
 * at +0x11. Returns 2 when a write fails, else 0.
 */
int __cdecl Sim_1004e719(const uint8_t *rec);
/*
 * LZW-compresses the pixels that next() returns (until -1) with the starting code size bits + 1 and a 12-bit
 * limit, through the code writer Sim_1004e24a and the string table at [0x1015f35c]; returns 0, or the error of
 * Sim_1004e441 when the tables cannot be allocated.
 */
int __cdecl Lzw_Compress(int bits, W7mPixelFn next);

/* Resource loaders: find the record named by key (a signed word -1 means "free it again with HeapFree"). */
int __cdecl Sim_100462c0(const int16_t *key, float *a, float *b, float *c, float *d, float *e, float *f, float *g);
int __cdecl Sim_100466b6(const int16_t *key);
int __cdecl Sim_100468c3(const int16_t *key, uint32_t *rects, uint32_t *pair);

/*
 * Sets the kind flags (0x1000..0x5000) of obj+0x90 from its type obj[0] and name; returns the flag set or 0 (or,
 * when obj+0x28 is NULL, obj itself: the original returns with the pointer still in eax).
 */
uint32_t __cdecl Sim_1004ee50(uint32_t *obj);
/* Moves the angle at obj+8 towards obj+4 (period obj+0x10, rate obj+0xc, time obj+0); returns it (st(0)). */
float __cdecl Sim_10047cf2(uint32_t *obj);
void __cdecl Sim_10045308(float *a, float *b, uint32_t *c, uint32_t *d, uint32_t *e, uint32_t *f);
void __cdecl Sim_1004409f(int index);
void __cdecl Sim_100410ff(const uint8_t *packet);
void __cdecl Sim_10040191(const uint8_t *packet);

#endif
