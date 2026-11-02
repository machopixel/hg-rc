#include "w4u.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../win/w4u.h"
/* 1.2's WAV music (resume 0x10015f59, pause 0x10015f1b, stop 0x10015f97) with the Ogg files of platform/music.h in
 * place of the CD (Diego: the music plays from the music folder, the CD is ignored) */
#include "../../platform/music.h"
#define W4UC_PAUSE(on) Music_Pause(on)

// FUNCTION: HGSIM 0x1001f367
void __cdecl Sim_1001f367(void)
{
    if (G32(0x1015d1bc) != 0) {
        if (GS32(0x101321a8) != -1 && (G32(0x101321c0) & 8) != 0)
            W4UC_PAUSE(0);
        G32(0x1015d1bc) = 0;
    }
}

// FUNCTION: HGSIM 0x1001f335
void __cdecl Sim_1001f335(void)
{
    if (G32(0x1015d1bc) == 0) {
        if (GS32(0x101321a8) != -1 && (G32(0x101321c0) & 8) != 0)
            W4UC_PAUSE(1);
        G32(0x1015d1bc) = 1;
    }
}

// FUNCTION: HGSIM 0x1001f399
void __cdecl Sim_1001f399(void)
{
    if (GS32(0x101321a8) != -1) {
        Music_Stop();
        G32(0x1015d1bc) = 0;    /* 1.2 also clears the pause */
    }
}

// FUNCTION: HGSIM 0x1001f4d3
void __cdecl Sim_1001f4d3(void)
{
    Sim_1001f367();
}
