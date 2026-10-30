/*
 * Round 9, letter h: text helpers of the string table loader: C escape sequences (0x100b6f6a) and the text
 * decoration of the test modes (0x100b7430).
 */
#ifndef HG_SIM_UNSORTED_100B_W9H_H
#define HG_SIM_UNSORTED_100B_W9H_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9h_text.c ---- */
/*
 * Resolves the C escape sequences of `text` in place and returns it (NULL stays NULL): \a \b \f \n \r \t \v \' \" \\ \?
 * give their character, \0..\7 take up to three octal digits (read with "%o"), \x up to three hex digits ("%x"; a bare
 * \x stays), any other \c stays as the two characters; the characters 10..13 are dropped; a backslash at the end
 * ends the text. Digit classes come from the C runtime's table [0x10159450], or Crt_IsCType when [0x1015965c] > 1.
 */
char *__cdecl Text_Unescape(char *text);
/*
 * Decorates the text *text (a heap block) for the localisation test modes: flags 1 or 8 replace it by a copy where
 * A E I O U a e i o u y ! ? are followed by their accented form (0xc4 0xcb 0xcf 0xd6 0xdc 0xe4 0xeb 0xef 0xf6 0xfc 0xff
 * 0xa1 0xbf) with '&' appended (nothing happens when the block cannot be allocated); flag 2 replaces every character
 * not in " \n\r\t\v\f" by '*' and the first and last character by '$' (an empty text writes the '$' before it like the
 * original); flag 4 replaces the text by "%d: %s" of `number` and the text. The first argument is not used.
 */
void __cdecl Sim_100b7430(uint32_t unused, char **text, uint32_t flags, uint32_t number);

#endif
