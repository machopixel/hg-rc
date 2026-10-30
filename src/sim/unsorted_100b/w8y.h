/*
 * Round 8, letter y: functions of 0x100b45b4..0x100bcd61 (the debug unit boxes, the language text file, the
 * resource cache and its hash tables, the packed resource file). Helpers that are not converted yet (or that
 * another hook file patches, or that another round-8 helper works on) are called through their original address.
 */
#ifndef HG_SIM_UNSORTED_100B_W8Y_H
#define HG_SIM_UNSORTED_100B_W8Y_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Not converted (it reads four unset stack bytes): (key: word +0, name +2; ext, the 4-character file extension,
 * 2, &size, 0); loads a resource and returns its data (0 when there is none), sets the key word.
 */
#define W8Y_CALL_100b8d6c \
    SIM_FN(uint32_t(__cdecl *)(void *, uint32_t, uint32_t, int, uint32_t *, uint32_t), 0x100b8d6c)
/* Not converted (round 8, batch x): (object). */
#define W8Y_CALL_10089e91 SIM_FN(void(__cdecl *)(uint32_t), 0x10089e91)
/* Not converted: (english text pointer, translated text pointer, options, line number). */
#define W8Y_CALL_100b7430 SIM_FN(void(__cdecl *)(char **, char **, uint32_t, int), 0x100b7430)
/* Not converted: finishes a text block (text); returns a text. */
#define W8Y_CALL_100b6f6a SIM_FN(char *(__cdecl *)(char *), 0x100b6f6a)
/* Not converted (round 8, batch v): (new name, name); 0 when the file was made. */
#define W8Y_CALL_10048613 SIM_FN(int(__cdecl *)(const char *, const char *), 0x10048613)
/* Not converted (round 8, batches z and A): fclose(stream), remove(name). */
#define W8Y_CALL_100fd760 SIM_FN(int(__cdecl *)(void *), 0x100fd760)
#define W8Y_CALL_100fdb30 SIM_FN(int(__cdecl *)(const char *), 0x100fdb30)
/* Not converted: (address, 0xf0). */
#define W8Y_CALL_100eb070 SIM_FN(void(__cdecl *)(void *, int), 0x100eb070)
/*
 * Not converted (it reads unset stack words when an entry has no data block): sets up entry i of the table
 * 0x101a2100 again; returns the index of the entry it ended at, or -1.
 */
#define W8Y_CALL_100b43ce SIM_FN(int32_t(__cdecl *)(int32_t), 0x100b43ce)
/* Not converted: (entry index, entry's +0x10); non-zero when the entry could be made. */
#define W8Y_CALL_100b48de SIM_FN(int(__cdecl *)(int32_t, uint32_t), 0x100b48de)
/* Not converted (round 8, batch w): (the entry's object +0x28). */
#define W8Y_CALL_100897c7 SIM_FN(void(__cdecl *)(uint32_t), 0x100897c7)
/* Not converted: _assert(expression, file, line) of the DLL's C library. */
#define W8Y_CALL_100ff860 SIM_FN(void(__cdecl *)(const char *, const char *, int), 0x100ff860)
/* Not converted: makes room in the cache (0, type name). */
#define W8Y_CALL_100b9203 SIM_FN(void(__cdecl *)(int, const char *), 0x100b9203)
/* Not converted: non-zero when the processor can be asked for its vendor (fills 0x1019917c..0x10199194). */
#define W8Y_CALL_100b8f30 SIM_FN(int(__cdecl *)(void), 0x100b8f30)
/* Not converted (it reads an unset stack pointer when lang.ini has no "language ="): the language letter in al
   ('X' without the file); the rest of eax is passed on by Sim_100b9eab as it is. */
#define W8Y_CALL_100b9dde SIM_FN(uint32_t(__cdecl *)(void), 0x100b9dde)
/* Patched by hooks_mem.c (src/sim/mem): Str_TrimEnd, Str_CutAtBlank, Str_SkipBlanks, Cache_FindName. */
#define W8Y_CALL_100ba285 SIM_FN(void(__cdecl *)(char *), 0x100ba285)
#define W8Y_CALL_100ba21c SIM_FN(void(__cdecl *)(char *), 0x100ba21c)
#define W8Y_CALL_100ba252 SIM_FN(char *(__cdecl *)(char *), 0x100ba252)
#define W8Y_CALL_100ba1c9 SIM_FN(uint32_t(__cdecl *)(uint32_t, const char *), 0x100ba1c9)
/* Not converted (round 8, batch z): sprintf(buffer, format, ...) of the DLL's C library. */
#define W8Y_CALL_100fc780 SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* Not converted (round 8, batch A): fseek(stream, offset, origin), fread(buffer, size, count, stream). */
#define W8Y_CALL_100feb20 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, int), 0x100feb20)
#define W8Y_CALL_100fe970 SIM_FN(int32_t(__cdecl *)(uint32_t), 0x100fe970) /* ftell (round 25) */
#define W8Y_CALL_100fef30 SIM_FN(void *(__cdecl *)(uint32_t), 0x100fef30) /* malloc on [0x1017b8f8] (round 25) */
#define W8Y_CALL_100fe830 SIM_FN(uint32_t(__cdecl *)(void *, uint32_t, uint32_t, uint32_t), 0x100fe830)

/*
 * Entries of the table 0x101a2100 (0x88 bytes: +0x10 value, +0x14 parent index, +0x18 flags, +0x24 record index
 * into 0x109e9ba0 (0x50 bytes, flags word +0), +0x28 object, +0x2c node).
 * Marks the entry obj+8 (bit 0x200), sets it up again with 0x100b43ce when obj+8 is not -1; when that gives an
 * entry with flag 0x400 whose record has 0x400 and 0x8000, its node gets Sim_10028f38(node, 0x100035ee, 0) and
 * the entry is released (Sim_100b5ad7). Then Sim_100b469d.
 */
void __cdecl Sim_100b45b4(void *obj);
/*
 * For each of the [0x1017d218] entries with a parent (+0x14 > -1): when the parent has 0x200 and the entry not,
 * 0x1019a07c = 1 and 0x100b43ce(i). When the parent has 0x800: an entry without it gets it (0x1019a07c = 1) and
 * its object freed (Sim_100b534b). Otherwise an entry with 0x800 and without 0x200 loses 0x800 (0x1019a07c = 1)
 * and is made again (0x100b48de(i, +0x10)): when that works and its record (when it has one) has 0x8000, its node
 * gets Sim_10028f38(node, 0x100035ee, 0) and the entry is released (Sim_100b5ad7); when it fails its flags are
 * cleared. With 0x1017d228 set, an entry whose node's first dword is 0 gets 0x100897c7(object).
 */
void __cdecl Sim_100b469d(void);

/*
 * Toggles the debug unit boxes. Off (0x101a1164 clear): it is set, the resource "unitbox" is loaded
 * (0x100b8d6c with the extension pointers 0x1014cedc / 0x1014cf48) and for each of the [0x1017d220] entries of
 * 0x101a1180 (0x7c bytes: corner floats +0..+8 and +0xc..+0x14, position +0x18..+0x20, matrix +0x48) whose first
 * floats differ, a box scaled by (|d| + 10 - 1) / 10 per axis (Sim_100e2776, Sim_100e282a(0)) is made from the
 * data (Sim_100e2a31) into 0x101a10e0[i]; a box that comes out gets state 4 (Sim_1002d890), the matrix
 * (translation by the position, Sim_10060656) times the entry's matrix (Mat3x4_Multiply), Sim_10009c55 and
 * 0x10089e91, and the result becomes 1. The data is freed (HeapFree of 0x1017b8f8 for key -1, else
 * Sim_1002828e(key, extension)). On (set): cleared, every box of the table is freed (Sim_10009787) and cleared,
 * result 1. Returns the result.
 */
int __cdecl Sim_100b5502(void);
/*
 * Loads "unitbox" as Sim_100b5502 does and makes the boxes of the box tree from root (Sim_100b58c5), then frees
 * the data.
 */
void __cdecl Sim_100b581d(void *root);
/*
 * Box tree node: floats x0, x1, y0, y1, z0, z1 (+0..+0x14) and four children (+0x1c..+0x28). For a node: a box of
 * scale (|x1-x0| + 10 - 1) / 10, (|y1-y0| + 10 - 1) * 2 / 10, (|z1-z0| + 10 - 1) / 10 (Sim_100e2776,
 * Sim_100e282a(4)) is made from data (Sim_100e2a31 with the count limit size); when it comes out it gets state 4,
 * the translation to the centre ((a + b) / 2 per axis) and 0x10089e91. Then the four children.
 */
void __cdecl Sim_100b58c5(void *node, uint32_t data, int32_t size);

/*
 * Reads the language file: name (default "lang.txt"); a name without ".txt" is first made into "temp.txt"
 * (0x10048613, removed again at the end when it was made). The blocks of the file (Sim_100b6ba0) go into a new
 * table { int count; pairs *items } (Heap_Calloc(8, 1); items of 8 bytes: english, translated text; the room
 * grows from 100 by doubling, Heap_Realloc; when that fails both texts are freed and reading stops); the pairs are
 * sorted (Crt_QSort with the compare function 0x100b6b87) and the file is closed. Returns the table, 0 when the
 * file does not open or the table cannot be allocated.
 * The original leaves the two text pointers unset before the first block; they are only read when a
 * :TRANSLATED: line comes before any :ENGLISH: line. Here they start as 0.
 */
void *__cdecl Sim_100b69d0(const char *name);
/*
 * Reads lines (Stream_ReadLine, 0x100 bytes; each counts in *lines) up to a complete block: ":ENGLISH:" (9 bytes,
 * Sim_1010d240) reads the english text into *eng (Sim_100b6e5e); ":TRANSLATED:" (12) the translated one into *tr,
 * and when *eng is set: a missing translation becomes a copy of the english text (0 when that allocation fails),
 * 0x100b7430(eng, tr, *opt, *lines) when *opt is set, and 1 is returned; ":OPTIONS:" (9) reads a text, lowers its
 * capitals (Crt_IsCType(c, 1) when [0x1015965c] > 1, else the ctype table 0x10159450) and sets bits 1, 2, 4, 8 of
 * *opt for "elbonian", "stars", "linenum", "xtdelbon", then frees it. Returns 0 at the end of the file.
 */
int __cdecl Sim_100b6ba0(void *stream, char **eng, char **tr, uint32_t *opt, int *lines);
/*
 * Reads lines (0x100 bytes; each counts in *lines) and joins them with " " into a new text (Heap_Malloc, then
 * Heap_Realloc for each further line; 0 when an allocation fails) up to a line of only " \n\r\t\v\f" characters,
 * where 0x100b6f6a(text) is returned; at the end of the file the text is returned.
 */
char *__cdecl Sim_100b6e5e(void *stream, int *lines);

/*
 * Display shutdown: kills the timer event 0x1017d5cc (timeKillEvent, timeEndPeriod(0x10199448)) and frees the
 * blocks 0x1017d5d0 and 0x1017d5d4 (HeapFree, 0x1017d5d8 cleared too) when they are set. Then the driver's
 * function +0x20 of the table 0x1015e688 is asked (when 0x1017b920 is set); when it answers 0: 0x1017d5b8/bc take
 * 0x109e3e20/24, 0x100eb070(0x1017d5a0, 0xf0) and the function +0x1c of the table 0x1015e68c is called with the
 * rectangle (x, y, x + 0x27f, y + 0x1df) of 0x10199440 / 0x10199444.
 */
void __cdecl Sim_100b8938(void);

/*
 * Resource cache. A cache type (0x24 bytes): +0 hash size, +4 byte limit, +8 item count, +0xc bytes, +0x10 hash
 * table (dwords), +0x14 pointer array, +0x18 name (8), +0x20 free function. An item (0x24 bytes): +0 key (8 bytes),
 * +8 in use, +0xc size, +0x10/+0x14 time stamp, +0x18 data, +0x1c next, +0x20 previous in its hash chain; the
 * chain of key k starts at hash[(int)(key[0] ^ key[1]) % hash size]. The types are in the array 0x1017d604.
 */
/*
 * Writes the cache status log to the file name ("w"): per type its name, hash size, item count and bytes, the hash
 * chains of 5 or more items, the items in use and the free ones; then the bytes in use, free and in all.
 */
void __cdecl Sim_100b8ff9(const char *name);
/*
 * Drops the free items (in use <= 0, at most 0x400) of the cache type named type (upper-cased first): they are
 * unlinked, counted off and freed (Sim_100b9675 with the type's free function). The heap 0x1017b914 is chosen from
 * the name first (see pick_heap); at the end, when it is the "WTB" heap 0x1017b904, that heap is destroyed
 * (HeapDestroy) and cleared; 0x1017b914 is cleared. Returns 1 when the type exists, else 0.
 */
int __cdecl Sim_100b979f(char *type);
/*
 * Adds data (size bytes) under the 8-byte key to the cache type named type, making the type first when it is
 * missing (hash 0x400, limit 0x100000, sorted in with 0x100b9db0); room is made (0x100b9203) when the bytes would
 * reach the limit. The new item (in use 1) is put first in its chain, its key copied as a string (8), and it is
 * stamped with the 64-bit time of the function 0x1017d60c; returns the low word of that stamp.
 */
uint32_t __cdecl Sim_100b9b3f(uint32_t data, const uint32_t *key, const char *type, uint32_t size);
/*
 * Cache start: 0x1017d608 = 1 when 0x100b8f30 answers and the vendor text it leaves (copied to 0x101991a0) is
 * "GenuineIntel" (no case); the time function 0x1017d60c is 0x100034d1 then, else 0x10002b62. With a pack name the
 * pack is opened (Sim_100bcbcf(name, 0x100023c4, 0x100b9dde())) into 0x1017d5fc, else that is 0; without a pack
 * the directory list is read (Sim_100b9fa5). Then the cache types are read from cacheFile (Sim_100ba304).
 */
void __cdecl Sim_100b9eab(char *packName, const char *cacheFile);
/*
 * Reads the file "where": lines (not starting with '#') of the form type=dir;dir;... make a record per type
 * (name (8), +8 pointer array; in the array 0x1017d600 of 0x40) and add a copy of each directory to it.
 */
void __cdecl Sim_100b9fa5(void);
/*
 * Makes the array of cache types 0x1017d604 (0x20) and reads the types from the file name: lines (not starting
 * with '#') "type hashsize limit" (-1 for the defaults 0x400 and 0x100000); a line with a word missing frees the
 * type (also one that existed). New types are inserted with the compare function 0x100b9db0.
 */
void __cdecl Sim_100ba304(const char *name);
/*
 * Loads the resource name.ext (both upper-cased): from the cache (Sim_100baf3f) when it is there; otherwise the
 * language letter is read (0x100b9dde, kept in 0x1017d5f8), the file names are formatted (sprintf, the results are
 * not used) and, with a pack open (0x1017d5fc), it is read from the pack (Sim_100bca49) and added to the cache.
 * The size goes to *outSize when given. 0x1017b914 (chosen from ext first) is cleared. Returns the data or 0.
 * ext must be at most 6 characters (the original formats ext and a letter into 8 bytes of its stack).
 */
uint32_t __cdecl Sim_100baff4(char *name, char *ext, uint32_t *outSize);

/*
 * Round 25: loads the resource name.ext (both upper-cased): the heap [0x1017b914] chosen from ext (WTB [0x1017b904],
 * XEL, WAV, 3DI, MLT / TLT / GDF / BWD / SDF), the cached block when there is one (Sim_100baf3f); otherwise the file
 * "name.ext<language letter>" (0x100b9dde, kept in [0x1017d5f8]), else "name.ext", else the pack (Sim_100bca49) while
 * [0x1017d5fc] is set, else the search path (Sim_100bae0a). A file is read whole into a block of that heap (assert
 * at syscache.c lines 0x442 / 0x46c / 0x4bb when it cannot be had) and closed; the block is filed with Sim_100b9b3f.
 * The size goes to *outSize when given; [0x1017b914] is cleared. Returns the block or 0. ext must be at most 6
 * characters (the original formats ext and the letter into 8 bytes of its stack).
 */
/* Round 28: when set, called at the start of every Sim_100ba7d5 file load (the loading animation's tick). */
extern void(__cdecl *g_w8yLoadPoll)(void);
void *__cdecl Sim_100ba7d5(char *name, char *ext, uint32_t *outSize);

/*
 * Packed resource file 0x1017e1c0. Its directory (Sim_100bcd61): types (array 0x1017e1d0 of records { name (4),
 * pointer array of items }), groups 0x1017e1c8 (0x1017e1c4 records of 12 bytes: name (4), position, count),
 * unpacker names 0x1017dfb8 + 32 * i (0x1017e1bc of them, from 1) and items (12 bytes: name (8), position).
 * An item in the file: a 0x16-byte header (name (8), ext (4), method word +0xc, size +0xe, packed size +0x12)
 * and the packed bytes. Method 0 is stored; others are unpacked by the function 0x1017dfd4 + 32 * method.
 */
/*
 * For every group named type (4 characters, after upper-casing): reads its items one after the other
 * (Sim_100bc70d) and hands each to the function 0x1017e1cc (data, name, ext, size) when that is set. Returns 0.
 * The name buffer is 16 bytes, of which the item name fills at most 8 (the rest is left as it is, as in the
 * original).
 */
int __cdecl Sim_100bc610(char *type);
/*
 * Reads the item at pos of the pack: the header (fseek, fread), name (8) and ext (3 + 0) when asked for, *size and
 * *packed. Stored items are read into a new block (cache heap 0x1017b914 while allocating). Packed ones are read
 * into the buffer 0x1017b958 when it is set and the packed size is below 0x1017b95c (signed), else into a
 * temporary block, and unpacked into a new block of *size bytes; a failing unpacker gives 0 (block freed).
 * The heap selector 0x1017b8f8 is put back to 0x1017b900 as in the original. Returns the block.
 * A short header read uses the buffer's previous contents, as in the original (the buffer is a local).
 */
void *__cdecl Sim_100bc70d(uint32_t pos, uint32_t *size, uint32_t *packed, char *name, char *ext);
/*
 * Finds name in the pack: type list of ext plus the language letter 0x1017e1b8 ("%s%c" into 8 bytes, so ext must
 * be at most 6 characters), else of ext; the item by name (compare 0x100bcba7), retried in the list of ext.
 * Returns the item's data (Sim_100bc70d; the size into *outSize when given) or 0.
 */
uint32_t __cdecl Sim_100bca49(const char *name, const char *ext, uint32_t *outSize);
/*
 * Reads the directory of the pack file name ("rb", the whole file: _filelength, fread) into the tables described
 * above; returns 1, or 0 when the file does not open. The file is closed and the read block freed.
 */
int __cdecl Sim_100bcd61(const char *name);

#endif
