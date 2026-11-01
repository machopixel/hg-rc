/*
 * Round 3 functions of HGSIM.DLL in 0x10040000..0x10042200 and 0x10047aa0..0x1004be11 (letter p).
 *
 * The first group builds a network message in one of the buffers whose pointers are stored in the data
 * (0x1015ec58..0x1015ecd0) and sends it with Net_DpSend_* (unsorted_1007/w3p.h). Every message starts with a
 * 2-byte header copied from the data at 0x1013ad4c..0x1013ae24; the payload follows at +2. "pointer table"
 * is the table of record pointers at 0x109e8b40 (count at 0x1015dc64), "list" the record at [0x1015d004]
 * (player count at +0x12, key words at the pointer +0x16, 0x1e-byte records with a word id at +0x1a at the
 * pointer +0x126), "self" the dword at 0x1017b8c8.
 * Names carry the original address because the code alone does not say what the messages mean. Functions
 * that end with the send return its result (the original's eax); the others return nothing (the
 * original's eax is whatever it had left).
 */
#ifndef HG_SIM_UNSORTED_1004_W3P_H
#define HG_SIM_UNSORTED_1004_W3P_H

#include <stdint.h>

#include "../simmem.h"

/* Message 0x1013ad68: for table entry `index` with the block at entry+0x28, sends the bits of the 8 dwords at block+0xbf5+4*i that were non-zero (and clears them), when any. */
void __cdecl Sim_10040376(int index);
/* Message 0x1013ad6c, only for an entry in state 3 (+0x14): index, entry+0xc and self as bytes; entry+0xc becomes self. */
void __cdecl Sim_10040564(int index);
/* When msg[4] is self and table entry msg[2] is in state 3: Sim_10040721(entry, msg + 5), Sim_10040564(entry+4), entry+0x18 = 0. */
void __cdecl Sim_1004084a(const uint8_t *msg);
/* Message 0x1013ad70 (0x21 bytes: index, Sim_10040645(buf + 3, entry)) for an entry in state 3 whose +0xc is self; nothing for an index outside the table. */
void __cdecl Sim_100408bd(int index);
/* Message 0x1013ad74 with a byte and the two words of `words` (zeros when NULL). */
int __cdecl Sim_100409be(int kind, const uint16_t *words);
/* Message 0x1013ad78 with a byte, sent to the player named by word 0x1015ecd8. */
int __cdecl Sim_10040ab8(int kind);
/* Message 0x1013ade0: bytes +0x137 and +0x138 of the list record as words (also stored to dwords 0x1013aca4 and 0x1013aca0, sign-extended). */
int __cdecl Sim_10040eb3(void);
/*
 * Message 0x1013ade4 (a[4], word w, byte b, word c). With (int16) w == 0x4200 it goes to every list player in state 1 whose
 * Sim_10052be7 is 0 and that is not self, else to the owner (+0xc) of table entry (w & 0xff) when that entry is in state 3 and the owner is not negative.
 */
void __cdecl Sim_10040f5f(const uint8_t *a, int w, int b, int c);
/* Message 0x1013ade8: a byte and a dword. */
int __cdecl Sim_10041203(int kind, uint32_t value);
/* Message 0x1013adec: two bytes and a dword. */
int __cdecl Sim_100412cb(int kind, int sub, uint32_t value);
/* Message "KI" (0x1013adf0, 1.2: 12 bytes): for the first table entries (up to 10), the kills of player i by the local
   player (g_w34Kills); then Sim_10041444(). */
void __cdecl Sim_100413c7(void);
/*
 * Word table 0x1015d0f0 (8 words) is rebuilt from the 8 x 8 word table at 0x1016552c (row stride 16 bytes): for every pair (i, j)
 * of table entries it adds [j][i] to word j, or subtracts it when the dwords +8 of both entries are equal. If the word of self
 * changed, the function at dword 0x1015eb20 is called with (session, hi | word 0x1013acac, the new word sign-extended).
 */
void __cdecl Sim_10041444(void);
/* Stores the bytes msg[2 + i] into the kill table's column `column` (not once the game is over), then Sim_10041444()
   and Sim_1007b02d(). */
void __cdecl Sim_1004158c(const uint8_t *msg, int column);
/* Message 0x1013adf4 from three floats and three floats of the block of self's entry (+0x554..), each as trunc(x * 65536 +- 0.5), sent to the player of Sim_1003dbc0(id). */
int __cdecl Sim_100415eb(int id, float x, float y, float z);
/* Message 0x1013ae04 (header only). */
int __cdecl Sim_1004185b(void);
/* Message 0x1013ae08 (header only); with dword 0x1015ecd0 == 1 it sets word 0x1015ec7c to 1 and sends to everybody, else to word 0x1015ecd8. */
int __cdecl Sim_10041888(void);
/*
 * With dword 0x1015ecd0 == 1: marks byte 0x1015d090 + who, asks the function at dword 0x1015ead8 for a count and when every
 * list player (from the last) has its mark, calls Sim_10041888. Otherwise, when who is the id Sim_1003db56 finds for word 0x1015ecd8,
 * sets bit 0 of dword 0x1017b94c and word 0x1015ec7c to 1. `unused` is not read.
 */
void __cdecl Sim_100418e3(int unused, int who);
/* Message 0x1013ae0c (header only): like Sim_10041888 without the word store. */
int __cdecl Sim_10041997(void);
/* Message 0x1013ae10 (header only), to everybody. */
int __cdecl Sim_100419e9(void);
/* Message 0x1013ae14: dword 0x1015ec68 and one bit for each of the first 254 records of the 0x50-byte table at 0x109e9ba0 (word +0 & 4), 8 per byte. */
int __cdecl Sim_10041a47(void);
/* Message 0x1013ae18 (header only), once: while dword 0x1015ec74 is 0 it copies self to dword 0x101655ac, sends and sets the flag. */
void __cdecl Sim_10041c00(void);
/* Always 1. The first time dword 0x1015ec58 and the session dword 0x1015ecd4 are set, it sends message 0x1013ae24 (header only) once; if dword 0x1015eccc is 1 it becomes 2 and dwords 0x1015ecd0 and 0x1015dc84 are cleared. */
int __cdecl Sim_100420a0(void);
/*
 * Word 0x1013acb0 = `word`, word 0x1015ecd8 = word 0x1013acac, calls the function at dword 0x1015eae0 with (session, 0, 0x10001654, 0, 0)
 * (0x10001654 is a function of the DLL), then asks the one at 0x1015ead8: a result of 1 sets dword 0x1015eccc = 2 and clears
 * 0x1015ecd0, 0x1015dc84 and word 0x1015ec7c; else dwords 0x1015eccc/0x1015ecd0/0x1015dc84 become 1/1/0 when the words
 * 0x1013acac and 0x1015ecd8 are equal and 1/2/2 when not.
 */
void __cdecl Sim_100421b0(int word);

/* p[1] = a, p[2] = b, p[3] = trunc(d * double 0x101115d8), p[0] = dword 0x1015dc78; returns 1 when p[3] > 0. */
int __cdecl Sim_10047aa0(int32_t *p, int a, int b, double d);
/* Adds a step to p[0] (or sets it to 1) from two ratios c / d and a / b that are compared with the thresholds at 0x10126ec0..0x10126f00. */
void __cdecl Sim_1004bce5(int32_t *p, int a, int b, int c, int d);
/* A step (0, -2 .. -6 or -8) chosen by the ratio a / b against the thresholds at 0x10126ec0..0x10126ee8. */
int __cdecl Sim_1004be11(int a, int b);

#endif

/* Round 33, 1.2's 10-player rating (w3p_net.c) */
extern uint8_t g_w34Kills[10 * 0x14];
extern int16_t g_w34Rating[10];
extern uint32_t g_w34NetOver;
/* Network round, 1.2's 10-player tables (w34_netover.c), each indexed by player (unit +4 / the sender):
   1.0's 8-entry tables at the right, 1.2's 10-entry ones in brackets */
extern uint32_t g_net10Heard[10];       /* frame of the player's last message: 0x1015db00 (0x107cab60) */
extern uint32_t g_net10Db20[10];        /* 0x1015db20 (0x107caba0) */
extern uint32_t g_net10Left[10];        /* the player left ("SS"): 0x1015db40 (0x107cb440) */
extern int32_t g_net10SnStamp[10];      /* the "SN" message's stamp: 0x1015db80 (0x107cb4a0) */
extern uint8_t g_net10Mark[10];         /* 0x1015d1b0 (0x107cb4d0) */
extern uint8_t g_net10Ready[10];        /* 0x1015d090 (0x107cb430) */
extern uint8_t g_net10Row[10][0x16];    /* the last unit state received ("DA" / "DM"), from +5: 0x1015da40 (0x107caa80) */
extern uint8_t g_net10DiSent[10][0xd1]; /* the last "DI" message sent per player (0x107cac00, new in 1.2) */
extern uint8_t *g_net10DiBuf;           /* the "DI" message buffer (1.2 [0x10131504]) */
#define NET10_PLAYERS 10
/* the damage handover (w3p_net.c): 1.2's "DI" message, its part records packed and unpacked, its receive */
void __cdecl Sim12_100342cf(uint8_t *buf, const uint8_t *unit);
void __cdecl Sim12_10034342(uint8_t *unit, const uint8_t *buf);
void __cdecl Sim12_10033c90(int index);
void __cdecl Sim12_10033d7b(const uint8_t *msg);
/* 1.2's player update helpers (w9b_net.c): "TC" with the parts, the host's round-robin handover, a destroyed gear */
void __cdecl Sim12_100343b5(int index, uint8_t to);
void __cdecl Sim12_10036590(int index);
void __cdecl Sim12_100362d2(int index);
extern uint8_t g_w34AkBuf[0x100];
void __cdecl Sim12_100352b7(uint8_t killer, int unit);
void __cdecl Sim12_10035348(const uint8_t *msg);
