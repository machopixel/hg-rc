/*
 * Measures the interpreted functions (DECISIONS.md) natively: the platform-math versions (src/sim/util/w10_interp.c)
 * against the bit-exact x87 references (src/sim/util/w10_interp_x87.c, proven equal to the original by
 * tools/verify/spec_w10_<address>_x87.py), C against C over many random inputs. The platform math is the Windows
 * UCRT (ucrtbase.dll), the library the Windows build links.
 *
 * Build and run (32-bit mingw, from the repository root):
 *   i686-w64-mingw32-gcc -std=gnu17 -O1 -msse2 -mfpmath=sse -I src -I src/sim/mem -I src/sim/core
 *       tools/verify/interp_measure.c src/sim/util/w10_interp.c src/sim/util/w10_interp_x87.c -o interp_measure.exe
 *   interp_measure.exe <path of HGSIM.DLL> [trials]
 *
 * The DLL's sections are loaded as the sim memory (its constants and tables); the callees are stubs that log their
 * arguments and answer the same values to both versions. Every 32-bit word that comes out (outputs, sim memory,
 * logged arguments) is compared: equal, or both ordinary floats (1e-30 < |x| < 1e30) within a relative difference
 * that is reported; anything else counts as a failure.
 */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "sim/simmem.h"
#include "sim/util/w10_interp.h"

unsigned char *g_simBase;

void __cdecl Sim_10060681_x87(const float *m, float *pitch, float *heading, float *roll);
void __cdecl Sim_100d71b9_x87(void *obj, uint8_t *node, float dx, float dy, float dz, int spread,
                              const uint8_t *weapon, void *q);
void __cdecl Sim_1000cbaa_x87(void);
void __cdecl Sim_100608ef_x87(const float *a, const float *b, float t, float *out);
void __cdecl Sim_10060220_x87(float *m, float ax, float ay, float az, uint32_t tx, uint32_t ty, uint32_t tz,
                              uint32_t flags);
int __cdecl Sim_1001a706_x87(int mode, char *text, int32_t time);

/* ---- the platform math: UCRT ---------------------------------------------------------------------------- */
typedef double(__cdecl *MathFn)(double);
static MathFn u_sin, u_cos, u_asin, u_acos;
double sin(double x) { return u_sin(x); }
double cos(double x) { return u_cos(x); }
double asin(double x) { return u_asin(x); }
double acos(double x) { return u_acos(x); }

/* ---- random numbers --------------------------------------------------------------------------------------- */
static uint64_t g_rng = 0x9e3779b97f4a7c15ull;
static uint64_t rnd64(void)
{
    uint64_t z = (g_rng += 0x9e3779b97f4a7c15ull);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
    return z ^ (z >> 31);
}
static double uni(double a, double b) { return a + (b - a) * (double)(rnd64() >> 11) * (1.0 / 9007199254740992.0); }
static int pick(int n) { return (int)(rnd64() % (uint64_t)n); }
static uint32_t fb(double v)
{
    float f = (float)v;
    uint32_t u;
    memcpy(&u, &f, 4);
    return u;
}
static float bf(uint32_t u)
{
    float f;
    memcpy(&f, &u, 4);
    return f;
}

/* ---- call log and stub answers ----------------------------------------------------------------------------- */
typedef struct {
    int id, n;
    uint32_t w[12];
} Rec;
static Rec g_log[2][96];
static int g_logn[2], g_run;
static uint32_t g_ans[8][8];   /* answers per stub: g_ans[stub][call] */
static int g_ansn[8];

static void logw(int id, int n, const uint32_t *w)
{
    Rec *r;
    if (g_logn[g_run] >= 96)
        return;
    r = &g_log[g_run][g_logn[g_run]++];
    r->id = id;
    r->n = n;
    memcpy(r->w, w, 4 * (size_t)n);
}
static uint32_t ans(int stub) { return g_ans[stub][g_ansn[stub]++ & 7]; }
static uint32_t ub(float f)
{
    uint32_t u;
    memcpy(&u, &f, 4);
    return u;
}

double __cdecl Sim_10084b57(float a, float b)
{
    uint32_t w[2] = {ub(a), ub(b)};
    logw(1, 2, w);
    return bf(ans(0));
}
float __cdecl Sim_1008486e(float x, float y)
{
    uint32_t w[2] = {ub(x), ub(y)};
    logw(2, 2, w);
    return bf(ans(1));
}
void __cdecl Sim_100aa0da(void *obj, uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    uint32_t w[5] = {(uint32_t)(uintptr_t)obj, a, b, c, d};
    logw(3, 5, w);
}
void __cdecl Sim_100d709d(void *obj, uint32_t *a, uint32_t *b, uint32_t *c, void *q)
{
    uint32_t w[2] = {(uint32_t)(uintptr_t)obj, (uint32_t)(uintptr_t)q};
    logw(4, 2, w);
    *a = ans(2), *b = ans(2), *c = ans(2);
}
int __cdecl Sim_1009c929(int m)
{
    uint32_t w[1] = {(uint32_t)m};
    logw(5, 1, w);
    return (int)ans(3);
}
void __cdecl Sim_10084c95(float *x, float *y, float *z)
{
    uint32_t w[3] = {ub(*x), ub(*y), ub(*z)};
    logw(6, 3, w);
    *x = bf(ans(4)), *y = bf(ans(4)), *z = bf(ans(4));
}
void __cdecl Sim_100a9d47(void *obj, uint32_t a, uint32_t b, uint32_t c)
{
    uint32_t w[4] = {(uint32_t)(uintptr_t)obj, a, b, c};
    logw(7, 4, w);
}
void __cdecl Sim_100a9d7c(void *obj, float dx, float dy, float dz)
{
    uint32_t w[4] = {(uint32_t)(uintptr_t)obj, ub(dx), ub(dy), ub(dz)};
    logw(8, 4, w);
}

/* 0x1001a706: the record slot, the interpolators and the node (mode 1, the path step, is what is measured; the
 * other callees only make the two versions link) */
static uint32_t g_slot;
static uint8_t g_rec[0x88];
uint32_t __cdecl Sim_100cc6e0(void) { return 0; }
int __cdecl Sim_100cc8ba(int p) { (void)p; return (int)(uintptr_t)&g_slot; }
uint32_t __cdecl Sim_100096ee(void *obj)
{
    uint32_t w[1] = {(uint32_t)(uintptr_t)obj};
    logw(13, 1, w);
    return 0x00c90000u;
}
float __cdecl Sim_10047bcd(uint32_t *obj)
{
    uint32_t w[1] = {(uint32_t)((uint8_t *)obj - g_rec)};
    logw(14, 1, w);
    return bf(ans(5));
}
float __cdecl Sim_10047cf2(uint32_t *obj)
{
    uint32_t w[1] = {(uint32_t)((uint8_t *)obj - g_rec)};
    logw(15, 1, w);
    return bf(ans(6));
}
void __cdecl Sim_100aa799(void *obj)
{
    uint32_t w[1] = {(uint32_t)(uintptr_t)obj};
    logw(16, 1, w);
}
int __cdecl Sim_10047b76(void *rec, uint32_t a, uint32_t b, double d)
{
    uint32_t w[4] = {(uint32_t)((uint8_t *)rec - g_rec), a, b, (uint32_t)(d * 1e6)};
    logw(17, 4, w);
    return 0;
}
int __cdecl Sim_10047c92(void *rec, uint32_t a, uint32_t b, double d, uint32_t e)
{
    uint32_t w[5] = {(uint32_t)((uint8_t *)rec - g_rec), a, b, (uint32_t)(d * 1e6), e};
    logw(18, 5, w);
    return 0;
}
int __cdecl Sim_100fd710(const char *s, const char *format, ...) { (void)s, (void)format; return 0; }
int __cdecl Sim_100db770(int value) { return value; }
int __cdecl Sim_100b38c5(int key) { (void)key; return -1; }
void *__cdecl Sim_100b399c(int index) { (void)index; return 0; }
int32_t __cdecl Crt_AToI(const char *s) { return atoi(s); }
char *__cdecl Crt_StrChr(const char *s, int c) { return strchr(s, c); }
int __cdecl Str_CompareNoCase(const char *a, const char *b) { return _stricmp(a, b); }

/* functions of the DLL called through their address (SIM_FN): jump thunks to these */
static double __cdecl h_fabs(double x) { return fabs(x); }
static void __cdecl h_ident(float *m)
{
    static const float id[12] = {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0};
    memcpy(m, id, sizeof id);
    logw(9, 0, 0);
}
static void __cdecl h_fix(float *m, int order)
{
    uint32_t w[12];
    memcpy(w, m, 44);
    w[11] = (uint32_t)order;
    logw(10, 12, w);
}
static void __cdecl h_copy(float *from, float *to)
{
    uint32_t w[12];
    memcpy(w, from, 48);
    logw(11, 12, w);
    (void)to;
}
static void __cdecl h_assert(const char *e, const char *f, unsigned line)
{
    uint32_t w[1] = {line};
    logw(12, 1, w);
    (void)e, (void)f;
}
static void thunk(uint32_t addr, void *to)
{
    unsigned char *at = SIM_PTR(unsigned char, addr);
    int32_t rel = (int32_t)((intptr_t)to - (intptr_t)(at + 5));
    at[0] = 0xe9;
    memcpy(at + 1, &rel, 4);
}

/* ---- comparison --------------------------------------------------------------------------------------------- */
typedef struct {
    const char *name;
    long trials, trials_diff, words_diff, bad;
    double max_abs, max_rel;  /* max_rel and max_ulp only over values of magnitude >= 1e-6 */
    int max_ulp;
} Stats;

static int ord(uint32_t u) { return (u & 0x80000000u) ? -(int)(u & 0x7fffffffu) : (int)u; }

/* Returns the number of differing words. A difference is accepted when both words are finite floats within
 * 1e-4 * max(1, |value|) of each other (far beyond any rounding; the report gives the real figures); anything else
 * (NaN against a number, infinities, integer words) is a failure in s->bad. */
static int cmp_words(Stats *s, const uint32_t *a, const uint32_t *b, int n)
{
    int i, d = 0;
    for (i = 0; i < n; i++) {
        float x, y;
        double absd, big;
        if (a[i] == b[i])
            continue;
        x = bf(a[i]), y = bf(b[i]);
        absd = fabs((double)x - (double)y);
        big = fmax(fabs((double)x), fabs((double)y));
        if (!isfinite(x) || !isfinite(y) || !(absd <= 1e-4 * fmax(1.0, big))) {
            s->bad++;
            continue;
        }
        if (absd > s->max_abs)
            s->max_abs = absd;
        if (big >= 1e-6) {
            int ulp = abs(ord(a[i]) - ord(b[i]));
            if (absd / big > s->max_rel)
                s->max_rel = absd / big;
            if (ulp > s->max_ulp)
                s->max_ulp = ulp;
        }
        d++;
    }
    s->words_diff += d;
    return d;
}

static int cmp_logs(Stats *s)
{
    int i, d = 0;
    if (g_logn[0] != g_logn[1]) {
        s->bad++;
        return 1;
    }
    for (i = 0; i < g_logn[0]; i++) {
        if (g_log[0][i].id != g_log[1][i].id || g_log[0][i].n != g_log[1][i].n) {
            s->bad++;
            return 1;
        }
        d += cmp_words(s, g_log[0][i].w, g_log[1][i].w, g_log[0][i].n);
    }
    return d;
}

static void start(int run)
{
    g_run = run;
    g_logn[run] = 0;
    memset(g_ansn, 0, sizeof g_ansn);
    *SIM_PTR(uint32_t, 0x1017e888) = 0;  /* errno of the original's C runtime */
}

static void answers(void)
{
    int i, k;
    for (i = 0; i < 8; i++)
        for (k = 0; k < 8; k++)
            g_ans[i][k] = (uint32_t)rnd64();
}

static void finish(Stats *s, int d)
{
    s->trials++;
    if (d)
        s->trials_diff++;
}

/* ---- the six functions --------------------------------------------------------------------------------------- */
static void t_10060681(Stats *s)
{
    float m[9], o[2][3];
    uint32_t e[2];
    int i, run, d;
    for (i = 0; i < 9; i++)
        m[i] = (float)uni(-1, 1);
    switch (pick(6)) {
    case 0: m[5] = (float)uni(-0.1, 0.1); break;
    case 1: m[5] = (float)uni(0.09, 0.11) * (pick(2) ? 1 : -1); break;
    case 2: m[5] = bf(0x7fc00000); break;
    default: break;
    }
    answers();
    switch (pick(5)) {
    case 0: g_ans[0][0] = fb(uni(0, 0.004)); break;
    case 1: g_ans[0][0] = fb(uni(1.0, 2.0)); break;
    case 2: g_ans[0][0] = 0x7fc00000; break;
    default: g_ans[0][0] = fb(uni(0.004, 1.0)); break;
    }
    for (i = 0; i < 8; i++)
        g_ans[1][i] = fb(uni(-180, 180));
    for (run = 0; run < 2; run++) {
        start(run);
        (run ? Sim_10060681 : Sim_10060681_x87)(m, &o[run][0], &o[run][1], &o[run][2]);
        e[run] = *SIM_PTR(uint32_t, 0x1017e888);
    }
    d = cmp_words(s, (uint32_t *)o[0], (uint32_t *)o[1], 3) + cmp_logs(s);
    if (e[0] != e[1])
        s->bad++;
    finish(s, d);
}

static void t_100d71b9(Stats *s)
{
    static uint32_t node[0x40], blk[0x18], weapon[4], obj[0x10], q[0x10];
    float dx = (float)uni(-1, 1), dy, dz = (float)uni(-1, 1);
    uint32_t e[2];
    int i, run, d, spread = pick(2);
    switch (pick(6)) {
    case 0: dy = (float)uni(1, 3) * (pick(2) ? 1 : -1); break;
    case 1: dy = bf(0x7fc00000); break;
    case 2: dy = (float)uni(0.99, 1.0); break;
    default: dy = (float)uni(-1, 1); break;
    }
    blk[0x50 / 4] = fb(uni(0, 500));
    node[0x8c / 4] = (uint32_t)(uintptr_t)blk;
    weapon[2] = pick(3) ? 0 : 0x40;
    answers();
    for (i = 0; i < 8; i++)
        g_ans[1][i] = fb(uni(-180, 180)), g_ans[2][i] = fb(uni(-3000, 3000)), g_ans[4][i] = fb(uni(-1, 1));
    for (i = 0; i < 8; i++)
        g_ans[3][i] = (uint32_t)pick(200);
    for (run = 0; run < 2; run++) {
        start(run);
        (run ? Sim_100d71b9 : Sim_100d71b9_x87)(obj, (uint8_t *)node, dx, dy, dz, spread, (uint8_t *)weapon, q);
        e[run] = *SIM_PTR(uint32_t, 0x1017e888);
    }
    d = cmp_logs(s);
    if (e[0] != e[1])
        s->bad++;
    finish(s, d);
}

static void t_1000cbaa(Stats *s)
{
    static uint32_t tab[2][46 * 0xa8 / 4], cnt[2], e[2];
    uint8_t *kinds = SIM_PTR(uint8_t, 0x10145508);
    int k, run, d;
    *SIM_PTR(uint32_t, 0x1013abc0) = pick(2) ? fb(0.0296) : fb(uni(0.001, 0.1));
    for (k = 0; k < 0x2e; k++) {
        uint8_t *r = kinds + k * 0xd6;
        double lo = uni(0, 2000);
        uint32_t v;
        v = pick(3) ? 0x40 : 0;
        memcpy(r + 0x86, &v, 4);
        v = fb(lo);
        memcpy(r + 0x4c, &v, 4);
        v = fb(lo + (pick(4) ? uni(1, 8000) : 0.5));
        memcpy(r + 0x50, &v, 4);
        v = pick(6) ? fb(pick(2) ? uni(1, 40) : uni(10, 2000)) : 0;
        memcpy(r + 0x3c, &v, 4);
        v = pick(5) ? 1 : (uint32_t)pick(3);
        memcpy(r + 0x38, &v, 4);
    }
    for (run = 0; run < 2; run++) {
        start(run);
        *SIM_PTR(uint32_t, 0x1015d0a0) = 0;
        memset(SIM_PTR(uint8_t, 0x10a3a600), 0, sizeof tab[0]);
        (run ? Sim_1000cbaa : Sim_1000cbaa_x87)();
        memcpy(tab[run], SIM_PTR(uint8_t, 0x10a3a600), sizeof tab[0]);
        cnt[run] = *SIM_PTR(uint32_t, 0x1015d0a0);
        e[run] = *SIM_PTR(uint32_t, 0x1017e888);
    }
    d = cmp_words(s, tab[0], tab[1], (int)(sizeof tab[0] / 4));
    if (cnt[0] != cnt[1] || e[0] != e[1])
        s->bad++;
    finish(s, d);
}

static void quat(float *q)
{
    double v[4], n = 0;
    int i;
    for (i = 0; i < 4; i++)
        v[i] = uni(-1, 1), n += v[i] * v[i];
    n = sqrt(n);
    for (i = 0; i < 4; i++)
        q[i] = (float)(v[i] / n);
}

static void t_100608ef(Stats *s)
{
    float a[4], b[4], o[2][4], t;
    uint32_t e[2];
    int i, run, d, mode = pick(10);
    quat(a);
    if (mode == 0)
        memcpy(b, a, sizeof b);
    else if (mode == 1)
        for (i = 0; i < 4; i++)
            b[i] = -a[i];
    else if (mode == 2)
        for (i = 0; i < 4; i++)
            b[i] = a[i] + (float)uni(-1e-3, 1e-3);
    else
        quat(b);
    switch (pick(8)) {
    case 0: t = (float)uni(-2, 3); break;
    case 1: t = bf(0x7f800000); break;
    case 2: t = bf(0x7fc00000); break;
    default: t = (float)uni(0, 1); break;
    }
    for (run = 0; run < 2; run++) {
        start(run);
        (run ? Sim_100608ef : Sim_100608ef_x87)(a, b, t, o[run]);
        e[run] = *SIM_PTR(uint32_t, 0x1017e888);
    }
    d = cmp_words(s, (uint32_t *)o[0], (uint32_t *)o[1], 4) + cmp_logs(s);
    if (e[0] != e[1])
        s->bad++;
    finish(s, d);
}

static float angle(void)
{
    switch (pick(10)) {
    case 0: return 0.0f;
    case 1: return (float)uni(-1e-6, 1e-6);
    case 2: return 90.0f;
    case 3: return bf(0x7f800000);
    case 4: return bf(0x7fc00000);
    default: return (float)uni(-360, 360);
    }
}

static void t_10060220(Stats *s)
{
    float m[2][12];
    uint32_t e[2], flags = pick(4) ? (uint32_t)pick(8) : (uint32_t)rnd64();
    float ax = angle(), ay = angle(), az = angle();
    int run, d;
    for (run = 0; run < 2; run++) {
        start(run);
        memset(m[run], 0x5a, sizeof m[run]);
        (run ? Sim_10060220 : Sim_10060220_x87)(m[run], ax, ay, az, 1, 2, 3, flags);
        e[run] = *SIM_PTR(uint32_t, 0x1017e888);
    }
    d = cmp_words(s, (uint32_t *)m[0], (uint32_t *)m[1], 12) + cmp_logs(s);
    if (e[0] != e[1])
        s->bad++;
    if (d && getenv("MEASURE_DEBUG") && s->trials_diff < 3) {
        int i;
        printf("  angles %.9g %.9g %.9g flags %x\n", ax, ay, az, flags);
        for (i = 0; i < 12; i++)
            printf("   m[%d] %.9g %.9g\n", i, m[0][i], m[1][i]);
    }
    finish(s, d);
}

static void t_1001a706(Stats *s)
{
    static uint8_t path[0x44 + 6 * 0x1c], rec0[0x88], out[2][0x88];
    uint32_t e[2], v;
    int i, run, d, n = 2 + pick(4);
    int32_t time = pick(12000);
    memset(path, 0, sizeof path);
    memcpy(path + 0x40, &n, 4);
    for (i = 0; i < n; i++) {
        uint8_t *q = path + 0x44 + i * 0x1c;
        uint32_t w[7];
        w[0] = fb(uni(-5000, 5000));
        w[1] = fb(uni(-500, 500) * (pick(3) ? 1 : 60));
        w[2] = fb(uni(-5000, 5000));
        w[3] = fb(uni(-90, 90));
        w[4] = fb(uni(-400, 400));
        w[5] = fb(uni(-90, 90));
        w[6] = (uint32_t)(pick(5) ? 1 + pick(1000) : 0);
        memcpy(q, w, sizeof w);
    }
    for (i = 0; i < 0x88; i += 4) {
        v = fb(uni(-720, 720));
        memcpy(rec0 + i, &v, 4);
    }
    v = 0x00c70000u;
    memcpy(rec0, &v, 4);
    v = (uint32_t)(uintptr_t)path;
    memcpy(rec0 + 8, &v, 4);
    v = (uint32_t)pick(5000);
    memcpy(rec0 + 0xc, &v, 4);
    v = (uint32_t)pick(3000);
    memcpy(rec0 + 0x10, &v, 4);
    v = (uint32_t)(pick(4) ? 1 : 0);
    memcpy(rec0 + 0x14, &v, 4);
    v = (uint32_t)pick(3);
    memcpy(rec0 + 0x18, &v, 4);
    answers();
    for (i = 0; i < 8; i++)
        g_ans[1][i] = fb(uni(-180, 180)), g_ans[5][i] = fb(uni(-3000, 3000)), g_ans[6][i] = fb(uni(-360, 360));
    for (run = 0; run < 2; run++) {
        static uint32_t unit = 0x00c70000u;
        start(run);
        memcpy(g_rec, rec0, sizeof g_rec);
        v = (uint32_t)(uintptr_t)&unit;
        memcpy(g_rec, &v, 4);
        g_slot = (uint32_t)(uintptr_t)g_rec;
        (run ? Sim_1001a706 : Sim_1001a706_x87)(1, "1", time);
        memcpy(out[run], g_rec, sizeof g_rec);
        e[run] = *SIM_PTR(uint32_t, 0x1017e888);
    }
    d = cmp_words(s, (uint32_t *)out[0], (uint32_t *)out[1], 0x88 / 4) + cmp_logs(s);
    if (e[0] != e[1])
        s->bad++;
    finish(s, d);
}

/* ---- loading the DLL's sections as the sim memory --------------------------------------------------------- */
static int load(const char *path)
{
    FILE *f = fopen(path, "rb");
    long size;
    unsigned char *file;
    IMAGE_NT_HEADERS32 *nt;
    IMAGE_SECTION_HEADER *sec;
    int i;
    if (!f)
        return 0;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    file = malloc((size_t)size);
    if (!file || fread(file, 1, (size_t)size, f) != (size_t)size)
        return 0;
    fclose(f);
    nt = (IMAGE_NT_HEADERS32 *)(file + ((IMAGE_DOS_HEADER *)file)->e_lfanew);
    g_simBase = VirtualAlloc(0, nt->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_simBase)
        return 0;
    sec = IMAGE_FIRST_SECTION(nt);
    for (i = 0; i < nt->FileHeader.NumberOfSections; i++) {
        DWORD n = sec[i].SizeOfRawData < sec[i].Misc.VirtualSize ? sec[i].SizeOfRawData : sec[i].Misc.VirtualSize;
        memcpy(g_simBase + sec[i].VirtualAddress, file + sec[i].PointerToRawData, n);
    }
    free(file);
    return 1;
}

int main(int argc, char **argv)
{
    static Stats st[6] = {{"0x10060681"}, {"0x100d71b9"}, {"0x1000cbaa"}, {"0x100608ef"}, {"0x10060220"},
                          {"0x1001a706"}};
    void (*const fn[6])(Stats *) = {t_10060681, t_100d71b9, t_1000cbaa, t_100608ef, t_10060220, t_1001a706};
    long trials = argc > 2 ? atol(argv[2]) : 1000000;
    HMODULE ucrt = LoadLibraryA("ucrtbase.dll");
    int i, bad = 0;
    long k;
    if (argc < 2 || !load(argv[1]) || !ucrt) {
        fprintf(stderr, "usage: interp_measure <HGSIM.DLL> [trials]  (needs ucrtbase.dll)\n");
        return 2;
    }
    u_sin = (MathFn)GetProcAddress(ucrt, "sin");
    u_cos = (MathFn)GetProcAddress(ucrt, "cos");
    u_asin = (MathFn)GetProcAddress(ucrt, "asin");
    u_acos = (MathFn)GetProcAddress(ucrt, "acos");
    thunk(0x100fcb70, (void *)h_fabs);
    thunk(0x1006018d, (void *)h_ident);
    thunk(0x1005fcdc, (void *)h_fix);
    thunk(0x1006007d, (void *)h_copy);
    thunk(0x100ff860, (void *)h_assert);
    for (i = 0; i < 6; i++) {
        long n = i == 2 ? trials / 10 : trials;   /* the table function does 46 x 20 entries per trial */
        for (k = 0; k < n; k++)
            fn[i](&st[i]);
        printf("%s %ld trials: %ld with a difference (%ld words); max absolute %.3g; for |value| >= 1e-6 max "
               "relative %.3g, max %d float steps; %ld failures\n",
               st[i].name, st[i].trials, st[i].trials_diff, st[i].words_diff, st[i].max_abs, st[i].max_rel,
               st[i].max_ulp, st[i].bad);
        bad += st[i].bad != 0;
    }
    return bad ? 1 : 0;
}
