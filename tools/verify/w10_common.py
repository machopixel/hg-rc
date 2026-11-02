# Shared helpers for the round 10 specs (one spec per function, spec_w10_<addr>.py). A spec does
#     exec(open('tools/verify/w10_common.py').read())
# and then uses closure(), helper()/begin() and the small memory helpers below.
#
# closure(files): SOURCES = the given .c files plus every src/sim .c file needed to link them (found from
# the symbols each file defines and uses; the object files are cached in build/w10cache, keyed on mtime).
#
# Stubs: helper(addr, cname, nargs) replaces the original function at addr (and the C function cname, if
# the C calls a converted one by name) by a stub that logs its arguments and answers from rets(). A pointer
# into the stack is logged as 'stack'; texts can be logged by content (sargs) and records by their bytes
# (deref) so that the WHOLE object a callee reads is compared, not just its address (round 9 lesson).
import os, re, struct, subprocess, json, glob
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EAX, UC_X86_REG_EIP, UC_X86_REG_FPCW,
                               UC_X86_REG_FPSW, UC_X86_REG_FPTAG)

W10_SRC = os.path.abspath('src')
W10_CACHE = os.path.abspath(os.path.join('build', 'w10cache'))
STACK_LO, STACK_HI = 0x00700000, 0x00800000
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']
_SKIP_DIRS = ('hooks', 'launcher', 'platform', 'plugins')


def _sim_files():
    out = []
    for f in glob.glob(os.path.join(W10_SRC, 'sim', '**', '*.c'), recursive=True):
        rel = os.path.relpath(f, W10_SRC).replace('\\', '/')
        out.append(rel)
    return sorted(out)


_NEWEST_H = []


def _newest_header():
    if not _NEWEST_H:
        hs = glob.glob(os.path.join(W10_SRC, 'sim', '**', '*.h'), recursive=True)
        _NEWEST_H.append(max(os.path.getmtime(h) for h in hs))
    return _NEWEST_H[0]


def _symbols(rel):
    """(defined, undefined) symbols of one source file, cached."""
    os.makedirs(W10_CACHE, exist_ok=True)
    src = os.path.join(W10_SRC, rel)
    key = rel.replace('/', '__')
    meta = os.path.join(W10_CACHE, key + '.json')
    # a header change (e.g. a SIM_FN macro switched to a direct call) changes the symbols: newest header counts too
    mt = max(os.path.getmtime(src), _newest_header())
    if os.path.exists(meta):
        d = json.load(open(meta))
        if d['mtime'] == mt:
            return set(d['def']), set(d['und'])
    obj = os.path.join(W10_CACHE, key + '.o')
    r = subprocess.run(['i686-w64-mingw32-gcc', '-O1', '-ffreestanding', '-fno-builtin', '-w', '-c',
                        '-I', W10_SRC, '-I', os.path.join(W10_SRC, 'sim', 'mem'),
                        '-I', os.path.join(W10_SRC, 'sim', 'core'),
                        '-I', os.path.join(W10_SRC, '..', 'third_party', 'openlibm', 'hg'),
                        '-I', os.path.join(W10_SRC, '..', 'third_party', 'openlibm', 'include')] + CFLAGS + [src, '-o', obj],
                       capture_output=True, text=True)
    if r.returncode != 0:
        d = {'mtime': mt, 'def': [], 'und': [], 'err': r.stderr[:2000]}
    else:
        nm = subprocess.check_output(['i686-w64-mingw32-nm', obj]).decode()
        dfn, und = set(), set()
        for line in nm.splitlines():
            p = line.split()
            if len(p) == 2 and p[0] == 'U':
                und.add(p[1])
            elif len(p) == 3 and p[1] in 'TDBRC':
                dfn.add(p[2])
        d = {'mtime': mt, 'def': sorted(dfn), 'und': sorted(und)}
    json.dump(d, open(meta, 'w'))
    return set(d['def']), set(d['und'])


def _rt_defs():
    obj = os.path.join(W10_CACHE, 'rt_%d.o' % os.getpid())     # per process: parallel runs (run_specs.py) never share it
    os.makedirs(W10_CACHE, exist_ok=True)
    subprocess.check_call(['i686-w64-mingw32-gcc', '-O1', '-ffreestanding', '-fno-builtin', '-w', '-c', '-I', W10_SRC,
                           os.path.join('tools', 'verify', 'rt.c'), '-o', obj])
    nm = subprocess.check_output(['i686-w64-mingw32-nm', obj]).decode()
    os.remove(obj)
    return {p[2] for p in (l.split() for l in nm.splitlines()) if len(p) == 3 and p[1] in 'TDBRC'}


def closure(files):
    files = [f.replace('\\', '/') for f in files]
    files = [f for f in files if os.path.exists(os.path.join(W10_SRC, f))]     # deleted (dropped features)
    defs = {}
    for rel in _sim_files():
        if rel.endswith('_test.c') or '/w9e_shim' in rel:
            continue
        dfn, _ = _symbols(rel)
        for s in dfn:
            defs.setdefault(s, rel)
    rt = {'_memset', '_memcpy', '_memmove', '_memcmp', '_strlen', '_strcmp', '__stricmp', '__chkstk', '___chkstk_ms'}
    rt |= _rt_defs()     # what tools/verify/rt.c already provides (g_simBase, memcpy, ...)
    out, todo = [], list(files)
    while todo:
        f = todo.pop()
        if f in out:
            continue
        out.append(f)
        _, und = _symbols(f)
        for s in und:
            if s in rt:
                continue
            if s in defs and defs[s] not in out:
                todo.append(defs[s])
            elif s.startswith('_Plat_') and '../tools/verify/plat_stub.c' not in out:
                out.append('../tools/verify/plat_stub.c')    # round 32: the platform layer's checker stand-in
    return out


# ---- memory helpers -------------------------------------------------------------------------------
def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def w8(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, bytes(v & 0xff for v in vals))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def wf(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%df' % len(vals), *vals))


def r32(ctx, addr):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(addr, 4)))[0]


def ur32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def ustr(uc, addr, limit=400):
    out = b''
    while len(out) < limit:
        try:
            c = bytes(uc.mem_read(addr + len(out), 1))
        except Exception:
            return ('bad', addr)
        if c == b'\0':
            break
        out += c
    return out


def rfloat(r):
    """A float32 bit pattern: mostly ordinary values, sometimes zeros, tiny, huge, inf or NaN."""
    x = r.random()
    if x < 0.04:
        return r.choice([0, 0x80000000, 0x7f800000, 0xff800000, 0x7fc00000, 0x00000001, 0x3f800000, 0xbf800000])
    if x < 0.7:
        return struct.unpack('<I', struct.pack('<f', r.uniform(-2000.0, 2000.0)))[0]
    if x < 0.85:
        return struct.unpack('<I', struct.pack('<f', r.uniform(-2.0, 2.0)))[0]
    return r.getrandbits(32)


# ---- stubs ------------------------------------------------------------------------------------------
HELPERS = {}
SARGS = {}      # addr -> set of argument indexes logged as text
DEREF = {}      # addr -> {argument index: number of bytes logged}
EFFECT = {}     # addr -> function(uc, call number, raw args, eax) -> None | eax | (extra log, eax or None)
MASK = {}       # (addr, index) -> mask
STDCALL = set() # stub addresses that pop their own arguments (COM methods called through a table)
ST = {}


def helper(addr, cname, nargs, sargs=(), deref=None):
    HELPERS[addr] = (cname, nargs)
    if sargs:
        SARGS[addr] = set(sargs)
    if deref:
        DEREF[addr] = dict(deref)


def _log_args(uc, addr, nargs, esp):
    raw = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
    args = []
    for k, v in enumerate(raw):
        if (addr, k) in MASK:
            v &= MASK[(addr, k)]
        if k in SARGS.get(addr, ()):
            v = ustr(uc, v) if v else None
        elif k in DEREF.get(addr, {}):
            try:
                v = ('deref', bytes(uc.mem_read(v, DEREF[addr][k])))
            except Exception:
                v = ('bad', v)
        args.append('stack' if isinstance(v, int) and STACK_LO <= v <= STACK_HI else v)
    return raw, args


def _stub_cb(m, addr):
    cname, nargs = HELPERS[addr]

    def cb(uc, address, size, ud):
        if addr not in ST.get('active', ()):
            return
        esp = uc.reg_read(UC_X86_REG_ESP)
        if ST.get('log') is not m.api_log:
            ST['log'] = m.api_log
            ST['count'] = {}
        raw, args = _log_args(uc, addr, nargs, esp)
        n = len(m.api_log)
        m.api_log.append(('stub_%x' % addr, tuple(args)))
        k = ST['count'].get(addr, 0)
        ST['count'][addr] = k + 1
        rv = ST['rets'].get(addr) or [0]
        eax = rv[n % len(rv)] & 0xffffffff
        if addr in EFFECT:
            v = EFFECT[addr](m.uc, k, raw, eax)     # the tracked handle: writes to DLL data are put back
            if isinstance(v, tuple):
                extra, v = v
                m.api_log.extend(extra)
            if v is not None:
                eax = v & 0xffffffff
        uc.reg_write(UC_X86_REG_EAX, eax)
        uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4 + (4 * nargs if addr in STDCALL else 0))
    return cb


def begin(ctx, *active):
    """First call of every generator: installs the stubs once, sets the game's x87 state (control word
    0x27f) and switches on the stubs of the helpers given for this trial."""
    m, r = ctx.m, ctx.rng
    if not getattr(m, '_w10', False):
        m._w10 = True
        for addr, (cname, nargs) in sorted(HELPERS.items()):
            cb = _stub_cb(m, addr)
            m.raw.hook_add(UC_HOOK_CODE, cb, None, addr, addr)
            if cname is not None:
                m.raw.hook_add(UC_HOOK_CODE, cb, None, m.syms[cname], m.syms[cname])
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['active'] = set(active)
    for a in active:
        assert a in HELPERS, hex(a)
    ST['count'] = {}
    ST['log'] = None
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in active}
    return r


def rets(addr, *values):
    ST['rets'][addr] = list(values)


# ---- st(0) results ---------------------------------------------------------------------------------
# The checker cannot see st(0). For a function that returns a float or double in st(0), the spec calls it
# through a trampoline (one for the original, one for the C) that stores st(0) at OUTF and returns 0:
#     dict(name=..., orig=TRAMP_O, c='w10_tramp_c', gen=g, ...)  and the generator calls  st0(ctx, orig, c, n)
OUTF = 0x00AFF000
TRAMP_O = 0x00F00800
TRAMP_C = 0x00F00900


def _tramp(target, nargs, kind):
    code = b'\x55\x89\xe5'
    for i in range(nargs - 1, -1, -1):
        code += b'\xff\x75' + bytes([8 + 4 * i])
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'
    if nargs:
        code += b'\x83\xc4' + bytes([4 * nargs])
    code += (b'\xdd\x1d' if kind == 'd' else b'\xd9\x1d') + struct.pack('<I', OUTF)   # fstp qword/dword [OUTF]
    code += b'\x31\xc0\x5d\xc3'
    return code


def st0(ctx, orig, cname, nargs, kind='f'):
    """Installs the two trampolines (once). kind 'f' stores st(0) as a float, 'd' as a double."""
    m = ctx.m
    if not getattr(m, '_w10_tramp', False):
        m._w10_tramp = True
        m.raw.mem_write(TRAMP_O, _tramp(orig, nargs, kind))
        m.raw.mem_write(TRAMP_C, _tramp(m.syms[cname], nargs, kind))
        m.syms['w10_tramp_c'] = TRAMP_C


# ---- branch coverage (optional): W10_COV=0xADDR:size prints which instructions of the original ran -----
import atexit as _atexit
_COV = os.environ.get('W10_COV')
_COV_SEEN = set()
_COV_RESULT = []


def _cov_install(m):
    if not _COV or getattr(m, '_w10_cov', False):
        return
    m._w10_cov = True
    a, n = _COV.split(':')
    a, n = int(a, 16), int(n, 0)

    def cb(uc, address, size, ud):
        _COV_SEEN.add(address)
    m.raw.hook_add(UC_HOOK_CODE, cb, None, a, a + n - 1)

    def result():
        import capstone
        md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        code = bytes(m.raw.mem_read(a, n))
        ins = [i.address for i in md.disasm(code, a)]
        missed = [hex(x) for x in ins if x not in _COV_SEEN]
        return len(ins) - len(missed), len(ins), missed
    _COV_RESULT.append(result)

    def report():
        run, total, missed = result()
        print('coverage %d/%d instructions; not run: %s' % (run, total, ' '.join(missed[:40])))
    if os.environ.get('DIFFTEST_VERBOSE'):      # quiet mode: difftest puts the coverage in its summary and log
        _atexit.register(report)


def _cov_summary():
    """(instructions run, instructions, not-run addresses) of the W10_COV range, or None without W10_COV."""
    return _COV_RESULT[0]() if _COV_RESULT else None


_begin_plain = begin


def begin(ctx, *active):
    _cov_install(ctx.m)
    return _begin_plain(ctx, *active)


# ---- float stubs: a converted helper that answers in st(0) is stubbed by a hook that keeps esp and jumps to
# "fld dword [value]; ret". FSTUB = {addr: (C name or None, number of dword args)}; the answers come from
# ST['fvals'][addr] by call number (generator sets it). Every argument dword is logged.
FCODE = 0x00F00C00
FVAL = 0x00AFF100
FSTUB = {}


def fstubs(ctx):
    m = ctx.m
    if getattr(m, '_fstub', False):
        return
    m._fstub = True
    for slot, (addr, (cname, n)) in enumerate(sorted(FSTUB.items())):
        m.raw.mem_write(FCODE + 0x10 * slot, b'\xd9\x05' + struct.pack('<I', FVAL + 4 * slot) + b'\xc3')

        def cb(uc, address, size, ud, addr=addr, n=n, slot=slot):
            if ST.get('flog') is not m.api_log:
                ST['flog'] = m.api_log
                ST['fcount'] = {}
            esp = uc.reg_read(UC_X86_REG_ESP)
            m.api_log.append(('fstub_%x' % addr, struct.unpack('<%dI' % n, bytes(uc.mem_read(esp + 4, 4 * n)))))
            k = ST['fcount'].get(addr, 0)
            ST['fcount'][addr] = k + 1
            vals = ST['fvals'][addr]
            m.uc.mem_write(FVAL + 4 * slot, struct.pack('<I', vals[k % len(vals)]))
            uc.reg_write(UC_X86_REG_EIP, FCODE + 0x10 * slot)
        m.raw.hook_add(UC_HOOK_CODE, cb, None, addr, addr)
        if cname is not None:
            m.raw.hook_add(UC_HOOK_CODE, cb, None, m.syms[cname], m.syms[cname])
        if addr in globals().get('FSTUB12', {}):        # round 33: a 1.2 spec's twin of the stubbed callee
            m.raw.hook_add(UC_HOOK_CODE, cb, None, FSTUB12[addr], FSTUB12[addr])


# ---- platform math library for the interpreted functions (DECISIONS.md): the C side's sin / cos / asin / acos /
# atan2 (tools/verify/rt.c) answer with the host's math module, a modern 64-bit libm, so the measured tolerance is
# the real difference between the original's x87 CRT and a modern library. A domain error answers NaN.
LIBM_CODE = 0x00F00D00     # "fld qword [esp - 8]; ret": the answer sits just below the stack pointer (the stack is
LIBM = {'sin': 1, 'cos': 1, 'asin': 1, 'acos': 1, 'atan2': 2}     # not compared, so it is no memory difference)


def libm(ctx):
    import math
    m = ctx.m
    if getattr(m, '_w10_libm', False):
        return
    m._w10_libm = True
    for slot, (name, n) in enumerate(sorted(LIBM.items())):
        if name not in m.syms:
            continue
        m.raw.mem_write(LIBM_CODE + 0x10 * slot, b'\xdd\x44\x24\xf8\xc3')

        def cb(uc, address, size, ud, name=name, n=n, slot=slot):
            esp = uc.reg_read(UC_X86_REG_ESP)
            a = struct.unpack('<%dd' % n, bytes(uc.mem_read(esp + 4, 8 * n)))
            try:
                v = getattr(math, name)(*a)
            except ValueError:
                v = float('nan')
            uc.mem_write(esp - 8, struct.pack('<d', v))
            uc.reg_write(UC_X86_REG_EIP, LIBM_CODE + 0x10 * slot)
        m.raw.hook_add(UC_HOOK_CODE, cb, None, m.syms[name], m.syms[name])


# ---- indirect calls -------------------------------------------------------------------------------
# A function pointer in data that the function calls (an object's method, a callback) is set by the generator
# to one of these addresses; helper(IND1, None, n) makes it a logging stub like any other.
IND1 = 0x00F00300
IND2 = 0x00F00340
IND3 = 0x00F00380
IND4 = 0x00F003C0
IND5 = 0x00F00400
IND6 = 0x00F00440
