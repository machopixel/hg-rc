/*
 * Round 9, letter f: the numbered-argument text formatter 0x100b78d0 of HGSIM.DLL.
 */
#ifndef HG_SIM_UNSORTED_100B_W9F_H
#define HG_SIM_UNSORTED_100B_W9F_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9f_text.c ---- */
/*
 * Formats the text Sim_100b7834([0x1017d594], key) into out: its "%N:<spec>" items name the N-th extra argument
 * ("%%" is a percent sign). The specs are collected in argument order into one vsprintf format separated by
 * 0x7f, formatted with the extra arguments, cut apart again with strtok, and put into the text in its own
 * order. Returns the length of out, or -1 when the text is unknown, an item has no number, a 0x7f piece is
 * missing, vsprintf fails or a heap block cannot be had (the blocks got until then stay allocated, as in the
 * original).
 */
int32_t __cdecl Text_FormatNumbered(char *out, uint32_t key, ...);

#endif
