#include "w3p.h"

/* 1.2's receive guard (1.2 0x101314c0): set by the message loop from the game-over flag [0x1015f1fc] before each
   message; the network message handlers ignore their message while it is set. On its own so every checker build
   of a handler links it. */
uint32_t g_w34NetOver;

/* 1.2's 10-player tables (w3p.h); 1.0 has 8 entries each at fixed addresses next to other globals */
uint32_t g_net10Heard[10];
uint32_t g_net10Db20[10];
uint32_t g_net10Left[10];
int32_t g_net10SnStamp[10];
uint8_t g_net10Mark[10];
uint8_t g_net10Ready[10];
uint8_t g_net10Row[10][0x16];
uint8_t g_net10DiSent[10][0xd1];
uint8_t *g_net10DiBuf;
