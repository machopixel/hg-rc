#include "w5z.h"
#include "../../platform/plat.h"
#include "../simmem.h"
#include "../util/access.h"

/* Sim_1008663f (0x1008663f, the original SimMain's __finally block: AbnormalTermination 0x1010026a, the routing
   message and the abnormal-end box) is deleted: only the original SimMain reaches it (its call at 0x10086638 and its
   scope table entry 0x1012b8c8); the converted SimMain does the same in its own W21_FINALLY (DECISIONS.md, "The last
   original code"). */
