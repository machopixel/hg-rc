/*
 * Body of the interpreted functions (DECISIONS.md), compiled twice:
 *   w10_interp.c      W10I_K_* = the platform math library (asin, acos, sin, cos); the hooked version
 *   w10_interp_x87.c  W10I_K_* = x87 code as the original's C runtime runs it; the bit-exact reference (not hooked)
 * Everything but the four math kernels is shared, so the two differ only in the math.
 */
typedef union {
    float f;
    uint32_t u;
} W10iBits;

/* The original's C runtime (inside HGSIM) reports a domain error by storing EDOM (33) in its errno at 0x1017e888
 * and answers the x87 "indefinite" NaN (or the NaN it was given). The platform library would not touch that errno
 * and its NaN may carry another sign, so these wrappers keep the original's behaviour outside the domain and use the
 * kernel W10I_K_* inside it. */
#define W10I_ERRNO 0x1017e888u
#define W10I_EDOM 33u

static double W10i_Domain(double x)
{
    union {
        double d;
        uint64_t u;
    } v;

    G32(W10I_ERRNO) = W10I_EDOM;
    if (x != x)
        return x;
    v.u = 0xfff8000000000000ull;
    return v.d;
}

static double W10i_Asin(double x)
{
    if (!(x >= -1.0 && x <= 1.0))
        return W10i_Domain(x);
    return W10I_K_ASIN(x);
}

static double W10i_Acos(double x)
{
    if (!(x >= -1.0 && x <= 1.0))
        return W10i_Domain(x);
    return W10I_K_ACOS(x);
}

/* sin / cos: an infinite or NaN argument is the CRT's domain error */
static double W10i_Sin(double x)
{
    if (x - x != 0.0)
        return W10i_Domain(x);
    return W10I_K_SIN(x);
}

static double W10i_Cos(double x)
{
    if (x - x != 0.0)
        return W10i_Domain(x);
    return W10I_K_COS(x);
}

typedef void(__cdecl *W10iAssertFn)(const char *expr, const char *file, unsigned line);
#define W10I_ASSERT(cond, expr, file, line)                                                                  \
    do {                                                                                                     \
        if (!(cond))                                                                                         \
            SIM_FN(W10iAssertFn, 0x100ff860)(SIM_PTR(const char, expr), SIM_PTR(const char, file), (line)); \
    } while (0)

// FUNCTION: HGSIM 0x10060681
void __cdecl W10I_FN(Sim_10060681)(const float *m, float *pitch, float *heading, float *roll)
{
    float p, h, r;

    if ((double)m[5] > W10I_D(0x1012b518) || !((double)m[5] >= W10I_D(0x1012b520))) {
        double len = Sim_10084b57(m[2], m[8]);

        if (!(len > W10I_D(0x1012b528))) {
            /* straight up or down: no heading from the matrix */
            h = (U32(m, 0x14) & 0x80000000u) != 0 ? 90.0f : -90.0f;
            p = 0.0f;
            r = Sim_1008486e(m[1], m[0]);
            *heading = h;
            *pitch = p;
            *roll = r;
            return;
        }
        /* interpreted: the original multiplies the CRT acos's x87 result */
        p = (float)(W10i_Acos((double)(float)len) * W10I_D(0x1012b530));
        if ((U32(m, 0x14) & 0x80000000u) != 0)
            p = -p;
    } else {
        /* interpreted: the original multiplies the CRT asin's x87 result */
        p = -(float)(W10i_Asin((double)m[5]) * W10I_D(0x1012b530));
    }
    h = Sim_1008486e(m[2], m[8]);
    r = Sim_1008486e(m[3], m[4]);
    *heading = h;
    *pitch = p;
    *roll = r;
}

// FUNCTION: HGSIM 0x100d71b9
void __cdecl W10I_FN(Sim_100d71b9)(void *obj, uint8_t *node, float dx, float dy, float dz, int spread,
                                    const uint8_t *weapon, void *q)
{
    W10iBits h, p, x, y, z;

    h.f = Sim_1008486e(dx, dz);
    /* interpreted: the original multiplies the CRT asin's x87 result */
    p.f = -(float)(W10i_Asin((double)dy) * W10I_D(0x1012bd40));
    Sim_100aa0da(node, p.u, h.u, 0, 1);
    Sim_100d709d(obj, &x.u, &y.u, &z.u, q);
    if (spread != 0) {
        float s = (float)(Sim_1009c929(200) - 100);
        float ax = -dz, ay = 0.0f, az = dx;
        float ux, uy, uz;

        Sim_10084c95(&ax, &ay, &az);
        x.f = (float)((double)s * ax + (double)x.f);
        z.f = (float)((double)s * az + (double)z.f);
        ux = (float)((double)dy * az);
        uy = (float)((double)dz * ax + (double)az * dx);
        uz = -(float)((double)ax * dy);
        x.f = (float)((double)s * ux + (double)x.f);
        y.f = (float)((double)s * uy + (double)y.f);
        z.f = (float)((double)s * uz + (double)z.f);
    }
    Sim_100a9d47(node, x.u, y.u, z.u);
    if ((U32(weapon, 8) & 0x40) == 0) {
        float k = (float)((double)FL(BYTES(U32(node, 0x8c)), 0x50) * (double)*SIM_PTR(const float, 0x1012bcf4));

        dx = (float)((double)dx * k);
        dy = (float)((double)dy * k);
        dz = (float)((double)dz * k);
        Sim_100a9d7c(node, dx, dy, dz);
    }
}

// FUNCTION: HGSIM 0x1000cbaa
void __cdecl W10I_FN(Sim_1000cbaa)(void)
{
    uint8_t *kind = SIM_PTR(uint8_t, 0x10145508);
    uint8_t *row = SIM_PTR(uint8_t, 0x10a3a600);
    float g = *SIM_PTR(const float, 0x1013abc0);
    int k, j;

    for (k = 0; k < 0x2e; k++, kind += 0xd6) {
        double span;
        float step, reach, scale, range;

        if ((U32(kind, 0x86) & 0x40) == 0)
            continue;
        range = FL(kind, 0x4c);
        span = (double)FL(kind, 0x50) - (double)FL(kind, 0x4c);
        reach = (float)span;
        step = (float)(span / (double)*SIM_PTR(const float, 0x10111088));
        scale = (float)((double)*SIM_PTR(const float, 0x1011109c) / ((double)FL(kind, 0x3c) * (double)FL(kind, 0x3c)));
        g = (float)((double)S32(kind, 0x38) * (double)g);
        U32(row, 0) = (uint32_t)k;
        FL(row, 4) = (float)((double)*SIM_PTR(const float, 0x10111088) / (double)reach);
        for (j = 0; j < 20; j++) {
            double t;

            range = (float)((double)range + (double)step);
            FL(row, 8 + 4 * j) = range;
            t = (double)g * (double)range * (double)scale;
            if (t >= (double)*SIM_PTR(const float, 0x101110a0))
                U32(row, 0x58 + 4 * j) = 0x42340000u; /* 45.0: out of reach, the steepest useful angle */
            else /* interpreted: the original multiplies the CRT asin's x87 result */
                FL(row, 0x58 + 4 * j) = (float)(W10i_Asin((double)(float)t) * W10I_D(0x101110a8) *
                                                 (double)*SIM_PTR(const float, 0x10111078));
        }
        row += 0xa8;
        G32(0x1015d0a0)++;
    }
}

// FUNCTION: HGSIM 0x100608ef
void __cdecl W10I_FN(Sim_100608ef)(const float *a, const float *b, float t, float *out)
{
    double d, s0, s1;
    int i;

    W10I_ASSERT(a != 0, 0x10144d04, 0x10144ce4, 0x1f7);
    W10I_ASSERT(b != 0, 0x10144d28, 0x10144d08, 0x1f8);
    W10I_ASSERT(out != 0, 0x10144d4c, 0x10144d2c, 0x1f9);
    d = (double)a[3] * b[3] + (double)a[0] * b[0] + (double)a[1] * b[1] + (double)a[2] * b[2];
    if (W10I_D(0x1012b538) + d > W10I_D(0x1012b540)) {
        if (W10I_D(0x1012b538) - d > W10I_D(0x1012b540)) {
            /* interpreted: the original divides the CRT sin's x87 results (through the CRT acos) */
            double om = W10i_Acos(d);
            double so = W10i_Sin(om);

            s0 = W10i_Sin((W10I_D(0x1012b538) - (double)t) * om) / so;
            s1 = W10i_Sin((double)t * om) / so;
        } else {
            s0 = W10I_D(0x1012b538) - (double)t;
            s1 = (double)t;
        }
        for (i = 0; i < 4; i++)
            out[i] = (float)(s0 * a[i] + s1 * b[i]);
        return;
    }
    /* opposite orientations: blend towards a perpendicular one */
    out[0] = -a[1];
    out[1] = a[0];
    out[2] = -a[3];
    out[3] = a[2];
    s0 = W10i_Sin((W10I_D(0x1012b538) - (double)t) * W10I_D(0x1012b548));
    s1 = W10i_Sin((double)t * W10I_D(0x1012b548));
    for (i = 0; i < 3; i++)
        out[i] = (float)(s0 * a[i] + s1 * out[i]);
}

typedef void(__cdecl *W10iMatFn)(float *m);
typedef void(__cdecl *W10iMatFixFn)(float *m, int order);
typedef void(__cdecl *W10iMatCopyFn)(float *from, float *to);
typedef double(__cdecl *W10iFabsFn)(double x);
#define W10I_SMALL(v) (!(SIM_FN(W10iFabsFn, 0x100fcb70)((double)(v)) >= (double)*SIM_PTR(const float, 0x1012b510)))

// FUNCTION: HGSIM 0x10060220
void __cdecl W10I_FN(Sim_10060220)(float *m, float ax, float ay, float az, uint32_t tx, uint32_t ty, uint32_t tz,
                                    uint32_t flags)
{
    double rx = (double)ax * W10I_D(0x1012b508);
    double ry = (double)ay * W10I_D(0x1012b508);
    double rz = (double)az * W10I_D(0x1012b508);
    double cx, cy, cz, sx, sy, sz;

    /* interpreted throughout: the original stores or multiplies the CRT sin / cos's x87 results */
    if (W10I_SMALL(ax)) {
        if (W10I_SMALL(ay)) {
            SIM_FN(W10iMatFn, 0x1006018d)(m);
            if (!W10I_SMALL(az)) {
                m[4] = (float)W10i_Cos(rz);
                m[0] = m[4];
                m[3] = (float)W10i_Sin(rz);
                m[1] = -m[3];
            }
            goto tail;
        }
        if (W10I_SMALL(az)) {
            SIM_FN(W10iMatFn, 0x1006018d)(m);
            m[8] = (float)W10i_Cos(ry);
            m[0] = m[8];
            m[2] = (float)W10i_Sin(ry);
            m[6] = -m[2];
            goto tail;
        }
    } else if (W10I_SMALL(ay) && W10I_SMALL(az)) {
        SIM_FN(W10iMatFn, 0x1006018d)(m);
        m[8] = (float)W10i_Cos(rx);
        m[4] = m[8];
        m[7] = (float)W10i_Sin(rx);
        m[5] = -m[7];
        goto tail;
    }

    cx = W10i_Cos(rx);
    cy = W10i_Cos(ry);
    cz = W10i_Cos(rz);
    sx = W10i_Sin(rx);
    sy = W10i_Sin(ry);
    sz = W10i_Sin(rz);
    if ((flags & 4) != 0) {
        sx = -sx;
        sy = -sy;
        sz = -sz;
    }
    switch (flags & 3) {
    case 1:
        m[0] = (float)(cz * cy);
        m[3] = (float)(cx * sz + cz * sx * sy);
        m[6] = (float)(sz * sx - cz * cx * sy);
        m[2] = (float)sy;
        m[5] = (float)(-cy * sx);
        m[8] = (float)(cx * cy);
        SIM_FN(W10iMatFixFn, 0x1005fcdc)(m, 1);
        break;
    case 0:
        m[0] = (float)(cz * cy + sz * sx * sy);
        m[3] = (float)(cx * sz);
        m[6] = (float)(cy * sz * sx - cz * sy);
        m[2] = (float)(cx * sy);
        m[5] = (float)(-sx);
        m[8] = (float)(cx * cy);
        SIM_FN(W10iMatFixFn, 0x1005fcdc)(m, 1);
        break;
    case 2:
        m[0] = (float)(cz * cy);
        m[3] = (float)(sx * sy + cx * cy * sz);
        m[6] = (float)(cy * sz * sx - cx * sy);
        m[1] = (float)(-sz);
        m[4] = (float)(cz * cx);
        m[7] = (float)(cz * sx);
        SIM_FN(W10iMatFixFn, 0x1005fcdc)(m, 2);
        break;
    }
    if ((flags & 4) != 0)
        SIM_FN(W10iMatCopyFn, 0x1006007d)(m, m);
tail:
    U32(m, 0x24) = tx;
    U32(m, 0x28) = ty;
    U32(m, 0x2c) = tz;
}

/* ---- 0x1001a706 (round 14): a unit following a path ------------------------------------------------------------ */
typedef void *(__stdcall *W10iHeapAllocFn)(uint32_t heap, uint32_t flags, uint32_t bytes);

/* The path's start: the position and angle interpolators restarted (rate 0.3, angles wrapped at 360), the length
 * +0x10 summed over the points (+0x40 points of 0x1c bytes from +0x44, their time at +0x18), and the unit +0 found
 * by the key of the text (0 when there is none). */
static int W10i_PathStart(uint8_t *rec, const char *text)
{
    union {
        uint64_t u;
        double d;
    } rate;
    uint8_t *p = BYTES(U32(rec, 8)) + 0x44;
    int32_t sum = 0, i, key, at;

    rate.u = 0x3fd3333333333333ull;
    Sim_10047b76(rec + 0x1c, U32(p, 0), U32(p, 0), rate.d);
    Sim_10047b76(rec + 0x2c, U32(p, 4), U32(p, 4), rate.d);
    Sim_10047b76(rec + 0x3c, U32(p, 8), U32(p, 8), rate.d);
    Sim_10047c92(rec + 0x4c, 0, 0, rate.d, 0x43b40000u);
    Sim_10047c92(rec + 0x60, 0, 0, rate.d, 0x43b40000u);
    Sim_10047c92(rec + 0x74, 0, 0, rate.d, 0x43b40000u);
    for (i = 0; i < S32(U32(rec, 8), 0x40); i++)
        sum += S32(U32(rec, 8), 0x5c + i * 0x1c);
    S32(rec, 0x10) = sum;
    key = Sim_100db770(Crt_AToI(text));
    key = Sim_100db770(Crt_AToI(text));
    at = Sim_100b38c5(key);
    if (at == -1)
        return 0;
    U32(rec, 0) = (uint32_t)(uintptr_t)Sim_100b399c(at);
    return 1;
}

/* Wraps the turn d = angle - previous into -180..180 (by 360) and keeps the previous angle as angle - d. */
static void W10i_PathTurn(uint8_t *rec, uint32_t off, float angle)
{
    float d = (float)((double)angle - (double)FL(rec, off + 4));

    while (d > GFL(0x101111cc))
        d = (float)((double)d - (double)GFL(0x101111d0));
    while (!(d >= GFL(0x101111d4)))
        d = (float)((double)d + (double)GFL(0x101111d0));
    FL(rec, off) = angle;
    FL(rec, off + 4) = (float)((double)angle - (double)d);
}

// FUNCTION: HGSIM 0x1001a706
int __cdecl W10I_FN(Sim_1001a706)(int mode, char *text, int32_t time)
{
    uint8_t *rec, *path, *cur, *nxt;
    int32_t n, t, acc, prev, i;
    float f, h, a, b, c;
    W10iBits x, y, z;

    if (mode == -1) {
        rec = BYTES(*(uint32_t *)(uintptr_t)Sim_100cc8ba((int)Sim_100cc6e0()));
        if (rec == 0)
            return 0;
        S32(rec, 0xc) = time;
        if (U32(rec, 8) != 0 && !W10i_PathStart(rec, text))
            return 0;
        return 1;
    }
    if (mode == 0) {
        /* the fields are three non-empty words of at most 8 characters in every shipped world file (DECISIONS.md,
         * round 14), so sscanf always fills the three buffers */
        char how[64], turn[64], name[64];
        char *s;

        rec = (uint8_t *)SIM_API(W10iHeapAllocFn, 0x10ac5658)(G32(0x1017b8f8), 1, 0x88);
        if (rec == 0)
            return 0;
        *(uint32_t *)(uintptr_t)Sim_100cc8ba((int)Sim_100cc6e0()) = (uint32_t)(uintptr_t)rec;
        U32(rec, 4) = 0;
        U32(rec, 8) = 0;
        S32(rec, 0xc) = time;
        s = Crt_StrChr(text, ';');
        if (s != 0) {
            *s++ = 0;
            Sim_100fd710(s, SIM_PTR(const char, 0x10131780), how, turn, name);
            if (Str_CompareNoCase(how, SIM_PTR(const char, 0x10131794)) == 0)
                U32(rec, 0x18) = 0;
            else if (Str_CompareNoCase(how, SIM_PTR(const char, 0x1013179c)) == 0)
                U32(rec, 0x18) = 1;
            else
                U32(rec, 0x18) = 2;
            U32(rec, 0x14) = Str_CompareNoCase(turn, SIM_PTR(const char, 0x101317a4)) == 0;
            for (i = 0; i < GS32(0x1015d17c); i++) {
                if (Str_CompareNoCase(name, SIM_PTR(const char, 0x10a00040 + i * 0x744)) == 0) {
                    U32(rec, 8) = 0x10a00040u + (uint32_t)i * 0x744u;
                    break;
                }
            }
        }
        if (U32(rec, 8) != 0 && !W10i_PathStart(rec, text))
            return 0;
        return 1;
    }
    if (mode != 1)
        return 1;
    rec = BYTES(*(uint32_t *)(uintptr_t)Sim_100cc8ba((int)Sim_100cc6e0()));
    if (rec == 0 || U32(rec, 0) == 0 || U32(rec, 8) == 0 || U32(U32(rec, 0), 0) == 0)
        return 0;
    U32(rec, 4) = Sim_100096ee((void *)(uintptr_t)U32(U32(rec, 0), 0));
    if (U32(rec, 4) == 0)
        return 0;
    path = BYTES(U32(rec, 8));
    n = S32(path, 0x40);
    if (n <= 1)
        return 0;
    t = (int32_t)((uint32_t)time - U32(rec, 0xc));
    acc = 0;
    prev = 0;
    for (i = 0; i < n; i++) {
        prev = acc;
        acc = (int32_t)((uint32_t)acc + U32(path, 0x5c + i * 0x1c));
        if (!(t > acc))
            break;
    }
    if (i >= n - 1) {
        if (S32(rec, 0x18) == 2)
            return 0;
        if (S32(rec, 0x18) == 1) {
            union {
                uint64_t u;
                double d;
            } rate;

            rate.u = 0x3fd3333333333333ull;
            i = 0;
            t = 0;
            S32(rec, 0xc) = time;
            prev = 0;
            Sim_10047b76(rec + 0x1c, U32(path, 0x44), U32(path, 0x44), rate.d);
            Sim_10047b76(rec + 0x2c, U32(path, 0x48), U32(path, 0x48), rate.d);
            Sim_10047b76(rec + 0x3c, U32(path, 0x4c), U32(path, 0x4c), rate.d);
            Sim_10047c92(rec + 0x4c, 0, 0, rate.d, 0x43b40000u);
            Sim_10047c92(rec + 0x60, 0, 0, rate.d, 0x43b40000u);
            Sim_10047c92(rec + 0x74, 0, 0, rate.d, 0x43b40000u);
        } else if (i == n) {
            i = 0;
            t = (int32_t)((uint32_t)t - U32(rec, 0x10));
            S32(rec, 0xc) = (int32_t)((uint32_t)time - (uint32_t)t);
            prev = 0;
        }
    }
    cur = path + 0x44 + i * 0x1c;
    nxt = i == n - 1 ? path + 0x44 : path + 0x44 + (i + 1) * 0x1c;
    f = (float)((double)(int32_t)((uint32_t)t - (uint32_t)prev) / (double)S32(cur, 0x18));
    FL(rec, 0x20) = (float)(((double)FL(nxt, 0) - (double)FL(cur, 0)) * (double)f + (double)FL(cur, 0));
    FL(rec, 0x30) = (float)(((double)FL(nxt, 4) - (double)FL(cur, 4)) * (double)f + (double)FL(cur, 4));
    FL(rec, 0x40) = (float)(((double)FL(nxt, 8) - (double)FL(cur, 8)) * (double)f + (double)FL(cur, 8));
    z.f = Sim_10047bcd((uint32_t *)(rec + 0x3c));
    y.f = Sim_10047bcd((uint32_t *)(rec + 0x2c));
    x.f = Sim_10047bcd((uint32_t *)(rec + 0x1c));
    Sim_100a9d47((void *)(uintptr_t)U32(rec, 4), x.u, y.u, z.u);
    h = FL(cur, 0x10);
    if (U32(rec, 0x14) != 0)
        h = (float)((double)Sim_1008486e((float)((double)FL(nxt, 0) - (double)FL(cur, 0)),
                                         (float)((double)FL(nxt, 8) - (double)FL(cur, 8))) +
                    (double)h);
    W10i_PathTurn(rec, 0x64, h);
    h = FL(cur, 0xc);
    if (U32(rec, 0x14) != 0)
        h = (float)((double)h - W10i_Asin(((double)FL(nxt, 4) - (double)FL(cur, 4)) / (double)GFL(0x101111d8)) *
                                    W10I_D(0x101111e0));
    W10i_PathTurn(rec, 0x50, h);
    W10i_PathTurn(rec, 0x78, FL(cur, 0x14));
    a = Sim_10047cf2((uint32_t *)(rec + 0x74));
    b = Sim_10047cf2((uint32_t *)(rec + 0x60));
    c = Sim_10047cf2((uint32_t *)(rec + 0x4c));
    x.f = c;
    y.f = b;
    z.f = a;
    Sim_100aa0da((void *)(uintptr_t)U32(rec, 4), x.u, y.u, z.u, 0);
    Sim_100aa799((void *)(uintptr_t)U32(rec, 4));
    return 1;
}
