/* The game archive's two packing methods: see unpack.h. The decoding itself is zlib's and its contrib/blast
 * (third_party/zlib). */
#include <string.h>
#include "zlib.h"
#include "contrib/blast/blast.h"
#include "unpack.h"

int Unpack_Zlib(uint8_t *out, uint32_t *size, const uint8_t *packed, uint32_t packedSize)
{
    uLongf outLen = *size;

    if (uncompress(out, &outLen, packed, packedSize) != Z_OK)
        return -1;
    *size = (uint32_t)outLen;
    return 0;
}

/* ---- PKWARE Data Compression Library, through blast() -------------------------------------------------------- */

typedef struct {
    const uint8_t *in;
    uint32_t inLen;             /* 0 once handed over: blast() then fails for want of input */
    uint8_t *out;
    uint32_t outLen, outPos;
} Blast;

static unsigned blast_input(void *how, unsigned char **buf)
{
    Blast *b = (Blast *)how;
    unsigned n = b->inLen;

    *buf = (unsigned char *)b->in;
    b->inLen = 0;
    return n;
}

static int blast_output(void *how, unsigned char *buf, unsigned len)
{
    Blast *b = (Blast *)how;

    if (b->outLen - b->outPos < len)
        return 1;               /* more output than the room */
    memcpy(b->out + b->outPos, buf, len);
    b->outPos += len;
    return 0;
}

int Unpack_Pkware(uint8_t *out, uint32_t *size, const uint8_t *packed, uint32_t packedSize)
{
    Blast b;

    b.in = packed;
    b.inLen = packedSize;
    b.out = out;
    b.outLen = *size;
    b.outPos = 0;
    if (blast(blast_input, &b, blast_output, &b, NULL, NULL) != 0)
        return -1;
    *size = b.outPos;
    return 0;
}
