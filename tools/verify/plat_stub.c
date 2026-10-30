/* Link targets for platform/plat.h in the checker's flat test image; the round-28 specs hook every one of them by
 * symbol, so the bodies never run. */
#include "../../src/platform/plat.h"

void Plat_WindowShow(void *window, int how) { (void)window, (void)how; }
void Plat_WindowPainted(void *window) { (void)window; }
int32_t Plat_WindowDefault(void *window, uint32_t message, uint32_t wParam, uint32_t lParam)
{
    (void)window, (void)message, (void)wParam, (void)lParam;
    return 0;
}
uint32_t Plat_EventTime(void) { return 0; }
void Plat_RequestQuit(void) {}
void Plat_CursorVisible(int visible) { (void)visible; }
uint32_t Plat_TicksMs(void) { return 0; }
uint32_t Plat_CurrentDir(char *out, uint32_t size) { (void)out, (void)size; return 0; }
void *Plat_LoadLibrary(const char *name) { (void)name; return 0; }
void *Plat_LibraryFunction(void *library, const char *name) { (void)library, (void)name; return 0; }
void Plat_FreeLibrary(void *library) { (void)library; }
