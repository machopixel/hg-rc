#include <string.h>
#include "w7r.h"
#include "w2f.h"
#include "w6i.h"
#include "w6k.h"
#include "../util/access.h"

#define W7R_REUSE (*GBYTES(0x1017e908))     /* set by 0x10100ed0 / 0x10100f00 around their call */
#define W7R_RECORD G32(0x1017e904)          /* digit record of the last Fmt_DoubleG */
#define W7R_EXP GS32(0x1017e90c)            /* its decimal exponent - 1 */
#define W7R_POINT (*GBYTES(0x10159660))     /* decimal point character */

static int w7r_negative(const uint8_t *rec)
{
    return S32(rec, 0) == '-';
}

// FUNCTION: HGSIM 0x10100be0
char *__cdecl Fmt_DoubleE(const uint32_t *value, char *buf, int ndig, int caps)
{
    uint8_t *rec;
    char *p;
    char *q;
    uint32_t e;

    if (W7R_REUSE != 0) {
        rec = BYTES(W7R_RECORD);
        Str_MakeRoom(buf + w7r_negative(rec), ndig > 0);
    } else {
        rec = (uint8_t *)Sim_10108f00(value[0], value[1]);
        Sim_10108e60(buf + w7r_negative(rec) + (ndig > 0), ndig + 1, rec);
    }
    p = buf;
    if (w7r_negative(rec)) {
        *buf = '-';
        p = buf + 1;
    }
    if (ndig > 0) {
        p[0] = p[1];
        p++;
        *p = (char)W7R_POINT;
    }
    q = p + (W7R_REUSE == 0) + ndig;
    /* the 6-byte exponent template (dword and word) */
    U32(q, 0) = G32(0x1012bf70);
    U16(q, 4) = U16(GBYTES(0x1012bf74), 0);
    if (caps != 0)
        *q = 'E';
    q++;
    if (*(const char *)(uintptr_t)U32(rec, 0xc) == '0')
        return buf;
    e = U32(rec, 4) - 1u;
    if ((int32_t)e < 0) {
        e = 0u - e;
        *q = '-';
    }
    q++;
    if ((int32_t)e >= 100) {
        *q = (char)(*q + (char)((int32_t)e / 100));
        e = (uint32_t)((int32_t)e % 100);
    }
    q++;
    if ((int32_t)e >= 10) {
        *q = (char)(*q + (char)((int32_t)e / 10));
        e = (uint32_t)((int32_t)e % 10);
    }
    q[1] = (char)(q[1] + (char)e);
    return buf;
}

// FUNCTION: HGSIM 0x10100d20
char *__cdecl Fmt_DoubleF(const uint32_t *value, char *buf, int ndig)
{
    uint8_t *rec;
    char *p;
    int32_t decpt;

    if (W7R_REUSE != 0) {
        char *s;

        rec = BYTES(W7R_RECORD);
        s = buf + w7r_negative(rec);
        if (W7R_EXP == ndig) {
            s += W7R_EXP;
            s[0] = '0';
            s[1] = 0;
        }
    } else {
        rec = (uint8_t *)Sim_10108f00(value[0], value[1]);
        Sim_10108e60(buf + w7r_negative(rec), (int32_t)(U32(rec, 4) + (uint32_t)ndig), rec);
    }
    p = buf;
    if (w7r_negative(rec)) {
        *buf = '-';
        p = buf + 1;
    }
    decpt = S32(rec, 4);
    if (decpt > 0) {
        p += decpt;
    } else {
        Str_MakeRoom(p, 1);
        *p++ = '0';
    }
    if (ndig > 0) {
        Str_MakeRoom(p, 1);
        *p++ = (char)W7R_POINT;
        decpt = S32(rec, 4);
        if (decpt < 0) {
            int32_t zeros = (int32_t)(0u - (uint32_t)decpt);
            int32_t n = ndig;

            if (W7R_REUSE != 0 || ndig >= zeros)
                n = zeros;
            Str_MakeRoom(p, (uint32_t)n);
            memset(p, '0', (uint32_t)n);
        }
    }
    return buf;
}

// FUNCTION: HGSIM 0x10100e20
char *__cdecl Fmt_DoubleG(const uint32_t *value, char *buf, int ndig, int caps)
{
    uint8_t *rec = (uint8_t *)Sim_10108f00(value[0], value[1]);
    char *s;
    int32_t e;

    W7R_RECORD = (uint32_t)(uintptr_t)rec;
    W7R_EXP = (int32_t)(U32(rec, 4) - 1u);
    s = buf + w7r_negative(rec);
    Sim_10108e60(s, ndig, rec);
    e = (int32_t)(U32(BYTES(W7R_RECORD), 4) - 1u);
    *GBYTES(0x1017e910) = (uint8_t)(W7R_EXP < e);
    W7R_EXP = e;
    if (e < -4 || e >= ndig)
        return (char *)(uintptr_t)Sim_10100ed0((uint32_t)(uintptr_t)value, (uint32_t)(uintptr_t)buf, (uint32_t)ndig,
                                               (uint32_t)caps);
    if (*GBYTES(0x1017e910) != 0) {
        /* rounding added a digit: drop the last one */
        while (*s++ != 0)
            ;
        s[-2] = 0;
    }
    return W7R_FN_10100F00(value, buf, ndig);
}
