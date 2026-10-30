#include "w6j.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x101019b0
void __cdecl Sim_101019b0(int ch, int n, void *stream, int *count)
{
    while (n-- > 0) {
        W6J_PUTCHAR(ch, stream, count);
        if (*count == -1)
            break;
    }
}

// FUNCTION: HGSIM 0x101019f0
void __cdecl Sim_101019f0(const char *s, int n, void *stream, int *count)
{
    while (n-- > 0) {
        W6J_PUTCHAR((signed char)*s++, stream, count);
        if (*count == -1)
            break;
    }
}

// FUNCTION: HGSIM 0x10101a30
uint32_t __cdecl Sim_10101a30(uint32_t *cursor)
{
    uint32_t next = *cursor + 4u;

    *cursor = next;
    return U32(next - 4u, 0);
}

// FUNCTION: HGSIM 0x10101a50
uint64_t __cdecl Sim_10101a50(uint32_t *cursor)
{
    uint32_t next = *cursor + 8u;

    *cursor = next;
    return (uint64_t)U32(next - 8u, 0) | ((uint64_t)U32(next - 4u, 0) << 32);
}
