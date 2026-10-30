/*
 * Round 12: game functions of HGSIM.DLL around 0x10010000..0x1001ffff that read uninitialised stack in the original;
 * converted with the start values recorded in DECISIONS.md ("Round 12: uninitialised stack").
 */
#ifndef HG_SIM_UNSORTED_1001_W12_H
#define HG_SIM_UNSORTED_1001_W12_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Three-mode handler of a text "key;n,name,n". Mode 0: a 0x1e-byte record (HeapAlloc of [0x1017b8f8], flags 1) into
 * the slot Sim_100cc8ba(Sim_100cc6e0()), +4 = -1, +8 = 0, +0x18 = 1; after the ';' sscanf("%d,%[^,],%d") into +0
 * (then * 100), a name and +0x14; the word +0x1c = Sim_100b8d5c(); +0xc = Sim_100b399c(Sim_100b38c5(Sim_100db770(
 * atoi(text)))), 0 returned when the index is -1. Mode 1: with the slot's record (0 returned without one), Sim_1000f029
 * and 0 when +0xc or +0x14 is 0 or the dword at +0xc is 0, else +0x10 = Sim_100096ee(that dword) and Sim_1000ee23.
 * Mode 2: Sim_1000f029 of the slot's record when there is one. Returns 1 otherwise.
 */
int __cdecl Sim_1001a4fa(int mode, char *text);

/*
 * Two-mode handler of a text "key;a,b,c,d" for a 0x1c-byte motion record. Mode 0: the record (HeapAlloc of
 * [0x1017b8f8], flags 1) into the slot Sim_100cc8ba(Sim_100cc6e0()), +4..+0x10 = 0; after the ';' sscanf
 * ("%f,%f,%f,%f") gives +8, +0xc, +0x10 and +0x14 = d * 181.0f (180.0f when |that| < [0x101111ac]), +0x18 = time;
 * fields sscanf does not fill count as 0.0 (the original reads stack leftovers, DECISIONS.md); +0 = Sim_100b399c(
 * Sim_100b38c5(Sim_100db770(atoi(text)))), 0 returned when the index is -1. Mode 1: with the slot's record whose +0
 * points at a non-zero dword (0 returned otherwise), +4 = Sim_100096ee(that dword), t = (time - +0x18) / +0x14,
 * +0x18 = time, Sim_100aa127(+4, +8 * t, +0xc * t, +0x10 * t, 0) and Sim_100aa799(+4). Returns 1 otherwise.
 */
int __cdecl Sim_10019fe1(int mode, char *text, int32_t time);

/*
 * Two-mode handler of a text "key;a,b,n,n" for a 0x1c-byte turning record. Mode 0: the record (HeapAlloc of
 * [0x1017b8f8], flags 1) into the slot Sim_100cc8ba(Sim_100cc6e0()), +0x14 = +4 = 0; after the ';' sscanf
 * ("%f,%f,%d,%d"): +0xc = 360 / b, +0x10 = a * pi * 2 / b, +0x14 = the first n, +8 = 0; fields sscanf does not fill
 * count as 0 (the original reads stack leftovers, DECISIONS.md); +0x18 = time; +0 = Sim_100b399c(Sim_100b38c5(
 * Sim_100db770(atoi(text)))), 0 returned when the index is -1. Mode 1: with the slot's record (+0 pointing at a
 * non-zero dword, +0x14 non-zero; 0 returned otherwise), +4 = Sim_100096ee(that dword); with angle +8,
 * t = (time - +0x18) / 181, +0x18 = time, a = +0x10 * t: Sim_100a9d7c(+4, cos * a, 0, -sin * a) (Sim_10084766 of
 * angle + 90 and of angle) and Sim_100aa799(+4); b = +0xc * t: Sim_100aa127(+4, 0, b, 0, 0), Sim_100aa799(+4), and
 * +8 = fmod(angle + b, 360). Returns 1 otherwise.
 */
int __cdecl Sim_1001a23e(int mode, char *text, int32_t time);

#endif