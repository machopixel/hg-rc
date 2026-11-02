/* Link targets for sim/util/unpack.h in the checker's flat test image (no C library there for zlib). No spec calls
 * them: the unpackers are checked natively against pkware.dll / zlib.dll by unpack_native.c. */
#include "../../src/sim/util/unpack.h"

int Unpack_Zlib(uint8_t *out, uint32_t *size, const uint8_t *packed, uint32_t packedSize)
{
    (void)out; (void)size; (void)packed; (void)packedSize;
    return -1;
}

int Unpack_Pkware(uint8_t *out, uint32_t *size, const uint8_t *packed, uint32_t packedSize)
{
    (void)out; (void)size; (void)packed; (void)packedSize;
    return -1;
}
