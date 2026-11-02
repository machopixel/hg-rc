/* Native check of src/sim/util/unpack.c against the game's pkware.dll / zlib.dll (DECISIONS.md, "Windows calls: the
 * decompressors"): every packed entry of HEAVYG.PRJ through both (same answer, size and bytes), then damaged copies
 * (cut short, a changed byte, too little room) through ours alone: no crash and no write past the room. The DLLs are
 * not given damaged data: they answer success for data cut short or without enough room, and can crash on it
 * (ours answers a failure). The entries come from the scratch tool that lists HEAVYG.MAP / .PRJ: a folder of <n>.<method>.bin
 * files and list.txt ("file size packed name").
 * Build and run (32-bit mingw, from the repository root):
 *   i686-w64-mingw32-gcc -O2 -I src -I third_party/zlib -o unpack_native.exe tools/verify/unpack_native.c
 *       src/sim/util/unpack.c third_party/zlib/{adler32,crc32,inffast,inflate,inftrees,uncompr,zutil}.c
 *       third_party/zlib/contrib/blast/blast.c
 *   unpack_native.exe <entries folder> <folder with PKWARE.DLL and ZLIB.DLL>
 */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim/util/unpack.h"

typedef int(__cdecl *UncompressFn)(void *out, uint32_t *size, uint32_t packed, uint32_t packedSize);
typedef int (*OursFn)(uint8_t *out, uint32_t *size, const uint8_t *packed, uint32_t packedSize);

static unsigned g_seed = 1;
static unsigned rnd(void) { g_seed = g_seed * 1103515245u + 12345u; return g_seed >> 8; }

/* one call to each; returns 1 when they agree (both fail, or both succeed with the same size and bytes) */
static int same(UncompressFn dll, OursFn ours, const uint8_t *in, uint32_t inLen, uint32_t room, int *okOut)
{
    uint8_t *a = (uint8_t *)calloc(room + 64, 1), *b = (uint8_t *)calloc(room + 64, 1);
    uint32_t sa = room, sb = room;
    int ra, rb, agree;

    ra = dll(a, &sa, (uint32_t)(uintptr_t)in, inLen);
    rb = ours(b, &sb, in, inLen);
    agree = (ra != 0) == (rb != 0) && (ra != 0 || (sa == sb && memcmp(a, b, sa) == 0));
    if (okOut)
        *okOut = ra == 0;
    free(a);
    free(b);
    return agree;
}

/* ours alone on damaged data: 1 when nothing was written past the room (64 guard bytes after it) */
static int safe(OursFn ours, const uint8_t *in, uint32_t inLen, uint32_t room, int *okOut)
{
    uint8_t *b = (uint8_t *)malloc(room + 64);
    uint32_t sb = room, i;
    int clean = 1;

    memset(b + room, 0xa5, 64);
    *okOut = ours(b, &sb, in, inLen) == 0;
    for (i = 0; i < 64; i++)
        clean &= b[room + i] == 0xa5;
    clean &= !*okOut || sb <= room;
    free(b);
    return clean;
}

int main(int argc, char **argv)
{
    char path[1024], file[64], name[64];
    unsigned long size, packedSize;
    UncompressFn dll[3];
    OursFn ours[3] = {0, Unpack_Pkware, Unpack_Zlib};
    const char *dllName[3] = {0, "PKWARE.DLL", "ZLIB.DLL"};
    long n[3] = {0}, bad[3] = {0}, damaged[3] = {0}, damagedBad[3] = {0}, damagedOk[3] = {0};
    FILE *list, *f;
    int m, k, ok;

    if (argc < 3)
        return 2;
    for (m = 1; m <= 2; m++) {
        HMODULE h;
        snprintf(path, sizeof(path), "%s/%s", argv[2], dllName[m]);
        h = LoadLibraryA(path);
        dll[m] = h ? (UncompressFn)(void *)GetProcAddress(h, "Uncompress") : NULL;
        if (!dll[m]) {
            printf("cannot load %s\n", path);
            return 2;
        }
    }
    snprintf(path, sizeof(path), "%s/list.txt", argv[1]);
    list = fopen(path, "r");
    if (!list)
        return 2;
    while (fscanf(list, "%63s %lu %lu %63s", file, &size, &packedSize, name) == 4) {
        uint8_t *in = (uint8_t *)malloc(packedSize + 1);
        m = file[6] - '0';
        snprintf(path, sizeof(path), "%s/%s", argv[1], file);
        f = fopen(path, "rb");
        if (!f || fread(in, 1, packedSize, f) != packedSize || m < 1 || m > 2) {
            printf("cannot read %s\n", path);
            return 2;
        }
        fclose(f);
        n[m]++;
        if (!same(dll[m], ours[m], in, (uint32_t)packedSize, (uint32_t)size, &ok) || !ok) {
            if (bad[m]++ < 10)
                printf("DIFFERS: %s (%s, method %d, %lu -> %lu)%s\n", file, name, m, packedSize, size,
                       ok ? "" : " (the DLL fails it)");
        }
        /* damaged copies: every 4th entry, 3 kinds */
        if (n[m] % 4 == 0) {
            for (k = 0; k < 3; k++) {
                uint32_t len = (uint32_t)packedSize, room = (uint32_t)size;
                uint8_t keep = 0;
                uint32_t at = 0;
                if (k == 0)
                    len = len / 2;                                  /* cut short */
                else if (k == 1 && len > 2) {                       /* one changed byte */
                    at = 2 + rnd() % (len - 2);
                    keep = in[at];
                    in[at] ^= (uint8_t)(1 + rnd() % 255);
                } else if (k == 2 && room > 0)
                    room = room - 1;                                /* one byte too little room */
                damaged[m]++;
                if (!safe(ours[m], in, len, room, &ok)) {
                    if (damagedBad[m]++ < 10)
                        printf("damaged copy written past its room: %s kind %d\n", file, k);
                }
                if (ok)
                    damagedOk[m]++;
                if (k == 1 && len > 2)
                    in[at] = keep;
            }
        }
        free(in);
    }
    for (m = 1; m <= 2; m++)
        printf("%s: %ld entries, %ld differ; %ld damaged copies (%ld ours still accepts), %ld written past their room\n",
               dllName[m],
               n[m], bad[m], damaged[m], damagedOk[m], damagedBad[m]);
    return bad[1] || bad[2] || damagedBad[1] || damagedBad[2];
}
