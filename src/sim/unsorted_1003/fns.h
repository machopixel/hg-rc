/*
 * Small leaf functions of HGSIM.DLL (original 0x1003217c..0x1003dc18), plain C
 * with no knowledge of the hook layer. Every function keeps the original cdecl
 * calling convention. Names are the hex address when the code alone does not
 * say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1003_H
#define HG_SIM_UNSORTED_1003_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Bit set at 0x1014cb38 (starts as 0xb00) and the flag at 0x1014cb34 (starts as 1). */
void __cdecl Sim_1003327f(uint32_t mask);          /* xor the mask into the bit set */
int __cdecl Sim_10033291(uint32_t mask);           /* 1 when none of the mask bits is set */
void __cdecl Sim_100332a3(uint32_t mask, int on);  /* on != 0 CLEARS the mask bits, 0 sets them */
int __cdecl Sim_100332d2(void);                    /* 1 when the flag at 0x1014cb34 is zero */

/* Table of 0x44-byte entries at 0x109e4cc4, count at 0x109e4cc0. */
int __cdecl Sim_1003356a(const unsigned char *entry);
int __cdecl Sim_10033580(const unsigned char *entry);
int __cdecl Sim_1003359f(unsigned char *entry, const unsigned char *src);
int __cdecl Sim_100335cf(unsigned char *entry, const unsigned char *src);
unsigned char *__cdecl Sim_10033c42(int index);

/* Table of 0x7e-byte entries at 0x109b9e20, count at 0x1015ea80. */
void __cdecl Sim_1003acf3(const unsigned char *src);
uint32_t __cdecl Sim_1003b417(int index);
int __cdecl Sim_1003b485(uint32_t key);
void __cdecl Sim_1003b50f(int index, uint32_t value);

/* Tables behind the pointer at 0x1015d004, enabled by the word at 0x1015ec84. */
uint32_t __cdecl Sim_1003db56(int16_t key);
int __cdecl Sim_1003dbc0(uint32_t id);

/* Single-variable setters. */
void __cdecl Sim_1003217c(void);
int __cdecl Sim_10034855(int value);
void __cdecl Sim_1003a52b(uint32_t value);

/* Fills the 40-byte header at 0x109e38c0 with the layout of a Windows BITMAPINFOHEADER. */
void __cdecl Dib_InitHeader(uint32_t width, int height);

/* In-place byte-wise transform of a buffer, keyed by a 32-bit seed. */
void __cdecl Sim_1003c7b0(unsigned char *buf, int count, uint32_t seed);

#endif
