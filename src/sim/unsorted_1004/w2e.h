/*
 * Wave 2e, original 0x10041cc6..0x100487d0: small routines on name strings and
 * the string tables that are stored XOR-scrambled in the DLL's data.
 */
#ifndef HG_SIM_UNSORTED_1004_W2E_H
#define HG_SIM_UNSORTED_1004_W2E_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* When the first 0x18 bytes of the string s equal those of the string at 0x1015d160: copies them to dst+0xe (strncpy rules, 0x18 bytes) and stores the low 16 bits of id at dst+0. unused is ignored. */
void __cdecl Sim_10041cc6(uint32_t id, const char *s, uint32_t unused, void *dst);
/*
 * With src non-null: when the 0x20 bytes of the string at src+0x17 equal those of the string
 * at 0x1015db60, copies 0x5f bytes from src to dst and clears the dword 0x1015ec50. With src
 * null: sets that dword to 1 if it was 0 and clears the 0x5f bytes at dst.
 */
void __cdecl Sim_10041d01(const void *src, void *dst);
/* Calls Sim_10044255. */
void __cdecl Sim_10043f10(void);
/* Clears the 0x118 bytes at 0x1015ed70 and the dwords 0x1013aec4, 0x1015ed20, 0x1015ed38 and 0x1015ee98. */
void __cdecl Sim_10044255(void);
/*
 * 1 when the string s, with every character XOR 9, equals the last strlen(s) characters of
 * the string that ends at 0x109b94be (compared from the end backwards), else 0.
 */
int __cdecl Sim_10048762(const char *s);
/* XORs 0x0a into every character of the (up to) 32 strings whose pointers are at 0x1013b384 and 0x1013b388 in each of 16 records of 12 bytes. */
void __cdecl Sim_100487d0(void);

#endif
