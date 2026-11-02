#include <string.h>
#include "w8v.h"
#include "w5c.h"
#include "../util/access.h"
#include "../unsorted_1010/w6k.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100a/w7o.h"
#include "../unsorted_100a/w7p.h"
#include "../crt/hg_winheap.h"

// FUNCTION: HGSIM 0x100470c7
int __cdecl Sim_100470c7(const char *name, const void *data)
{
    int h = (int)Sim_10109980((uint32_t)(uintptr_t)Sim_100474db(name), W8V_O_WRITE_NEW, 0x80);

    if (h == -1)
        return -1;
    W8V_WRITE(h, data, 0x17);
    W8V_CLOSE(h);
    return 0;
}

// FUNCTION: HGSIM 0x1004712a
int __cdecl Sim_1004712a(const char *name, uint32_t *out)
{
    uint32_t size, block;
    uint8_t rec[0x6c];
    uint32_t p;
    int h, i;

    h = W8V_CALL_10046ac4(Sim_100474db(name), &size, &block, 0);
    if (h != -1) {
        *out = block;
        if (G32(0x1015ec58) != 0)
            U32(*out, 0x20) = 1;
        W8V_CLOSE(h);
        return 1;
    }
    /* the default record: four 0x10000, 0xb, five 1, zeros, 8, the 9 bytes at 0x1013b2c0, zeros */
    for (i = 0; i < 4; i++)
        U32(rec, 4 * i) = 0x10000;
    U32(rec, 0x10) = 0xb;
    for (i = 0; i < 5; i++)
        U32(rec, 0x14 + 4 * i) = 1;
    for (i = 0; i < 7; i++)
        U32(rec, 0x28 + 4 * i) = 0;
    U32(rec, 0x44) = 8;
    U32(rec, 0x48) = G32(0x1013b2c0);
    U32(rec, 0x4c) = G32(0x1013b2c4);
    rec[0x50] = *GBYTES(0x1013b2c8);
    for (i = 0x51; i < 0x58; i++)
        rec[i] = 0;
    for (i = 0; i < 5; i++)
        U32(rec, 0x58 + 4 * i) = 0;
    p = (uint32_t)(uintptr_t)Hg_HeapAlloc(G32(0x1017b8f8), 1, 0x6c);
    *out = p;
    memcpy(BYTES(*out), rec, 0x6c);
    return -1;
}

// FUNCTION: HGSIM 0x1004724c
int __cdecl Sim_1004724c(const char *name, const void *data)
{
    int h = (int)Sim_10109980((uint32_t)(uintptr_t)Sim_100474db(name), W8V_O_WRITE_NEW, 0x80);

    if (h == -1)
        return -1;
    W8V_WRITE(h, data, 0x6c);
    W8V_CLOSE(h);
    return 0;
}

/* Copies the plain file `path` byte by byte into the scrambled record file `name`. */
// FUNCTION: HGSIM 0x10048560
int __cdecl FileRec_CopyFromFile(const char *name, const char *path)
{
    uint8_t c[4];
    void *rec;
    void *f;

    rec = FileRec_OpenWrite(name);
    if (rec == 0)
        return 1;
    f = Sim_100fd850(path, SIM_PTR(char, 0x1013b378));
    if (f == 0)
        return 2;
    while (W8V_FREAD(c, 1, 1, f) == 1) {
        if (FileRec_WriteScrambled(rec, c, 1) != 1)
            return 3;
    }
    if (W8V_FCLOSE(f) != 0)
        return 4;
    if (FileRec_Close(rec) != 0)
        return 5;
    return 0;
}

/* Copies the scrambled record file `name` byte by byte into the plain file `path`. */
// FUNCTION: HGSIM 0x10048613
int __cdecl FileRec_CopyToFile(const char *path, const char *name)
{
    uint8_t c[4];
    uint32_t *rec;
    void *f;

    rec = File_OpenRead(name);
    if (rec == 0)
        return 1;
    f = Sim_100fd850(path, SIM_PTR(char, 0x1013b37c));
    if (f == 0)
        return 2;
    while (Sim_100ae470(rec, c, 1) == 1) {
        if (W8V_FWRITE(c, 1, 1, f) != 1)
            return 3;
    }
    if (W8V_FCLOSE(f) != 0)
        return 4;
    if (FileRec_Close(rec) != 0)
        return 5;
    return 0;
}
