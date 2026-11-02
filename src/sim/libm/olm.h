/*
 * C runtime replacement (DECISIONS.md, "C runtime replacement"): included first by each wrapper of this folder
 * before one OpenLibm source file (third_party/openlibm). Renames OpenLibm's entry points to hg_* so they never
 * clash with the compiler's built-in maths or its C runtime. Needs the include paths third_party/openlibm/hg, then
 * third_party/openlibm/include.
 */
#ifndef HG_SIM_LIBM_OLM_H
#define HG_SIM_LIBM_OLM_H

#if defined(_M_IX86) && !defined(__i386__)
#define __i386__ 1   /* OpenLibm's fpmath.h picks the x86 layout by this name */
#endif

#define sqrt hg_sqrt
#define fmod hg_fmod
#define fabs hg_fabs
#define pow hg_pow
#define sin hg_sin
#define cos hg_cos
#define atan hg_atan
#define asin hg_asin
#define acos hg_acos
#define scalbn hg_scalbn
#define copysign hg_copysign
#define floor hg_floor

/* no aliases (ldexp, sinl, ...): only the hg_* names are built */
#define __weak_reference(sym, alias)
#define __strong_reference(sym, alias)

#ifdef _MSC_VER
#pragma warning(disable : 4018 4146 4244)   /* upstream code, built as it is */
#endif

#endif
