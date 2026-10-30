#include "w8A.h"
#include "w7r.h"
#include "../util/access.h"

#define ADDR(p) ((uint32_t)(uintptr_t)(p))

typedef void(__stdcall *W8aRaiseExceptionFn)(uint32_t code, uint32_t flags, uint32_t nargs, const uint32_t *args);

// FUNCTION: HGSIM 0x101030b0
void __cdecl Math_RaiseFpError(uint32_t *rec, uint32_t *pcw, uint32_t flags, uint32_t opcode,
                               const uint32_t *arg1, uint32_t *result)
{
    u32a *r = (u32a *)rec;
    u32a *cw = (u32a *)pcw;
    uint32_t args[1];
    uint32_t code = ADDR(rec); /* what the original passes when no flag is set */
    uint32_t st;

    r[1] = 0;
    r[2] = 0;
    r[3] = 0;
    if (flags & 0x10) {
        code = 0xc000008fu;
        r[1] |= 1;
    }
    if (flags & 2) {
        code = 0xc0000093u;
        r[1] |= 2;
    }
    if (flags & 1) {
        code = 0xc0000091u;
        r[1] |= 4;
    }
    if (flags & 4) {
        code = 0xc000008eu;
        r[1] |= 8;
    }
    if (flags & 8) {
        code = 0xc0000090u;
        r[1] |= 0x10;
    }
    /* the masks that are clear in the control word become the enable bits of the record */
    r[2] = (r[2] & ~0x10u) | ((~*cw & 1u) << 4);
    r[2] = (r[2] & ~8u) | ((~*cw & 4u) << 1);
    r[2] = (r[2] & ~4u) | ((~*cw >> 1) & 4u);
    r[2] = (r[2] & ~2u) | ((~*cw >> 3) & 2u);
    r[2] = (r[2] & ~1u) | ((~*cw >> 5) & 1u);
    st = (uint32_t)W8A_FN_10103aa0();
    if (st & 1)
        r[3] |= 0x10;
    if (st & 4)
        r[3] |= 8;
    if (st & 8)
        r[3] |= 4;
    if (st & 0x10)
        r[3] |= 2;
    if (st & 0x20)
        r[3] |= 1;
    switch (*cw & 0xc00) {
    case 0:
        r[0] &= ~3u;
        break;
    case 0x400:
        r[0] = (r[0] & ~2u) | 1u;
        break;
    case 0x800:
        r[0] = (r[0] & ~1u) | 2u;
        break;
    default:
        r[0] |= 3;
        break;
    }
    switch (*cw & 0x300) {
    case 0:
        r[0] = (r[0] & ~0x14u) | 8u;
        break;
    case 0x200:
        r[0] = (r[0] & ~0x18u) | 4u;
        break;
    case 0x300:
        r[0] &= ~0x1cu;
        break;
    default:
        break;
    }
    r[0] = (r[0] & 0xfffe001fu) | ((opcode & 0xfffu) << 5);
    r[8] |= 1;
    r[8] = (r[8] & ~0x1cu) | 2u;
    r[4] = arg1[0];
    r[5] = arg1[1];
    r[0x14] |= 1;
    r[0x14] = (r[0x14] & ~0x1cu) | 2u;
    r[0x10] = result[0];
    r[0x11] = result[1];
    W8A_FN_10103ab0();
    args[0] = ADDR(rec);
    SIM_API(W8aRaiseExceptionFn, 0x10ac5668)(code, 0, 1, args);
    /* a handler may have changed the record: copy its masks, rounding and precision back */
    if (r[2] & 0x10)
        *cw &= ~1u;
    if (r[2] & 8)
        *cw &= ~4u;
    if (r[2] & 4)
        *cw &= ~8u;
    if (r[2] & 2)
        *cw &= ~0x10u;
    if (r[2] & 1)
        *cw &= ~0x20u;
    switch (r[0] & 3) {
    case 0:
        *cw &= ~0xc00u;
        break;
    case 1:
        *cw = (*cw & ~0x800u) | 0x400u;
        break;
    case 2:
        *cw = (*cw & ~0x400u) | 0x800u;
        break;
    default:
        *cw |= 0xc00u;
        break;
    }
    /* the original clears the rounding bits again here instead of the precision bits */
    switch ((r[0] >> 2) & 7) {
    case 0:
        *cw = (*cw & ~0xc00u) | 0x300u;
        break;
    case 1:
        *cw = (*cw & ~0xc00u) | 0x200u;
        break;
    case 2:
        *cw &= ~0xc00u;
        break;
    default:
        break;
    }
    result[0] = r[0x10];
    result[1] = r[0x11];
}

// FUNCTION: HGSIM 0x10102e30
double __cdecl Math_DomainError(uint32_t op, double x, uint32_t cw)
{
    if (G32(0x1015c018) == 0)
        return Math_ReportError(1, op, x, 0.0, x, cw);
    G32(0x1017e888) = 0x21;
    W8A_FN_10103ad0(cw, 0xffff);
    return x;
}
