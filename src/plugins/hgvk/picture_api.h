/*
 * HGVK.DLL's picture interface (export HgvkPictureApi): whole 32-bit pictures straight to the window through
 * Vulkan, for the sim's loading screen (src/platform/picture_hgvk.c; DECISIONS.md, "The loading screen through
 * Vulkan"). Vulkan starts here when the renderer has not started it yet; FirstDevice then keeps it.
 */
#ifndef HGVK_PICTURE_API_H
#define HGVK_PICTURE_API_H

#include <stdint.h>

typedef struct HgvkPictureFns {
    /* Starts Vulkan on the window (its HWND) unless it runs already; 0 on success. */
    int (*Open)(void *window);
    /* The picture is no longer shown; Vulkan stays for the renderer. */
    void (*Close)(void);
    /* Shows width x height pixels (bytes R, G, B, A in memory order), scaled to the window keeping their
     * proportions, and presents. */
    void (*Show)(const uint32_t *rgba, int width, int height);
} HgvkPictureFns;

typedef const HgvkPictureFns *(*HgvkPictureApiFn)(void);

#endif
