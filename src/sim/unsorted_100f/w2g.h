/*
 * Wave 2g: C runtime helpers of HGSIM.DLL (0x100fd100..0x100fd560), converted
 * with their cdecl convention. The names carry the original address because
 * the DLL holds several copies of such helpers.
 */
#ifndef HG_SIM_W2G_100F_H
#define HG_SIM_W2G_100F_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* |value|; 0x80000000 stays 0x80000000. */
int __cdecl Math_AbsInt_100fd100(int value);
/*
 * strtok: the next token of `str` (or of the string saved in [0x1017e884] when
 * str is null) up to the first character of `delim`, which is overwritten with
 * 0; saves the position after it. Returns null when no token is left.
 */
char *__cdecl Str_NextToken_100fd560(char *str, const char *delim);

#endif
