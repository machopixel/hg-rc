#include "w8v.h"
#include "w5b.h"
#include "../util/access.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2k.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"

/* Loads the GDF resource `name` (any extension cut off) with 0x10059a34; returns its result or 0. */
// FUNCTION: HGSIM 0x100599a0
uint32_t __cdecl Sim_100599a0(const char *name)
{
    char buf[0x14];
    uint32_t size;
    uint32_t result = 0;
    char *dot;
    void *data;

    Str_Copy(buf, name);
    dot = Crt_StrChr(buf, '.');
    if (dot != 0)
        *dot = 0;
    data = W8V_CALL_100ba7d5(buf, SIM_PTR(char, 0x10144074), &size);
    if (data != 0) {
        result = W8V_CALL_10059a34(data, size);
        Sim_100bb2b0((const uint32_t *)buf, SIM_PTR(char, 0x10144078));
        W8V_CALL_100b9203(buf, SIM_PTR(char, 0x1014407c));
    }
    return result;
}

/* Index of the item p points to in items (0x438 bytes each), 0 for NULL. */
static uint32_t ItemIndex(uint32_t p, const uint8_t *items)
{
    return p != 0 ? (p - (uint32_t)(uintptr_t)items) / 0x438 : 0;
}

/*
 * Writes the file `name`: the 0x88-byte head, the head's +0x84 items (0x438 bytes each; their links at +0x54 and
 * +0x78 written as item indexes), the 0x3c-byte tail, tail +0x24 records of 0x6c bytes and, when size > 0, size
 * and the extra bytes. Returns fclose's result (0 when the file cannot be made).
 */
// FUNCTION: HGSIM 0x10059e5a
int __cdecl Sim_10059e5a(const char *name, const uint8_t *head, const uint8_t *items, const uint8_t *tail,
                         const void *data, int32_t size, const void *extra)
{
    const uint8_t *e;
    uint32_t index;
    int32_t i;
    void *f;

    f = Sim_100fd850(name, SIM_PTR(char, 0x10144080));
    if (f == 0)
        return 0;
    W8V_FWRITE5(head, 0x88, 1, f);
    for (i = 0; i < S32(head, 0x84); i++) {
        e = items + i * 0x438;
        W8V_FWRITE5(e, 4, 1, f);
        W8V_FWRITE5(e + 4, 4, 1, f);
        W8V_FWRITE5(e + 8, 4, 1, f);
        W8V_FWRITE5(e + 0xc, 4, 1, f);
        W8V_FWRITE5(e + 0x10, 4, 1, f);
        W8V_FWRITE5(e + 0x14, 0x20, 1, f);
        W8V_FWRITE5(e + 0x34, 0x20, 1, f);
        index = ItemIndex(U32(e, 0x54), items);
        W8V_FWRITE5(&index, 4, 1, f);
        W8V_FWRITE5(e + 0x58, 0x20, 1, f);
        index = ItemIndex(U32(e, 0x78), items);
        W8V_FWRITE5(&index, 4, 1, f);
        W8V_FWRITE5(e + 0x7c, 0x20, 6, f);
        W8V_FWRITE5(e + 0x13c, 0x2c, 1, f);
        W8V_FWRITE5(e + 0x168, 0x48, 0xa, f);
    }
    W8V_FWRITE5(tail, 0x3c, 1, f);
    W8V_FWRITE5(data, 0x6c, U32(tail, 0x24), f);
    if (size > 0) {
        W8V_FWRITE5(&size, 4, 1, f);
        W8V_FWRITE5(extra, (uint32_t)size, 1, f);
    }
    return W8V_FCLOSE5(f);
}

/*
 * Reads from the file written by Sim_10059e5a the name at tail +4 (into outName) and the dword at tail +0x2c.
 * Returns 1, or 0 when the file cannot be opened. The tail and head buffers are one block like the original's
 * frame (the name is copied with strcpy).
 */
// FUNCTION: HGSIM 0x1005a86f
int __cdecl Sim_1005a86f(const char *name, char *outName, uint32_t *outValue)
{
    uint8_t frame[0xc4];
    uint8_t *const tail = frame, *const head = frame + 0x3c;
    void *f;

    f = Sim_100fd850(name, SIM_PTR(char, 0x10144084));
    if (f == 0)
        return 0;
    W8V_FREAD5(head, 0x88, 1, f);
    W8V_FSEEK5(f, (int32_t)(U32(head, 0x84) * 0x438), 1);
    W8V_FREAD5(tail, 0x3c, 1, f);
    Str_Copy(outName, (const char *)tail + 4);
    *outValue = U32(tail, 0x2c);
    W8V_FCLOSE5(f);
    return 1;
}

/*
 * Writes rec to the file "dir\name": 0x12c bytes from +9, the +0x12d records of 0x48 bytes at +0x131 and 0x6c
 * bytes from +0x135. Returns fclose's result (0 when the file cannot be made).
 */
// FUNCTION: HGSIM 0x1005af72
int __cdecl Sim_1005af72(const uint8_t *rec, const char *dir, const char *name)
{
    char path[0x100];
    void *f;

    W8V_SPRINTF5(path, SIM_PTR(char, 0x101440a8), dir, name);
    f = Sim_100fd850(path, SIM_PTR(char, 0x101440b0));
    if (f == 0)
        return 0;
    W8V_FWRITE5(rec + 9, 0x12c, 1, f);
    if (U32(rec, 0x12d) != 0)
        W8V_FWRITE5(BYTES(U32(rec, 0x131)), 0x48, U32(rec, 0x12d), f);
    W8V_FWRITE5(rec + 0x135, 0x6c, 1, f);
    return W8V_FCLOSE5(f);
}

#define LOG_FILE G32(0x101668f8)

/* Opens hg.log (kept at 0x101668f8) and writes the start line with the date. Returns 1, or 0 when it fails. */
// FUNCTION: HGSIM 0x1005f8d0
int __cdecl Log_Open(void)
{
    char text[0x50];
    uint32_t now;
    void *f;

    f = Sim_100fd850(SIM_PTR(char, 0x101442f8), SIM_PTR(char, 0x101442f4));
    if (f == 0)
        return 0;
    LOG_FILE = (uint32_t)(uintptr_t)f;
    now = W8V_TIME5(0);
    W8V_SPRINTF5(text, SIM_PTR(char, 0x10144320), SIM_PTR(char, 0x10144300), Sim_100ff4e0(&now));
    Sim_1005f9aa(text);
    return 1;
}

/* Writes the end line with the date to hg.log and closes it. Returns 1, or 0 when it is not open. */
// FUNCTION: HGSIM 0x1005f949
int __cdecl Log_Close(void)
{
    char text[0x50];
    uint32_t now;

    if (LOG_FILE == 0)
        return 0;
    now = W8V_TIME5(0);
    W8V_SPRINTF5(text, SIM_PTR(char, 0x10144330), Sim_100ff4e0(&now));
    Sim_1005f9aa(text);
    W8V_FCLOSE5(BYTES(LOG_FILE));
    return 1;
}
