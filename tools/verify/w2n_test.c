/* Test entry for the checker only (spec_w2n.py): the float-to-int helper as a plain cdecl function. */
#include "sim/render/w2n.h"

int __cdecl Test_Ftol(double d)
{
    return Render_Ftol(d);
}
