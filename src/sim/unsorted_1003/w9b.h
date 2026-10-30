/*
 * Round 9, letter b, unsorted_1003: the network message pump (receives the two-letter messages of the session and
 * hands each to its handler, then does the periodic sends) and the builder of the "DA" state message.
 * Helpers that are not converted are called through their original address with SIM_FN and the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1003_W9B_H
#define HG_SIM_UNSORTED_1003_W9B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: applies a received "DA" state message (message, player index, extra word of the receive). */
#define W9B3_CALL_1003f3f8 SIM_FN(void(__cdecl *)(uint8_t *, int, uint32_t), 0x1003f3f8)
/* The "RG" message handler (Sim_10040af8, round 20), called through its address. */
#define W9B3_CALL_10040af8 SIM_FN(void(__cdecl *)(uint8_t *), 0x10040af8)

/*
 * Receives every pending message (Sim_1007e984 into [0x1015d008]) and dispatches it by its two-letter tag
 * to the handlers of unsorted_1004 (the sender is found with Sim_1003db56; the type 0 system message "eD"
 * and the "SS" message end the loop). Then, when the word 0x1015ec7c is set, does the periodic work:
 * marks players silent for more than 0x38e ticks, sends own state, Sim_1004227f, checks every player with
 * Sim_10042908 and starts ([0x1015f1fc]) when all are ready; otherwise sends the pre-game messages. Always
 * returns 1.
 */
int __cdecl Net_ProcessMessages(void);
/*
 * Builds the 0x42-byte "DA" message of player `index` in [0x1015ec88] (position and orientation as integers,
 * the flags of its block) and sends it with Net_DpSend_1007e920 (guaranteed when `guaranteed` is set); then
 * Sim_1003ffe9 unless the block's mode (+0x4f0) is 4 or 5. Nothing for an entry in a state other than 0 or 3.
 */
void __cdecl Net_SendStateMessage(int index, int guaranteed);

#endif
