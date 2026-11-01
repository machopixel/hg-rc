# Focus / cache round (1.2): the cache file reader Sim_100ba304 against 1.2's 0x100a26e4, on spec_w8y.py's
# generator. 1.2's change: a line with hash size 0 gets no slot table (+0x10 = 0, no allocation); the asserts give
# 1.2's source lines. Our fix (DECISIONS.md, "Cache size 0"): a size-0 line is left out (filing or finding a file
# of that type would divide by 0 in 1.2), so the C differs from 1.2 on those lines by design:
#   Sim_100ba304_v12       files without a size-0 line: the C and 1.2 must agree on everything (asserts included).
#   Sim_100ba304_v12_zero  size-0 lines: 1.2 runs on the same file with each such line rewritten (same length) to
#                          what the fix makes of it (see g12_zero); the C runs on the real lines (pre_c puts them
#                          back, post_c the rewritten ones again).
# The w8y stubs are hooked at 1.2's callee addresses too and log under the 1.0 address; the C library's globals
# the reader's callees use are paired by aligning their listings (tools/v12 method) and copied across.
# Negative check: W8Y_NEG12=1 builds the C with W8Y_MUTATE12 (1.0's assert line 0x39a for the purge list).
# The asserts before a null write (0x367, 0x37c, 0x39a) end those trials in a fault on both sides (both-fault, as in
# spec_w8y.py); their line numbers were checked by reading 1.2's listing (0x100a2705, 0x100a27c8, 0x100a28f8).
import os
HELPERS = {}
exec(open('tools/verify/spec_w8y.py').read())
exec(open('tools/verify/v12_common.py').read())
_w10 = {}
exec(open('tools/verify/w10_common.py').read(), _w10)
SOURCES = _w10['closure'](['sim/unsorted_100b/w8y_cache.c'])     # spec_w8y.py's list predates later rounds
if os.environ.get('W8Y_NEG12'):
    CFLAGS = list(CFLAGS) + ['-DW8Y_MUTATE12']

F10, F12 = 0x100ba304, 0x100a26e4


def _deep_pairs():
    """1.0 -> 1.2 for the reader and every function it reaches (listings aligned one pair at a time)"""
    seen, todo, out = {}, [(F10, F12)], {}
    while todo:
        a, b = todo.pop()
        if a in seen:
            continue
        seen[a] = b
        calls = {n for ins in _listing(V10_DLL, a) if ins[0].startswith('call') for n in ins[1]}
        for k, v in v12_pairs(a, b).items():
            out.setdefault(k, v)
            if k in calls and k not in seen and not _writable(V10_DLL, k):
                todo.append((k, v))
    return out


PAIRS = _deep_pairs()
_dup = {}
for _k, _v in PAIRS.items():
    _dup.setdefault(_v, []).append(_k)
# writable globals only, no import slots (1.2's own stay), none whose 1.2 twin is claimed twice
GLOBALS = {k: v for k, v in PAIRS.items() if _writable(V10_DLL, k) and k < 0x10ac5000 and len(_dup[v]) == 1}
STUB12 = {PAIRS[a]: a for a in CACHE_STUBS if a in PAIRS}
HEAPLIST10, HEAPLIST12 = 0x10159e38, A12(PAIRS[0x10159e38])
STRTOK10, STRTOK12 = 0x1017e884, A12(PAIRS[0x1017e884])


def _hook12(m):
    if getattr(m, '_w8y12', False):
        return
    m._w8y12 = True
    inner = _stub_cb(m)

    def cb(uc, address, size, ud):
        inner(uc, STUB12[address - BASE12 + 0x10000000], size, ud)
    for a12 in STUB12:
        m.raw.hook_add(UC_HOOK_CODE, cb, None, A12(a12), A12(a12))


def to12_more(m):
    _hook12(m)
    if ur32(m.uc, HEAPLIST10) == HEAPLIST10:        # heap_world's empty list points at itself
        m.uc.mem_write(HEAPLIST12, struct.pack('<I', HEAPLIST12))


def from12_more(m):
    if ur32(m.uc, HEAPLIST10) == HEAPLIST12:
        m.uc.mem_write(HEAPLIST10, struct.pack('<I', HEAPLIST10))


def e_fclose12(uc, n, raw, esp):
    """e_fclose for either side: strtok's position is 1.2's own global in 1.2's run"""
    for a in (STRTOK10, STRTOK12):
        p = ur32(uc, a)
        if STACK_LO <= p <= STACK_HI:
            uc.mem_write(a, struct.pack('<I', 0x00DEAD00))
            return [ustr(uc, p)], None
    return [], None


def lines_nonzero(ctx):
    r = ctx.rng
    out = []
    for _ in range(r.choice([0, 1, 2, 4, 6])):
        if r.random() < 0.1:
            out.append(b'# x\n')
            continue
        t = r.choice(TYPE_NAMES + [b'wav', b'tex'])
        parts = [t, r.choice([b'4', b'16', b'-1', b'1024', b'1023', b'7']), r.choice([b'1000', b'-1', b'65536', b'0'])]
        parts = parts[:r.choice([1, 2, 3, 3, 3])]
        out.append(r.choice([b' ', b'\t', b'  ']).join(parts) + b'\n')
    return out


def g12(ctx):
    r = cache_begin(ctx)
    ST['effects'][0x100fd760] = e_fclose12
    heap_world(ctx)
    fails(ctx, 0.03, 30)
    rets(0x100fd820, r.choice([text_stream(ctx, lines_nonzero(ctx))] * 8 + [0]))
    return [cstr(ctx, b'cache.ini')]


ZERO = {}


def g12_zero(ctx):
    """a file of 2..5 listed types, some listed again with size 0 (written '0', '00' or '0x10'), plus size-0 lines
    of new types, then a normal line. 1.2 sees, in the same number of bytes:
      a type listed before in capitals (the lookup is case-sensitive against the capitalised name, so only then is
      the record found): a comment, which 1.2 skips, as the C keeps the record as it was;
      a new type: the line cut after the size ('wav     0\n', no budget token), 1.2's missing-token path, which
      makes the record and frees it again, as the C does for the size-0 line."""
    r = cache_begin(ctx)
    ST['effects'][0x100fd760] = e_fclose12
    heap_world(ctx)
    types = r.sample(TYPE_NAMES + [b'wav', b'tex'], r.choice([2, 3, 4, 5]))
    real, shown = [], []
    for t in types:
        ln = t + b'\t' + r.choice([b'4', b'16', b'1023', b'-1']) + b'\t' + r.choice([b'1000', b'65536', b'-1']) + b'\n'
        real.append(ln)
        shown.append(ln)
    zero = [t for t in r.sample(types, r.choice(range(1, len(types) + 1))) if t.isupper()]
    zero += [r.choice([b'tex', b'pcx', b'wav', b'Wtb']) for _ in range(r.choice([0, 1, 2]))]
    r.shuffle(zero)
    for t in zero:
        size = r.choice([b'0', b'0x10', b'00'])
        ln = t + r.choice([b' ', b'\t']) + size + r.choice([b'', b' 0', b' 4000000', b'\t\t1536000']) + b'\n'
        real.append(ln)
        if t.isupper():
            shown.append(b'#' + b'-' * (len(ln) - 2) + b'\n')
        else:
            shown.append(t + b' ' * (len(ln) - len(t) - len(size) - 1) + size + b'\n')
    tail = r.choice(TYPE_NAMES) + b' 7 1000\n'
    real.append(tail)
    shown.append(tail)
    f = text_stream(ctx, shown)
    ZERO['buf'], ZERO['real'], ZERO['shown'] = r32(ctx, f), b''.join(real), b''.join(shown)
    rets(0x100fd820, f)
    return [cstr(ctx, b'cache.ini')]


def pre_zero(m):
    m.uc.mem_write(ZERO['buf'], ZERO['real'])


def post_zero(m):
    m.uc.mem_write(ZERO['buf'], ZERO['shown'])


def lm(log_o, log_c):
    """the asserts name 1.2's source path (its own string): logged as 1.0's"""
    out = []
    for e in log_o:
        if e[0] == 'stub_100ff860' and len(e[1]) > 1 and e[1][1] == PATH12:
            e = (e[0], (e[1][0], PATH10) + tuple(e[1][2:]))
        out.append(e)
    return out, log_c


PATH10, PATH12 = rb'C:\projx\Code\sim\Mem\syscache.c', rb'C:\pg\Mem\syscache.c'
WINDOWS = {HEAPLIST10: 0x18}     # the heap list head: next, prev, ..., the block range +0x10 / +0x14
_base = dict(name='Sim_100ba304', orig=F10, c='Sim_100ba304', gen=g12, trials=4000, void=True, api=HEAP_API)
SPECS = [v12_spec(_base, F12, extra_pairs=GLOBALS, windows=WINDOWS, to12_more=to12_more, from12_more=from12_more),
         v12_spec(dict(_base, gen=g12_zero, trials=3000), F12, extra_pairs=GLOBALS, windows=WINDOWS, to12_more=to12_more,
                  from12_more=from12_more, name='Sim_100ba304_v12_zero')]
SPECS[0]['log_map'] = SPECS[1]['log_map'] = lm
SPECS[1]['pre_c'] = pre_zero
SPECS[1]['post_c'] = post_zero
