/*
 * Round 9, letter d: the command-line switch parser of HGSIM.DLL at 0x1002ad50.
 * The error report 0x100bc280 is patched by another hook file and is called through its original address.
 */
#ifndef HG_SIM_UNSORTED_1002_W9D_H
#define HG_SIM_UNSORTED_1002_W9D_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

typedef void(__cdecl *W9dErrorFn)(int code, ...);
#define W9D_CALL_100bc280 SIM_FN(W9dErrorFn, 0x100bc280)

/* ---- w9d_cmdline.c ---- */
/*
 * Splits `cmdline` into tokens (strtok with the delimiters at 0x101354e4; the token state is the C runtime's
 * at 0x1017e884). A token that starts with '/' or '-' is a switch: the words at 0x10135518.. (compared without
 * case) set the flags 0x1017b940, 0x1017b944, 0x1017c8c4 (nogrid clears it), 0x1017c8c0, 0x1017c8e8, the display
 * driver name at 0x109e89e4 (two names, bit 0 of 0x109e89e0) and 0x1017b954; else the upper-cased second
 * character selects: B/G = text after '=' into 0x1017d5dc / 0x1017d5e0, C opts[0] = 1, D 0x1015f1e8 = 1,
 * E deletes hgdebug.txt and sets 0x1015dc28, F 0x1015dc94 = 0xb5 / (value after '=', default 10), J (with '=')
 * 0x1017d154 = text and like L 0x101668f4 = 1, M mono on and 0x1015f1e8 = 1, N network (0x1015ec58, bit 2 of
 * 0x109e89e0, name after '=' into 0x109e89fe, loads ANET2.DLL), O (with '=' third) E/F texts into
 * 0x1017e6b4 / 0x1017e6b0, P 0x1015dc24 = 1, Q Sim_1003217c, R 0x1017b8d4 = 1, S opts[1] = 0, T
 * Sim_100a7880(text after '='), V G = driver name after '=' / '=' digit (ctype class 4) = 0x1013a00c,
 * X '=' F/G/M/P/S = Sim_10034855(4 / 1 / 1 / 1 / 2) with 0x1017b3bc and 0x1015d06c, then mono on, 0x1015f1e8
 * = 1, opts[1] = 0, Z '=' H = 0x1017b940 (1 if H else 0); anything else reports error 10 with the token.
 * Every other token is copied over `rest`. Finally the rest goes to the mono top line. Returns 1, or 0 when
 * `cmdline` is NULL (then a message is printed first and the previous token state continues).
 */
int __cdecl CmdLine_Parse(char *cmdline, uint32_t *opts, char *rest);

#endif
