/* Link targets for platform/audio.h in the checker's flat test image; tools/verify/sound_common.py hooks every one of
 * them, so the bodies never run. */
#include "../../src/platform/audio.h"

int Audio_Open(void) { return 1; }
void Audio_Close(void) {}
int Audio_IsOpen(void) { return 1; }
void Audio_GetStats(AudioStats *out) { (void)out; }
AudioVoice *Audio_VoiceCreate(uint32_t bytes, uint32_t rate, uint32_t channels, uint32_t bits)
{
    (void)bytes, (void)rate, (void)channels, (void)bits;
    return 0;
}
void Audio_VoiceDestroy(AudioVoice *v) { (void)v; }
uint8_t *Audio_VoiceData(AudioVoice *v, uint32_t *bytes) { (void)v, (void)bytes; return 0; }
void Audio_VoicePlay(AudioVoice *v, int loop) { (void)v, (void)loop; }
void Audio_VoiceStop(AudioVoice *v) { (void)v; }
int Audio_VoiceIsPlaying(AudioVoice *v) { (void)v; return 0; }
int Audio_VoiceIsLooping(AudioVoice *v) { (void)v; return 0; }
int32_t Audio_VoiceSetVolume(AudioVoice *v, int32_t volume) { (void)v, (void)volume; return 0; }
int32_t Audio_VoiceSetPan(AudioVoice *v, int32_t pan) { (void)v, (void)pan; return 0; }
int32_t Audio_VoiceGetVolume(AudioVoice *v) { (void)v; return 0; }
int32_t Audio_VoiceGetPan(AudioVoice *v) { (void)v; return 0; }
int32_t Audio_VoiceSetRate(AudioVoice *v, uint32_t rate) { (void)v, (void)rate; return 0; }
uint32_t Audio_VoiceGetRate(AudioVoice *v) { (void)v; return 0; }
uint32_t Audio_VoiceGetPosition(AudioVoice *v) { (void)v; return 0; }
void Audio_VoiceSetPosition(AudioVoice *v, uint32_t bytes) { (void)v, (void)bytes; }
uint32_t Audio_VoiceGetStatus(AudioVoice *v) { (void)v; return 0; }
int Audio_VoiceLock(AudioVoice *v, uint32_t offset, uint32_t bytes, void **p1, uint32_t *n1, void **p2, uint32_t *n2,
                    uint32_t flags)
{
    (void)v, (void)offset, (void)bytes, (void)p1, (void)n1, (void)p2, (void)n2, (void)flags;
    return 0;
}
void Audio_VoiceUnlock(AudioVoice *v, void *p1, uint32_t n1, void *p2, uint32_t n2)
{
    (void)v, (void)p1, (void)n1, (void)p2, (void)n2;
}
