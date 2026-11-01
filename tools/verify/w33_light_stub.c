/* Round 33: the coloured-light globals of w12_sky.c, for checker builds that do not link the sky reader */
#include <stdint.h>

int32_t g_w33Ambient[3], g_w33Light1[3], g_w33Light2[3], g_w33Global[3];
float g_w33Light1Dir[3], g_w33Light2Dir[3], g_w33Light1Cam[3], g_w33Light2Cam[3];
