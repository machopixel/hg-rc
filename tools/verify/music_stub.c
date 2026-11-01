/* Link targets for platform/music.h in the checker's flat test image; spec_music_v12.py hooks every one of them, so
 * the bodies never run. */
#include "../../src/platform/music.h"

int Music_LastTrack(void) { return 0; }
int Music_HasTrack(int track) { (void)track; return 0; }
int Music_Play(int track) { (void)track; return 0; }
void Music_Pause(int pause) { (void)pause; }
void Music_Stop(void) {}
int Music_IsPlaying(void) { return 0; }
void Music_SetVolume(uint32_t volume) { (void)volume; }
void Music_Close(void) {}
