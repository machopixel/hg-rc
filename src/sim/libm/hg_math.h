/*
 * C runtime replacement (DECISIONS.md, "C runtime replacement"): the maths functions the game uses, from OpenLibm
 * v0.8.8 (third_party/openlibm), compiled into the program under these names. They replace the original C runtime's
 * x87 routines (sqrt 0x100fca04, fmod 0x100fcb30, fabs 0x100fcb70, pow 0x100fed29, sin 0x100ff644, atan 0x10100044,
 * asin 0x100fcc74) and the platform library in the interpreted functions (round 10). Plain C, on every platform.
 */
#ifndef HG_SIM_LIBM_HG_MATH_H
#define HG_SIM_LIBM_HG_MATH_H

double hg_sqrt(double x);
double hg_fmod(double x, double y);
double hg_fabs(double x);
double hg_pow(double x, double y);
double hg_sin(double x);
double hg_cos(double x);
double hg_atan(double x);
double hg_asin(double x);
double hg_acos(double x);
double hg_floor(double x);

#endif
