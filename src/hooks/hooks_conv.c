/*
 * The small HGSIM.DLL functions converted to C (src/sim/util, view and the
 * unsorted_* folders), put in place of the originals. Each conv_*.inc lists
 * one folder's functions as CONV(rva, name, original bytes...); the bytes are
 * checked first, so a different build of the game is never patched. hgrc.cfg
 * word "noconv" leaves them all out, to compare against the original code.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../launcher/debug.h"
#include "../sim/core/core.h"
#include "../sim/core/w2a.h"
#include "../sim/core/w2b.h"
#include "../sim/unsorted_1000/fns.h"
#include "../sim/unsorted_1001/fns.h"
#include "../sim/unsorted_1003/fns.h"
#include "../sim/unsorted_1003/w2b.h"
#include "../sim/unsorted_1004/fns.h"
#include "../sim/unsorted_1004/w2b.h"
#include "../sim/unsorted_1004/w2k.h"
#include "../sim/unsorted_1004/w2c.h"
#include "../sim/unsorted_1005/fns.h"
#include "../sim/unsorted_1007/fns.h"
#include "../sim/unsorted_1008/fns.h"
#include "../sim/unsorted_1009/fns.h"
#include "../sim/unsorted_100b/fns.h"
#include "../sim/unsorted_100c/fns.h"
#include "../sim/unsorted_100d/fns.h"
#include "../sim/unsorted_100d/w2d.h"
#include "../sim/util/util.h"
#include "../sim/view/view.h"
#include "../sim/view/w2a.h"
#include "../sim/view/w2d.h"
#include "../sim/core/w2j.h"
#include "../sim/unsorted_1000/w2e.h"
#include "../sim/unsorted_1000/w2f.h"
#include "../sim/unsorted_1000/w2g.h"
#include "../sim/unsorted_1000/w2h.h"
#include "../sim/unsorted_1000/w2i.h"
#include "../sim/unsorted_1000/w2j.h"
#include "../sim/unsorted_1000/w2k.h"
#include "../sim/unsorted_1000/w2l.h"
#include "../sim/unsorted_1001/w2e.h"
#include "../sim/unsorted_1001/w2f.h"
#include "../sim/unsorted_1001/w2g.h"
#include "../sim/unsorted_1001/w2h.h"
#include "../sim/unsorted_1001/w2i.h"
#include "../sim/unsorted_1001/w2j.h"
#include "../sim/unsorted_1001/w2k.h"
#include "../sim/unsorted_1001/w2l.h"
#include "../sim/unsorted_1002/w2e.h"
#include "../sim/unsorted_1002/w2f.h"
#include "../sim/unsorted_1002/w2g.h"
#include "../sim/unsorted_1002/w2h.h"
#include "../sim/unsorted_1002/w2i.h"
#include "../sim/unsorted_1002/w2j.h"
#include "../sim/unsorted_1002/w2k.h"
#include "../sim/unsorted_1002/w2l.h"
#include "../sim/unsorted_1003/w2e.h"
#include "../sim/unsorted_1003/w2f.h"
#include "../sim/unsorted_1003/w2g.h"
#include "../sim/unsorted_1003/w2h.h"
#include "../sim/unsorted_1003/w2i.h"
#include "../sim/unsorted_1003/w2j.h"
#include "../sim/unsorted_1003/w2k.h"
#include "../sim/unsorted_1003/w2l.h"
#include "../sim/unsorted_1004/w2e.h"
#include "../sim/unsorted_1004/w2f.h"
#include "../sim/unsorted_1004/w2g.h"
#include "../sim/unsorted_1004/w2h.h"
#include "../sim/unsorted_1004/w2i.h"
#include "../sim/unsorted_1004/w2j.h"
#include "../sim/unsorted_1004/w2k.h"
#include "../sim/unsorted_1004/w2l.h"
#include "../sim/unsorted_1005/w2e.h"
#include "../sim/unsorted_1005/w2f.h"
#include "../sim/unsorted_1005/w2g.h"
#include "../sim/unsorted_1005/w2h.h"
#include "../sim/unsorted_1005/w2i.h"
#include "../sim/unsorted_1005/w2j.h"
#include "../sim/unsorted_1005/w2k.h"
#include "../sim/unsorted_1005/w2l.h"
#include "../sim/unsorted_1006/w2e.h"
#include "../sim/unsorted_1006/w2f.h"
#include "../sim/unsorted_1006/w2k.h"
#include "../sim/unsorted_1006/w2l.h"
#include "../sim/unsorted_1007/w2e.h"
#include "../sim/unsorted_1007/w2f.h"
#include "../sim/unsorted_1007/w2g.h"
#include "../sim/unsorted_1007/w2h.h"
#include "../sim/unsorted_1007/w2i.h"
#include "../sim/unsorted_1007/w2j.h"
#include "../sim/unsorted_1007/w2k.h"
#include "../sim/unsorted_1007/w2l.h"
#include "../sim/unsorted_1008/w2f.h"
#include "../sim/unsorted_1008/w2g.h"
#include "../sim/unsorted_1008/w2h.h"
#include "../sim/unsorted_1008/w2i.h"
#include "../sim/unsorted_1008/w2j.h"
#include "../sim/unsorted_1008/w2k.h"
#include "../sim/unsorted_1008/w2l.h"
#include "../sim/unsorted_1009/w2e.h"
#include "../sim/unsorted_1009/w2f.h"
#include "../sim/unsorted_1009/w2g.h"
#include "../sim/unsorted_1009/w2h.h"
#include "../sim/unsorted_1009/w2i.h"
#include "../sim/unsorted_1009/w2j.h"
#include "../sim/unsorted_1009/w2k.h"
#include "../sim/unsorted_1009/w2l.h"
#include "../sim/unsorted_100a/w2e.h"
#include "../sim/unsorted_100a/w2f.h"
#include "../sim/unsorted_100a/w2g.h"
#include "../sim/unsorted_100a/w2h.h"
#include "../sim/unsorted_100a/w2i.h"
#include "../sim/unsorted_100a/w2j.h"
#include "../sim/unsorted_100a/w2k.h"
#include "../sim/unsorted_100a/w2l.h"
#include "../sim/unsorted_100b/w2e.h"
#include "../sim/unsorted_100b/w2f.h"
#include "../sim/unsorted_100b/w2g.h"
#include "../sim/unsorted_100b/w2h.h"
#include "../sim/unsorted_100b/w2i.h"
#include "../sim/unsorted_100b/w2j.h"
#include "../sim/unsorted_100b/w2l.h"
#include "../sim/unsorted_100c/w2e.h"
#include "../sim/unsorted_100c/w2f.h"
#include "../sim/unsorted_100c/w2g.h"
#include "../sim/unsorted_100c/w2h.h"
#include "../sim/unsorted_100c/w2i.h"
#include "../sim/unsorted_100c/w2l.h"
#include "../sim/unsorted_100d/w2e.h"
#include "../sim/unsorted_100d/w2f.h"
#include "../sim/unsorted_100d/w2g.h"
#include "../sim/unsorted_100d/w2h.h"
#include "../sim/unsorted_100d/w2i.h"
#include "../sim/unsorted_100d/w2j.h"
#include "../sim/unsorted_100d/w2k.h"
#include "../sim/unsorted_100e/w2e.h"
#include "../sim/unsorted_100e/w2j.h"
#include "../sim/unsorted_100f/w2e.h"
#include "../sim/unsorted_100f/w2g.h"
#include "../sim/unsorted_100f/w2h.h"
#include "../sim/unsorted_100f/w2j.h"
#include "../sim/unsorted_100f/w2k.h"
#include "../sim/unsorted_1010/w2f.h"
#include "../sim/unsorted_1010/w2g.h"
#include "../sim/unsorted_1010/w2h.h"
#include "../sim/unsorted_1010/w2i.h"
#include "../sim/unsorted_1010/w2j.h"
#include "../sim/unsorted_1010/w2k.h"
#include "../sim/util/w2l.h"
#include "../sim/view/w2l.h"
#include "../sim/render/w2m.h"
#include "../sim/render/w2n.h"
#include "../sim/unsorted_1000/w3o.h"
#include "../sim/unsorted_1001/w3o.h"
#include "../sim/unsorted_1002/w3o.h"
#include "../sim/unsorted_1003/w3o.h"
#include "../sim/unsorted_1004/w3o.h"
#include "../sim/unsorted_1005/w3o.h"
#include "../sim/unsorted_1006/w3o.h"
#include "../sim/unsorted_1007/w3o.h"
#include "../sim/unsorted_1008/w3o.h"
#include "../sim/unsorted_1009/w3o.h"
#include "../sim/unsorted_100a/w3o.h"
#include "../sim/unsorted_100b/w3o.h"
#include "../sim/unsorted_100c/w3o.h"
#include "../sim/unsorted_100d/w3o.h"
#include "../sim/util/w3o.h"
#include "../sim/render/w3q.h"
#include "../sim/unsorted_1000/w3q.h"
#include "../sim/unsorted_1002/w3q.h"
#include "../sim/unsorted_1003/w3q.h"
#include "../sim/unsorted_1004/w3q.h"
#include "../sim/unsorted_1005/w3q.h"
#include "../sim/unsorted_1006/w3q.h"
#include "../sim/unsorted_1007/w3q.h"
#include "../sim/unsorted_1008/w3q.h"
#include "../sim/unsorted_1009/w3q.h"
#include "../sim/unsorted_100a/w3q.h"
#include "../sim/unsorted_100d/w3q.h"
#include "../sim/util/w3q.h"
#include "../sim/core/w3r.h"
#include "../sim/render/w3r.h"
#include "../sim/unsorted_1001/w3r.h"
#include "../sim/unsorted_1002/w3r.h"
#include "../sim/unsorted_1003/w3r.h"
#include "../sim/unsorted_1005/w3r.h"
#include "../sim/unsorted_1007/w3r.h"
#include "../sim/unsorted_1008/w3r.h"
#include "../sim/unsorted_100d/w3r.h"
#include "../sim/util/w3r.h"
#include "../sim/view/w3r.h"
#include "../sim/render/w3s.h"
#include "../sim/unsorted_1000/w3s.h"
#include "../sim/unsorted_1001/w3s.h"
#include "../sim/unsorted_1002/w3s.h"
#include "../sim/unsorted_1003/w3s.h"
#include "../sim/unsorted_1004/w3s.h"
#include "../sim/unsorted_1005/w3s.h"
#include "../sim/unsorted_1007/w3s.h"
#include "../sim/unsorted_1008/w3s.h"
#include "../sim/unsorted_100b/w3s.h"
#include "../sim/unsorted_100c/w3s.h"
#include "../sim/unsorted_100d/w3s.h"
#include "../sim/unsorted_1000/w3t.h"
#include "../sim/unsorted_1001/w3t.h"
#include "../sim/unsorted_1002/w3t.h"
#include "../sim/unsorted_1003/w3t.h"
#include "../sim/unsorted_1004/w3t.h"
#include "../sim/unsorted_1006/w3t.h"
#include "../sim/unsorted_1008/w3t.h"
#include "../sim/unsorted_1009/w3t.h"
#include "../sim/unsorted_100a/w3t.h"
#include "../sim/unsorted_100b/w3t.h"
#include "../sim/unsorted_100c/w3t.h"
#include "../sim/unsorted_100d/w3t.h"
#include "../sim/util/w3t.h"
#include "../sim/render/w3p.h"
#include "../sim/unsorted_1000/w3p.h"
#include "../sim/unsorted_1002/w3p.h"
#include "../sim/unsorted_1003/w3p.h"
#include "../sim/unsorted_1004/w3p.h"
#include "../sim/unsorted_1006/w3p.h"
#include "../sim/unsorted_1007/w3p.h"
#include "../sim/unsorted_1009/w3p.h"
#include "../sim/unsorted_100b/w3p.h"
#include "../sim/unsorted_100c/w3p.h"
#include "../sim/unsorted_100d/w3p.h"
#include "../sim/unsorted_100e/w3p.h"
#include "../sim/util/w3p.h"
#include "../sim/win/w4a.h"
#include "../sim/unsorted_1000/w4u.h"
#include "../sim/unsorted_1001/w4u.h"
#include "../sim/unsorted_1002/w4u.h"
#include "../sim/unsorted_1003/w4u.h"
#include "../sim/unsorted_1004/w4u.h"
#include "../sim/unsorted_1005/w4u.h"
#include "../sim/unsorted_1008/w4u.h"
#include "../sim/unsorted_100a/w4u.h"
#include "../sim/win/w4u.h"
#include "../sim/unsorted_1000/w4v.h"
#include "../sim/unsorted_1002/w4v.h"
#include "../sim/unsorted_1003/w4v.h"
#include "../sim/unsorted_1004/w4v.h"
#include "../sim/unsorted_1005/w4v.h"
#include "../sim/unsorted_1007/w4v.h"
#include "../sim/unsorted_1008/w4v.h"
#include "../sim/unsorted_100a/w4v.h"
#include "../sim/unsorted_100b/w4v.h"
#include "../sim/unsorted_100c/w4v.h"
#include "../sim/unsorted_100d/w4v.h"
#include "../sim/win/w4v.h"
#include "../sim/unsorted_1000/w4w.h"
#include "../sim/unsorted_1003/w4w.h"
#include "../sim/unsorted_1004/w4w.h"
#include "../sim/unsorted_1007/w4w.h"
#include "../sim/unsorted_1008/w4w.h"
#include "../sim/unsorted_100b/w4w.h"
#include "../sim/unsorted_100e/w4w.h"
#include "../sim/win/w4w.h"
#include "../sim/unsorted_1000/w5x.h"
#include "../sim/unsorted_1001/w5x.h"
#include "../sim/unsorted_1002/w5x.h"
#include "../sim/unsorted_1003/w5x.h"
#include "../sim/unsorted_1004/w5x.h"
#include "../sim/unsorted_1005/w5x.h"
#include "../sim/unsorted_1007/w5x.h"
#include "../sim/unsorted_1008/w5x.h"
#include "../sim/unsorted_1009/w5x.h"
#include "../sim/unsorted_100a/w5x.h"
#include "../sim/unsorted_100b/w5x.h"
#include "../sim/unsorted_100c/w5x.h"
#include "../sim/unsorted_100d/w5x.h"
#include "../sim/unsorted_1000/w5y.h"
#include "../sim/unsorted_1001/w5y.h"
#include "../sim/unsorted_1002/w5y.h"
#include "../sim/unsorted_1003/w5y.h"
#include "../sim/unsorted_1004/w5y.h"
#include "../sim/unsorted_1005/w5y.h"
#include "../sim/unsorted_1006/w5y.h"
#include "../sim/unsorted_1008/w5y.h"
#include "../sim/unsorted_1009/w5y.h"
#include "../sim/unsorted_100a/w5y.h"
#include "../sim/unsorted_100b/w5y.h"
#include "../sim/unsorted_100c/w5y.h"
#include "../sim/unsorted_100d/w5y.h"
#include "../sim/unsorted_1000/w5z.h"
#include "../sim/unsorted_1001/w5z.h"
#include "../sim/unsorted_1002/w5z.h"
#include "../sim/unsorted_1004/w5z.h"
#include "../sim/unsorted_1006/w5z.h"
#include "../sim/unsorted_1007/w5z.h"
#include "../sim/unsorted_1008/w5z.h"
#include "../sim/unsorted_1009/w5z.h"
#include "../sim/unsorted_100a/w5z.h"
#include "../sim/unsorted_100b/w5z.h"
#include "../sim/unsorted_100c/w5z.h"
#include "../sim/unsorted_100d/w5z.h"
#include "../sim/unsorted_1000/w5a.h"
#include "../sim/unsorted_1001/w5a.h"
#include "../sim/unsorted_1002/w5a.h"
#include "../sim/unsorted_1003/w5a.h"
#include "../sim/unsorted_1004/w5a.h"
#include "../sim/unsorted_1005/w5a.h"
#include "../sim/unsorted_1006/w5a.h"
#include "../sim/unsorted_1007/w5a.h"
#include "../sim/unsorted_1008/w5a.h"
#include "../sim/unsorted_1009/w5a.h"
#include "../sim/unsorted_100a/w5a.h"
#include "../sim/unsorted_100b/w5a.h"
#include "../sim/unsorted_100c/w5a.h"
#include "../sim/unsorted_1000/w5b.h"
#include "../sim/unsorted_1001/w5b.h"
#include "../sim/unsorted_1002/w5b.h"
#include "../sim/unsorted_1003/w5b.h"
#include "../sim/unsorted_1004/w5b.h"
#include "../sim/unsorted_1005/w5b.h"
#include "../sim/unsorted_1006/w5b.h"
#include "../sim/unsorted_1008/w5b.h"
#include "../sim/unsorted_1009/w5b.h"
#include "../sim/unsorted_100a/w5b.h"
#include "../sim/unsorted_100b/w5b.h"
#include "../sim/unsorted_100c/w5b.h"
#include "../sim/unsorted_100d/w5b.h"
#include "../sim/unsorted_100e/w5b.h"
#include "../sim/unsorted_1000/w5c.h"
#include "../sim/unsorted_1001/w5c.h"
#include "../sim/unsorted_1002/w5c.h"
#include "../sim/unsorted_1003/w5c.h"
#include "../sim/unsorted_1004/w5c.h"
#include "../sim/unsorted_1005/w5c.h"
#include "../sim/unsorted_1006/w5c.h"
#include "../sim/unsorted_1008/w5c.h"
#include "../sim/unsorted_1009/w5c.h"
#include "../sim/unsorted_100a/w5c.h"
#include "../sim/unsorted_100b/w5c.h"
#include "../sim/unsorted_100c/w5c.h"
#include "../sim/unsorted_100d/w5c.h"
#include "../sim/unsorted_1000/w6d.h"
#include "../sim/unsorted_1001/w6d.h"
#include "../sim/unsorted_1002/w6d.h"
#include "../sim/unsorted_1003/w6d.h"
#include "../sim/unsorted_1003/w6e.h"
#include "../sim/unsorted_1004/w6e.h"
#include "../sim/unsorted_1005/w6e.h"
#include "../sim/unsorted_1006/w6e.h"
#include "../sim/unsorted_1007/w6e.h"
#include "../sim/unsorted_1008/w6f.h"
#include "../sim/unsorted_1009/w6f.h"
#include "../sim/unsorted_1009/w6g.h"
#include "../sim/unsorted_100a/w6g.h"
#include "../sim/unsorted_100b/w6g.h"
#include "../sim/unsorted_100c/w6g.h"
#include "../sim/win/w6g.h"
#include "../sim/unsorted_100c/w6h.h"
#include "../sim/unsorted_100d/w6h.h"
#include "../sim/unsorted_100e/w6h.h"
#include "../sim/unsorted_100f/w6h.h"
#include "../sim/unsorted_100f/w6i.h"
#include "../sim/unsorted_1010/w6i.h"
#include "../sim/unsorted_1010/w6j.h"
#include "../sim/unsorted_1010/w6k.h"
#include "../sim/render/w7o.h"
#include "../sim/unsorted_1000/w7l.h"
#include "../sim/unsorted_1001/w7l.h"
#include "../sim/unsorted_1002/w7l.h"
#include "../sim/unsorted_1003/w7l.h"
#include "../sim/unsorted_1003/w7m.h"
#include "../sim/unsorted_1004/w7m.h"
#include "../sim/unsorted_1005/w7m.h"
#include "../sim/unsorted_1005/w7n.h"
#include "../sim/unsorted_1006/w7n.h"
#include "../sim/unsorted_1007/w7n.h"
#include "../sim/unsorted_1008/w7n.h"
#include "../sim/unsorted_1008/w7o.h"
#include "../sim/unsorted_1009/w7o.h"
#include "../sim/unsorted_100a/w7o.h"
#include "../sim/unsorted_100a/w7p.h"
#include "../sim/unsorted_100b/w7p.h"
#include "../sim/unsorted_100c/w7p.h"
#include "../sim/unsorted_100c/w7q.h"
#include "../sim/unsorted_100d/w7q.h"
#include "../sim/unsorted_100f/w7q.h"
#include "../sim/unsorted_100f/w7r.h"
#include "../sim/unsorted_1010/w7r.h"
#include "../sim/unsorted_1010/w7s.h"
#include "../sim/unsorted_1000/w8t.h"
#include "../sim/unsorted_1001/w8t.h"
#include "../sim/unsorted_1002/w8t.h"
#include "../sim/unsorted_1002/w8u.h"
#include "../sim/unsorted_1003/w8u.h"
#include "../sim/unsorted_1004/w8u.h"
#include "../sim/unsorted_1004/w8v.h"
#include "../sim/unsorted_1005/w8v.h"
#include "../sim/unsorted_1006/w8v.h"
#include "../sim/unsorted_1006/w8w.h"
#include "../sim/unsorted_1007/w8w.h"
#include "../sim/unsorted_1008/w8w.h"
#include "../sim/unsorted_1008/w8x.h"
#include "../sim/unsorted_1009/w8x.h"
#include "../sim/unsorted_100a/w8x.h"
#include "../sim/unsorted_100a/w8y.h"
#include "../sim/unsorted_100b/w8y.h"
#include "../sim/render/w31.h"
#include "../sim/fx/w32.h"
#include "../sim/unsorted_100c/w8z.h"
#include "../sim/unsorted_100d/w8z.h"
#include "../sim/unsorted_100e/w8z.h"
#include "../sim/unsorted_100f/w8A.h"
#include "../sim/unsorted_100f/w8z.h"
#include "../sim/unsorted_1010/w8A.h"
#include "../sim/unsorted_1000/w9a.h"
#include "../sim/unsorted_1002/w9a.h"
#include "../sim/unsorted_1007/w9a.h"
#include "../sim/unsorted_100a/w9a.h"
#include "../sim/unsorted_1001/w9b.h"
#include "../sim/unsorted_1003/w9b.h"
#include "../sim/unsorted_1004/w9b.h"
#include "../sim/unsorted_1005/w9b.h"
#include "../sim/unsorted_1006/w9b.h"
#include "../sim/unsorted_1008/w9b.h"
#include "../sim/unsorted_100c/w9b.h"
#include "../sim/unsorted_1002/w9c.h"
#include "../sim/unsorted_1003/w9c.h"
#include "../sim/unsorted_1005/w9c.h"
#include "../sim/unsorted_1006/w9c.h"
#include "../sim/unsorted_1007/w9c.h"
#include "../sim/unsorted_1009/w9c.h"
#include "../sim/unsorted_100b/w9c.h"
#include "../sim/unsorted_100c/w9c.h"
#include "../sim/unsorted_100d/w9c.h"
#include "../sim/unsorted_1001/w9d.h"
#include "../sim/unsorted_1002/w9d.h"
#include "../sim/unsorted_1003/w9d.h"
#include "../sim/unsorted_1004/w9d.h"
#include "../sim/unsorted_1006/w9d.h"
#include "../sim/unsorted_1009/w9d.h"
#include "../sim/unsorted_100b/w9d.h"
#include "../sim/unsorted_100e/w9d.h"
#include "../sim/unsorted_1001/w9e.h"
#include "../sim/unsorted_1003/w9e.h"
#include "../sim/unsorted_1005/w9e.h"
#include "../sim/unsorted_1007/w9e.h"
#include "../sim/unsorted_1009/w9e.h"
#include "../sim/unsorted_100a/w9e.h"
#include "../sim/unsorted_100b/w9e.h"
#include "../sim/unsorted_100c/w9e.h"
#include "../sim/unsorted_100d/w9e.h"
#include "../sim/unsorted_1001/w9f.h"
#include "../sim/unsorted_1003/w9f.h"
#include "../sim/unsorted_1004/w9f.h"
#include "../sim/unsorted_1005/w9f.h"
#include "../sim/unsorted_1007/w9f.h"
#include "../sim/unsorted_100a/w9f.h"
#include "../sim/unsorted_100b/w9f.h"
#include "../sim/unsorted_1001/w9g.h"
#include "../sim/unsorted_1002/w9g.h"
#include "../sim/unsorted_1004/w9g.h"
#include "../sim/unsorted_1005/w9g.h"
#include "../sim/unsorted_1006/w9g.h"
#include "../sim/unsorted_1007/w9g.h"
#include "../sim/unsorted_1008/w9g.h"
#include "../sim/unsorted_100c/w9g.h"
#include "../sim/unsorted_1001/w9h.h"
#include "../sim/unsorted_1002/w9h.h"
#include "../sim/unsorted_1003/w9h.h"
#include "../sim/unsorted_1004/w9h.h"
#include "../sim/unsorted_1005/w9h.h"
#include "../sim/unsorted_1008/w9h.h"
#include "../sim/unsorted_1009/w9h.h"
#include "../sim/unsorted_100b/w9h.h"
#include "../sim/unsorted_100e/w9h.h"
#include "../sim/unsorted_1005/w10.h"
#include "../sim/unsorted_1000/w10.h"
#include "../sim/unsorted_1001/w10.h"
#include "../sim/unsorted_1009/w10.h"
#include "../sim/unsorted_1008/w10.h"
#include "../sim/unsorted_1006/w10.h"
#include "../sim/unsorted_100c/w10.h"
#include "../sim/unsorted_100d/w10.h"
#include "../sim/unsorted_1004/w10.h"
#include "../sim/unsorted_1003/w10.h"
#include "../sim/unsorted_100a/w10.h"
#include "../sim/unsorted_1002/w10.h"
#include "../sim/unsorted_1002/w26.h"
#include "../sim/unsorted_1005/w27.h"
#include "../sim/platform/w28.h"
#include "../sim/unsorted_1002/w29.h"
#include "../sim/render/w30.h"
#include "../sim/gfx/w37.h"
#include "../sim/unsorted_1007/w10.h"
#include "../sim/unsorted_1007/w10d.h"
#include "../sim/unsorted_100c/w10d.h"
#include "../sim/unsorted_100b/w10.h"
#include "../sim/util/w10_interp.h"
#include "../sim/unsorted_1003/w11.h"
#include "../sim/unsorted_1007/w11.h"
#include "../sim/unsorted_1005/w11.h"
#include "../sim/unsorted_1008/w11.h"
#include "../sim/unsorted_100b/w11.h"
#include "../sim/unsorted_100b/w12.h"
#include "../sim/unsorted_1001/w12.h"
#include "../sim/unsorted_100c/w12.h"
#include "../sim/unsorted_100a/w12.h"
#include "../sim/unsorted_1006/w12.h"
#include "../sim/unsorted_1005/w12.h"
#include "../sim/unsorted_100e/w12.h"
#include "../sim/unsorted_1001/w13.h"
#include "../sim/unsorted_100b/w13.h"
#include "../sim/unsorted_1007/w13.h"
#include "../sim/unsorted_1006/w13.h"
#include "../sim/unsorted_1004/w14.h"
#include "../sim/unsorted_1007/w14.h"
#include "../sim/unsorted_100b/w14.h"
#include "../sim/unsorted_100d/w14.h"
#include "../sim/unsorted_1005/w14.h"
#include "../sim/unsorted_1009/w14.h"
#include "../sim/unsorted_100d/w15.h"
#include "../sim/unsorted_1006/w16.h"
#include "../sim/unsorted_1006/w17.h"
#include "../sim/unsorted_1002/w18.h"
#include "../sim/render/w18.h"
#include "../sim/unsorted_1005/w18.h"
#include "../sim/unsorted_1008/w19.h"
#include "../sim/unsorted_1004/w20.h"
#include "../sim/unsorted_1008/w21.h"
#include "../sim/render/w22.h"
#include "../sim/render/w23.h"
#include "../sim/unsorted_1001/w23.h"
#include "../sim/unsorted_1007/w23.h"
#include "../sim/unsorted_100b/w24.h"
#include "../sim/unsorted_1000/w24.h"
#include "../sim/unsorted_1007/w24.h"
#include "hook.h"

#define CONV(rva_, name_, ...) static const unsigned char name_##_bytes[] = {__VA_ARGS__};
#include "conv_1003.inc"
#include "conv_1004.inc"
#include "conv_100d.inc"
#include "conv_util.inc"
#include "conv_view.inc"
#include "conv_w2a.inc"
#include "conv_w2b.inc"
#include "conv_w2c.inc"
#include "conv_w2d.inc"
#include "conv_w2e.inc"
#include "conv_w2f.inc"
#include "conv_w2g.inc"
#include "conv_w2h.inc"
#include "conv_w2i.inc"
#include "conv_w2j.inc"
#include "conv_w2k.inc"
#include "conv_w2l.inc"
#include "conv_w2m.inc"
#include "conv_w2n.inc"
#include "conv_w5x.inc"
#include "conv_w5y.inc"
#include "conv_w5z.inc"
#include "conv_w5a.inc"
#include "conv_w5b.inc"
#include "conv_w5c.inc"
#include "conv_w4u.inc"
#include "conv_w4v.inc"
#include "conv_w4w.inc"
#include "conv_w3p.inc"
#include "conv_w3o.inc"
#include "conv_w3q.inc"
#include "conv_w3r.inc"
#include "conv_w3s.inc"
#include "conv_w3t.inc"
#include "conv_w4a.inc"
#include "conv_w6d.inc"
#include "conv_w6e.inc"
#include "conv_w6f.inc"
#include "conv_w6g.inc"
#include "conv_w6h.inc"
#include "conv_w6i.inc"
#include "conv_w6j.inc"
#include "conv_w6k.inc"
#include "conv_w7l.inc"
#include "conv_w7m.inc"
#include "conv_w7n.inc"
#include "conv_w7o.inc"
#include "conv_w7p.inc"
#include "conv_w7q.inc"
#include "conv_w7r.inc"
#include "conv_w7s.inc"
#include "conv_w8t.inc"
#include "conv_w8u.inc"
#include "conv_w8v.inc"
#include "conv_w8w.inc"
#include "conv_w8x.inc"
#include "conv_w8y.inc"
#include "conv_w8z.inc"
#include "conv_w8A.inc"
#include "conv_w9a.inc"
#include "conv_w9b.inc"
#include "conv_w9c.inc"
#include "conv_w9d.inc"
#include "conv_w9e.inc"
#include "conv_w9f.inc"
#include "conv_w9g.inc"
#include "conv_w9h.inc"
#include "conv_w10.inc"
#include "conv_w12.inc"
#include "conv_w13.inc"
#include "conv_w14.inc"
#include "conv_w15.inc"
#include "conv_w16.inc"
#include "conv_w17.inc"
#include "conv_w18.inc"
#include "conv_w19.inc"
#include "conv_w20.inc"
#include "conv_w21.inc"
#include "conv_w22.inc"
#include "conv_w23.inc"
#include "conv_w24.inc"
#include "conv_w25.inc"
#include "conv_w26.inc"
#include "conv_w27.inc"
#include "conv_w28.inc"
#include "conv_w28_all.inc"
#include "conv_w29.inc"
#include "conv_w30.inc"
#include "conv_w32.inc"
#include "conv_w37.inc"
#undef CONV

#define CONV(rva_, name_, ...) \
    {.module = "HGSIM.DLL", .rva = (rva_), .name = #name_, .expect = name_##_bytes, \
     .stolen = sizeof(name_##_bytes), .replacement = (void *)(name_)},
static const HookDesc g_convHooks[] = {
#include "conv_1003.inc"
#include "conv_1004.inc"
#include "conv_100d.inc"
#include "conv_util.inc"
#include "conv_view.inc"
#include "conv_w2a.inc"
#include "conv_w2b.inc"
#include "conv_w2c.inc"
#include "conv_w2d.inc"
#include "conv_w2e.inc"
#include "conv_w2f.inc"
#include "conv_w2g.inc"
#include "conv_w2h.inc"
#include "conv_w2i.inc"
#include "conv_w2j.inc"
#include "conv_w2k.inc"
#include "conv_w2l.inc"
#include "conv_w2m.inc"
#include "conv_w2n.inc"
#include "conv_w5x.inc"
#include "conv_w5y.inc"
#include "conv_w5z.inc"
#include "conv_w5a.inc"
#include "conv_w5b.inc"
#include "conv_w5c.inc"
#include "conv_w4u.inc"
#include "conv_w4v.inc"
#include "conv_w4w.inc"
#include "conv_w3p.inc"
#include "conv_w3o.inc"
#include "conv_w3q.inc"
#include "conv_w3r.inc"
#include "conv_w3s.inc"
#include "conv_w3t.inc"
#include "conv_w4a.inc"
#include "conv_w6d.inc"
#include "conv_w6e.inc"
#include "conv_w6f.inc"
#include "conv_w6g.inc"
#include "conv_w6h.inc"
#include "conv_w6i.inc"
#include "conv_w6j.inc"
#include "conv_w6k.inc"
#include "conv_w7l.inc"
#include "conv_w7m.inc"
#include "conv_w7n.inc"
#include "conv_w7o.inc"
#include "conv_w7p.inc"
#include "conv_w7q.inc"
#include "conv_w7r.inc"
#include "conv_w7s.inc"
#include "conv_w8t.inc"
#include "conv_w8u.inc"
#include "conv_w8v.inc"
#include "conv_w8w.inc"
#include "conv_w8x.inc"
#include "conv_w8y.inc"
#include "conv_w8z.inc"
#include "conv_w8A.inc"
#include "conv_w9a.inc"
#include "conv_w9b.inc"
#include "conv_w9c.inc"
#include "conv_w9d.inc"
#include "conv_w9e.inc"
#include "conv_w9f.inc"
#include "conv_w9g.inc"
#include "conv_w9h.inc"
#include "conv_w10.inc"
#include "conv_w12.inc"
#include "conv_w13.inc"
#include "conv_w14.inc"
#include "conv_w15.inc"
#include "conv_w16.inc"
#include "conv_w17.inc"
#include "conv_w18.inc"
#include "conv_w19.inc"
#include "conv_w20.inc"
#include "conv_w21.inc"
#include "conv_w22.inc"
#include "conv_w23.inc"
#include "conv_w24.inc"
#include "conv_w25.inc"
#include "conv_w26.inc"
#include "conv_w27.inc"
#include "conv_w28.inc"
#include "conv_w28_all.inc"
#include "conv_w29.inc"
#include "conv_w30.inc"
#include "conv_w32.inc"
#include "conv_w37.inc"
};
#undef CONV

/* The .inc file each entry of g_convHooks came from, for the "convoff=" switch. */
#define CONV(rva_, name_, ...) __FILE__,
static const char *const g_convFiles[] = {
#include "conv_1003.inc"
#include "conv_1004.inc"
#include "conv_100d.inc"
#include "conv_util.inc"
#include "conv_view.inc"
#include "conv_w2a.inc"
#include "conv_w2b.inc"
#include "conv_w2c.inc"
#include "conv_w2d.inc"
#include "conv_w2e.inc"
#include "conv_w2f.inc"
#include "conv_w2g.inc"
#include "conv_w2h.inc"
#include "conv_w2i.inc"
#include "conv_w2j.inc"
#include "conv_w2k.inc"
#include "conv_w2l.inc"
#include "conv_w2m.inc"
#include "conv_w2n.inc"
#include "conv_w5x.inc"
#include "conv_w5y.inc"
#include "conv_w5z.inc"
#include "conv_w5a.inc"
#include "conv_w5b.inc"
#include "conv_w5c.inc"
#include "conv_w4u.inc"
#include "conv_w4v.inc"
#include "conv_w4w.inc"
#include "conv_w3p.inc"
#include "conv_w3o.inc"
#include "conv_w3q.inc"
#include "conv_w3r.inc"
#include "conv_w3s.inc"
#include "conv_w3t.inc"
#include "conv_w4a.inc"
#include "conv_w6d.inc"
#include "conv_w6e.inc"
#include "conv_w6f.inc"
#include "conv_w6g.inc"
#include "conv_w6h.inc"
#include "conv_w6i.inc"
#include "conv_w6j.inc"
#include "conv_w6k.inc"
#include "conv_w7l.inc"
#include "conv_w7m.inc"
#include "conv_w7n.inc"
#include "conv_w7o.inc"
#include "conv_w7p.inc"
#include "conv_w7q.inc"
#include "conv_w7r.inc"
#include "conv_w7s.inc"
#include "conv_w8t.inc"
#include "conv_w8u.inc"
#include "conv_w8v.inc"
#include "conv_w8w.inc"
#include "conv_w8x.inc"
#include "conv_w8y.inc"
#include "conv_w8z.inc"
#include "conv_w8A.inc"
#include "conv_w9a.inc"
#include "conv_w9b.inc"
#include "conv_w9c.inc"
#include "conv_w9d.inc"
#include "conv_w9e.inc"
#include "conv_w9f.inc"
#include "conv_w9g.inc"
#include "conv_w9h.inc"
#include "conv_w10.inc"
#include "conv_w12.inc"
#include "conv_w13.inc"
#include "conv_w14.inc"
#include "conv_w15.inc"
#include "conv_w16.inc"
#include "conv_w17.inc"
#include "conv_w18.inc"
#include "conv_w19.inc"
#include "conv_w20.inc"
#include "conv_w21.inc"
#include "conv_w22.inc"
#include "conv_w23.inc"
#include "conv_w24.inc"
#include "conv_w25.inc"
#include "conv_w26.inc"
#include "conv_w27.inc"
#include "conv_w28.inc"
#include "conv_w28_all.inc"
#include "conv_w29.inc"
#include "conv_w30.inc"
#include "conv_w32.inc"
#include "conv_w37.inc"
};
#undef CONV

/*
 * Sound trace ("soundtrace" in hgrc.cfg): logs what the sound start function
 * 0x1000e0d9 and the resource loader 0x100ba7d5 return. Diagnostic only; both
 * run the original code unchanged.
 */
static int(__cdecl *g_origSoundStart)(int, int, unsigned, int, int, int, int);
static int(__cdecl *g_origLoadResource)(char *, char *, DWORD *);
static int g_soundStartLogged, g_loadLogged;

// FUNCTION: HGSIM 0x1000e0d9
static int __cdecl TraceSoundStart(int id, int data, unsigned flags, int a4, int a5, int kind, int a7)
{
    int r = g_origSoundStart(id, data, flags, a4, a5, kind, a7);

    if (g_soundStartLogged < 60) {
        g_soundStartLogged++;
        DebugPrintf("soundtrace: start(id %d, data %08x, flags %04x, kind %d) = %d\n", id,
                    (unsigned)data, flags & 0xffff, kind, r);
    }
    return r;
}

// FUNCTION: HGSIM 0x100ba7d5
static int __cdecl TraceLoadResource(char *name, char *ext, DWORD *size)
{
    char n[40], e[12];
    int r;

    _snprintf(n, sizeof(n), "%s", name != NULL ? name : "(null)");
    n[sizeof(n) - 1] = 0;
    _snprintf(e, sizeof(e), "%s", ext != NULL ? ext : "(null)");
    e[sizeof(e) - 1] = 0;
    r = g_origLoadResource(name, ext, size);
    if (g_loadLogged < 120 && (r == 0 || _stricmp(e, "wav") == 0)) {
        g_loadLogged++;
        DebugPrintf("soundtrace: load(%s, %s) = %08x size %u\n", n, e, (unsigned)r,
                    size != NULL ? (unsigned)*size : 0u);
    }
    return r;
}

static const unsigned char kPrologueSoundStart[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x38};
static const unsigned char kPrologueLoadResource[] = {0x55, 0x8B, 0xEC, 0x81, 0xEC, 0x14, 0x01, 0x00, 0x00};

static const HookDesc g_traceHooks[] = {
    {.module = "HGSIM.DLL", .rva = 0xE0D9, .name = "SoundStart (trace)", .expect = kPrologueSoundStart,
     .stolen = sizeof(kPrologueSoundStart), .replacement = (void *)TraceSoundStart,
     .original = (void **)&g_origSoundStart},
    {.module = "HGSIM.DLL", .rva = 0xBA7D5, .name = "LoadResource (trace)", .expect = kPrologueLoadResource,
     .stolen = sizeof(kPrologueLoadResource), .replacement = (void *)TraceLoadResource,
     .original = (void **)&g_origLoadResource},
};

/*
 * x87 control word log (diagnostic, never changes it): every float proof assumes
 * 53-bit precision. Logged once when the sim loads and on the 1st and 2000th call
 * of the per-frame sound update 0x10039c00 (a tap: the original runs unchanged).
 */
static void LogFpuControlWord(const char *when)
{
    unsigned short cw;
    static const char *const kPrecision[4] = {"24", "reserved", "53", "64"};
    static const char *const kRounding[4] = {"nearest", "down", "up", "toward zero"};

#if defined(_MSC_VER)
    __asm fnstcw cw
#else
    __asm__ volatile("fnstcw %0" : "=m"(cw));
#endif
    DebugPrintf("FPU control word %s: 0x%04x (precision %s bits, rounding %s)\n", when, cw,
                kPrecision[(cw >> 8) & 3], kRounding[(cw >> 10) & 3]);
}

static int g_frameTapCalls;

static void __cdecl TapFrameFpu(const DWORD *args)
{
    (void)args;
    g_frameTapCalls++;
    if (g_frameTapCalls == 1)
        LogFpuControlWord("in the first frame");
    else if (g_frameTapCalls == 2000)
        LogFpuControlWord("after 2000 frames");
}

static const unsigned char kPrologueSoundUpdate[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x14};

static const HookDesc g_fpuLogHooks[] = {
    {.module = "HGSIM.DLL", .rva = 0x39C00, .name = "SoundUpdate (FPU log tap)",
     .expect = kPrologueSoundUpdate, .stolen = sizeof(kPrologueSoundUpdate), .tap = TapFrameFpu},
};

/*
 * hgrc.cfg words: "noconv" installs none of these hooks; "convoff=TEXT" (repeatable)
 * leaves out every hook from a conv_*.inc whose file name contains TEXT, for example
 * convoff=conv_w8 or convoff=w7m; "soundtrace" adds the sound trace above. The same
 * words are also read from hgrc_extra.cfg, which dev.cmd does not rewrite. With SDL3 sound (the default, see
 * HooksAudio_SdlSound) hooks_audio.c installs HGSIM's sound functions with SDL3 (sim/sound/sdl_sound.c), so the CONV entries of those
 * addresses, the sound start trace (0xE0D9) and the FPU log tap (0x39C00) are left out here.
 */
void HooksConv_OnSimExit(void)
{
    if (g_w31Tex)
        DebugPrintf("HGSIM.DLL: textures this mission: %u .TEX, %u .XEL fallback, %u not found\n",
                    (unsigned)g_w31TexLoaded, (unsigned)g_w31TexFallback, (unsigned)g_w31TexMissing);
}

void HooksConv_OnSimLoaded(HMODULE sim)
{
    enum { MAX_OFF = 16 };
    static const char *const kFiles[] = {"hgrc.cfg", "hgrc_extra.cfg"};
    const size_t total = sizeof(g_convHooks) / sizeof(g_convHooks[0]);
    FILE *fp;
    char word[64], off[MAX_OFF][56];
    static const DWORD kSdlSoundRvas[] = {0x39708, 0x397E7, 0x39931, 0x3A548};
    int nOff = 0, noconv = 0, trace = 0, sdlaudio = 0, nSdl = 0, nPlat = 0, sdlplat, count;
    HookDesc *use;
    size_t i, n = 0, f;
    int k;

    for (f = 0; f < sizeof(kFiles) / sizeof(kFiles[0]); f++) {
        fp = fopen(kFiles[f], "r");
        while (fp != NULL && fscanf(fp, "%63s", word) == 1) {
            if (_stricmp(word, "noconv") == 0)
                noconv = 1;
            else if (_stricmp(word, "soundtrace") == 0)
                trace = 1;
            else if (_strnicmp(word, "convoff=", 8) == 0 && word[8] != 0 && nOff < MAX_OFF) {
                strncpy(off[nOff], word + 8, sizeof(off[0]) - 1);
                off[nOff][sizeof(off[0]) - 1] = 0;
                nOff++;
            }
        }
        if (fp != NULL)
            fclose(fp);
    }
    sdlaudio = HooksAudio_SdlSound();   /* the one decision hooks_audio.c uses too */
    sdlplat = Hooks_SdlPlatform();      /* round 28: conv_w28.inc only with the SDL3 platform */
    g_w8yPatch = !noconv && Hooks_Renderer12(); /* 1.2's game: Vulkan, and Direct3D on 1.2's dx5draw (round 33) */
    g_w31Tex = g_w8yPatch;              /* round 31: 1.2's .TEX textures (in 3D mode), particles and the rest */
    g_w31TexLoaded = g_w31TexFallback = g_w31TexMissing = 0;
    DebugPrintf("HGSIM.DLL: 1.2 game (Patch folder, .TEX textures, particles) %s\n", g_w8yPatch ? "on" : "off");
    LogFpuControlWord("at sim load");
    g_frameTapCalls = 0;
    if (!sdlaudio)
        Hook_InstallModule(sim, "HGSIM.DLL", g_fpuLogHooks, sizeof(g_fpuLogHooks) / sizeof(g_fpuLogHooks[0]));
    if (trace) {
        /* with sdlaudio only the resource loader trace: 0xE0D9 is SndSdl_1000e0d9 then */
        count = Hook_InstallModule(sim, "HGSIM.DLL", g_traceHooks + (sdlaudio ? 1 : 0),
                                   sizeof(g_traceHooks) / sizeof(g_traceHooks[0]) - (sdlaudio ? 1 : 0));
        DebugPrintf("HGSIM.DLL: %d sound trace hook(s) installed\n", count);
    }
    if (noconv)
        return;
    W2k_FfTable12();                    /* the force-feedback table in 1.2's form, for 1.2's FFORCE.DLL */
    use = (HookDesc *)malloc(total * sizeof(HookDesc));
    if (use == NULL)
        return;
    for (i = 0; i < total; i++) {
        int skip = 0;

        for (k = 0; k < nOff && !skip; k++)
            skip = strstr(g_convFiles[i], off[k]) != NULL;
        for (k = 0; sdlaudio && !skip && k < (int)(sizeof(kSdlSoundRvas) / sizeof(kSdlSoundRvas[0])); k++)
            if (g_convHooks[i].rva == kSdlSoundRvas[k]) {
                skip = 1;
                nSdl++;
            }
        if (!skip && !sdlplat && strstr(g_convFiles[i], "conv_w28.inc") != NULL) {
            skip = 1;
            nPlat++;
        }
        if (!skip)
            use[n++] = g_convHooks[i];
    }
    for (k = 0; k < nOff; k++)
        DebugPrintf("convoff=%s\n", off[k]);
    count = Hook_InstallModule(sim, "HGSIM.DLL", use, n);
    g_simLiveBrightness = 1;   /* round 25: the brightness slider also rebuilds the 3D texture palette */
    DebugPrintf("HGSIM.DLL: %d of %d converted helper function(s) installed\n", count, (int)n);
    if (nSdl != 0)
        DebugPrintf("HGSIM.DLL: %d converted helper function(s) left to the SDL3 sound (sdlaudio)\n", nSdl);
    if (nPlat != 0)
        DebugPrintf("HGSIM.DLL: %d platform function(s) left to the original Windows code (no SDL3 platform)\n", nPlat);
    if (n + (size_t)nSdl + (size_t)nPlat != total)
        DebugPrintf("HGSIM.DLL: %d converted helper function(s) left out by convoff\n",
                    (int)(total - n - (size_t)nSdl - (size_t)nPlat));
    free(use);
}
