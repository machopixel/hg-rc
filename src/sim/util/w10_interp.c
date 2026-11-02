#include "w10_interp.h"
#include "../libm/hg_math.h"
#include "../simmem.h"
#include "access.h"
#include "util.h"
#include "w3o.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_1008/w5x.h"
#include "../unsorted_1008/w8w.h"
#include "../unsorted_100a/w3t.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100d/w2d.h"
#include "../unsorted_1004/w3s.h"
#include "../unsorted_1004/w6e.h"
#include "../unsorted_1004/w7m.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100d/fns.h"
#include "../unsorted_100f/w2k.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"

#define W10I_D(addr) (*SIM_PTR(const double, addr))

#define W10I_FN(name) name
#define W10I_K_ASIN(x) hg_asin(x)
#define W10I_K_ACOS(x) hg_acos(x)
#define W10I_K_SIN(x) hg_sin(x)
#define W10I_K_COS(x) hg_cos(x)

#include "w10_interp_body.h"