/* Link targets for platform/picture.h in the checker's flat test image (no Vulkan there). The windowed display
 * driver's drawing goes to the loading-screen picture now (DECISIONS.md, "The loading screen through Vulkan"); its
 * specs are superseded, so these only let the files that call them link. */
#include "../../src/platform/picture.h"

int Pic_Open(void *window) { (void)window; return 0; }
void Pic_Close(void) {}
void Pic_SetPalette(const uint8_t *rgb6) { (void)rgb6; }
int Pic_Blit(const uint8_t *pixels, int width, int height, int dx, int dy, int sx, int sy, int w, int h)
{
    (void)pixels, (void)width, (void)dx, (void)dy, (void)sx, (void)sy, (void)w, (void)h;
    return height;
}
int Pic_Stretch(const uint8_t *pixels, int width, int height, int sx, int sy, int w, int h)
{
    (void)pixels, (void)width, (void)sx, (void)sy, (void)w, (void)h;
    return height;
}
