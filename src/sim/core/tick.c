#include "core.h"

static CoreImports g_imports;

void Core_SetImports(const CoreImports *imports)
{
    g_imports = *imports;
}

// FUNCTION: HGSIM 0x1002a7e0
void Core_RunSteps(void)
{
    uint64_t now = g_imports.clock();
    uint64_t elapsed = 0;
    uint64_t steps, i;

    if (*g_imports.lastTick != 0) {
        elapsed = now - *g_imports.lastTick;
        steps = elapsed / *g_imports.stepLength;
        for (i = 0; i < steps; i++)
            g_imports.step();
    }
    /*
     * The original leaves the elapsed time unset on the very first call, so the
     * remainder it keeps is whatever was on the stack (never more than a step
     * length). Zero elapsed time starts the count at the current time.
     */
    *g_imports.lastTick = now - elapsed % *g_imports.stepLength;
}
