/* Link targets for platform/input.h in the checker's flat test image; the round-28 specs hook every one of them by
 * symbol (tools/verify/spec_w28_input.py), so the bodies never run. */
#include "../../src/platform/input.h"

int Input_Open(void *nativeWindow) { (void)nativeWindow; return 1; }
void Input_Close(void) {}
int Input_IsOpen(void) { return 1; }
void Input_Pump(InputPumpInfo *info) { (void)info; }
void Input_SetMessageRules(int *quitFlag, const int *cursorShown) { (void)quitFlag, (void)cursorShown; }
void Input_WaitEvent(void) {}
int Input_NextKey(InputKey *key) { (void)key; return 0; }
void Input_MouseCapture(int on) { (void)on; }
void Input_MouseTake(float *dx, float *dy) { (void)dx, (void)dy; }
unsigned Input_MouseButtons(void) { return 0; }
void Input_GetStats(InputStats *stats) { (void)stats; }
