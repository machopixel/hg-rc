/*
 * SDL3 display backend (display.h). The game's window is wrapped, a renderer is
 * created on it and the picture goes up as a streaming texture. SDL receives the
 * window's messages while it is wrapped and queues events nobody reads, so the
 * queue is emptied on every picture.
 */
#include <stddef.h>

#include <SDL3/SDL.h>

#include "display.h"
#include "../launcher/debug.h"

static SDL_Window *g_window;
static SDL_Renderer *g_renderer;
static SDL_Texture *g_texture;
static int g_width, g_height;
static unsigned g_presented;
static int g_inited;

int Display_Open(void *nativeWindow, int width, int height)
{
    SDL_PropertiesID props;

    if (g_window != NULL)
        return 1;
    if (nativeWindow == NULL)
        return 0;
    SDL_SetHint(SDL_HINT_QUIT_ON_LAST_WINDOW_CLOSE, "0");
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        DebugPrintf("Display: SDL_InitSubSystem failed: %s\n", SDL_GetError());
        return 0;
    }
    g_inited = 1;
    props = SDL_CreateProperties();
    SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, nativeWindow);
    g_window = SDL_CreateWindowWithProperties(props);
    SDL_DestroyProperties(props);
    if (g_window != NULL)
        g_renderer = SDL_CreateRenderer(g_window, NULL);
    if (g_renderer != NULL)
        g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888,
                                      SDL_TEXTUREACCESS_STREAMING, width, height);
    if (g_texture == NULL) {
        DebugPrintf("Display: cannot start: %s\n", SDL_GetError());
        Display_Close();
        return 0;
    }
    SDL_SetTextureScaleMode(g_texture, SDL_SCALEMODE_LINEAR);
    g_width = width;
    g_height = height;
    g_presented = 0;
    DebugPrintf("Display: SDL3 %dx%d picture, renderer %s\n", width, height,
                SDL_GetRendererName(g_renderer));
    return 1;
}

void Display_Close(void)
{
    if (g_texture != NULL)
        SDL_DestroyTexture(g_texture);
    if (g_renderer != NULL)
        SDL_DestroyRenderer(g_renderer);
    if (g_window != NULL)
        SDL_DestroyWindow(g_window); /* gives the window procedure back to the game */
    if (g_window != NULL)
        DebugPrintf("Display: closed after %u pictures\n", g_presented);
    g_texture = NULL;
    g_renderer = NULL;
    g_window = NULL;
    if (g_inited)
        SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    g_inited = 0;
}

int Display_IsOpen(void)
{
    return g_window != NULL;
}

/* The largest picture that fits the window, centred. */
static void Layout(float *scale, float *offsetX, float *offsetY)
{
    int cw = 0, ch = 0;
    float sx, sy;

    SDL_GetCurrentRenderOutputSize(g_renderer, &cw, &ch);
    sx = (float)cw / (float)g_width;
    sy = (float)ch / (float)g_height;
    *scale = sx < sy ? sx : sy;
    if (*scale <= 0.0f)
        *scale = 1.0f;
    *offsetX = ((float)cw - (float)g_width * *scale) * 0.5f;
    *offsetY = ((float)ch - (float)g_height * *scale) * 0.5f;
}

void Display_ClientToPicture(int *x, int *y)
{
    float s, ox, oy;

    if (g_renderer == NULL)
        return;
    Layout(&s, &ox, &oy);
    *x = (int)SDL_floorf(((float)*x - ox) / s);
    *y = (int)SDL_floorf(((float)*y - oy) / s);
}

void Display_PictureToClient(int *x, int *y)
{
    float s, ox, oy;

    if (g_renderer == NULL)
        return;
    Layout(&s, &ox, &oy);
    *x = (int)(ox + ((float)*x + 0.5f) * s);
    *y = (int)(oy + ((float)*y + 0.5f) * s);
}

void Display_Present(const uint32_t *argb)
{
    SDL_FRect dst;
    float s;

    if (g_texture == NULL)
        return;
    Layout(&s, &dst.x, &dst.y);
    if (g_presented == 0) {
        int ow = 0, oh = 0;

        SDL_GetCurrentRenderOutputSize(g_renderer, &ow, &oh);
        DebugPrintf("Display: render output %dx%d, picture scale %.3f at %.1f,%.1f\n", ow, oh,
                    s, dst.x, dst.y);
    }
    dst.w = (float)g_width * s;
    dst.h = (float)g_height * s;
    SDL_UpdateTexture(g_texture, NULL, argb, g_width * 4);
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
    SDL_RenderClear(g_renderer);
    SDL_RenderTexture(g_renderer, g_texture, NULL, &dst);
    SDL_RenderPresent(g_renderer);
    SDL_FlushEvents(SDL_EVENT_FIRST, SDL_EVENT_LAST);
    g_presented++;
}
