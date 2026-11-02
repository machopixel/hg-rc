/* Force feedback without a device (force.h): the state the original had when FFORCE.DLL did not load. */
#include "force.h"

#define FORCE_NOT_LOADED ((int32_t)0x8004050f)   /* what the sim answers itself when the library is missing */

int Force_Present(void)
{
    return 0;
}

int32_t Force_Init(uint32_t window, uint32_t arg)
{
    (void)window;
    (void)arg;
    return FORCE_NOT_LOADED;
}

int32_t Force_Effect(const void *table)
{
    (void)table;
    return FORCE_NOT_LOADED;
}

int32_t Force_Exit(void)
{
    return FORCE_NOT_LOADED;
}
