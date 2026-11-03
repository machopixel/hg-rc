#!/usr/bin/env python3
"""
Runs an original HGSIM.DLL function and its C conversion in the same emulated
machine (Unicorn), on the same random inputs, and compares the return value
and every byte of writable memory afterwards.

  difftest.py HGSIM.DLL spec.py [name ...]

spec.py defines SPECS = [dict(name, orig=0x100b9748, c='Cache_StampNewer',
args=[...], trials=300, setup=optional)], see tools/verify/README.md.
The C sources are compiled with i686-w64-mingw32-gcc into one flat image.
"""
import glob, json, math, os, random, re, struct, subprocess, sys, tempfile
import pefile
from unicorn import *
from unicorn.x86_const import *

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.normpath(os.path.join(HERE, '..', '..', 'src'))
CBASE = 0x40000000
STACK_TOP = 0x00800000
STACK_SIZE = 0x00100000
ARGMEM = 0x00A00000     # argument buffers
ARGSIZE = 0x00100000
RET_ADDR = 0x00F00000
STUB_BASE = 0x00F20000    # one 16-byte stub per imported Windows function
# round 31: specs with 'orig12' run 1.2's code (GameRip_12/hgsim.dll, relocated here) in the same machine; its
# imports get stubs of their own. Its pages are restored between trials like the 1.0 DLL's, but never compared:
# a spec's 'from12' copies 1.2's results to the 1.0 / C side before the comparison (see run_spec).
BASE12 = 0x20000000
STUB_BASE12 = 0x00F40000
V12_DLL = os.environ.get('DT_V12', os.path.join(HERE, '..', '..', '..', 'GameRip_12', 'hgsim.dll'))
PAGE = 0x1000


def _defining_files(names):
    """Source files (absolute) whose cached objects define the given symbols (build/w10cache, w10_common.py)."""
    want, out = {'_' + n for n in names}, []
    for meta in glob.glob(os.path.join(SRC, '..', 'build', 'w10cache', '*.json')):
        try:
            d = json.load(open(meta))
        except ValueError:
            continue
        if want & set(d.get('def', [])):
            rel = os.path.basename(meta)[:-len('.json')].replace('__', '/')
            if not rel.endswith('_test.c') and '/w9e_shim' not in rel:
                out.append(os.path.join(SRC, rel))
    return sorted(set(out))


def build_c(sources, cflags=()):
    """Compiles the sources into one flat PE; returns (sections, symbols)."""
    # C runtime replacement (src/sim/crt, src/sim/libm): always linked, so specs with a fixed source list find it too
    # Files a spec still lists but that were deleted (dropped features, DECISIONS.md) are left out.
    sources = [f for f in sources if os.path.exists(f)]
    # A file with a checker stand-in (tools/verify/stand_in, same name: code that needs SDL3) is replaced by it.
    have = {os.path.normcase(os.path.abspath(f)) for f in sources}
    sources = list(sources) + [f for d in ('crt', 'libm') for f in sorted(glob.glob(os.path.join(SRC, 'sim', d, '*.c')))
                               if os.path.normcase(os.path.abspath(f)) not in have]
    sources = [os.path.join(HERE, 'stand_in', os.path.basename(f))
               if os.path.exists(os.path.join(HERE, 'stand_in', os.path.basename(f))) else f for f in sources]
    tmp = tempfile.mkdtemp()
    exe = os.path.join(tmp, 't.exe')
    cmd = ['i686-w64-mingw32-gcc', '-O1', '-ffreestanding', '-fno-builtin', '-nostdlib',
           '-fno-stack-protector', '-fno-asynchronous-unwind-tables', '-w', '-mno-stack-arg-probe',
           '-I', SRC, '-I', os.path.join(SRC, 'sim', 'mem'), '-I', os.path.join(SRC, 'sim', 'core'),
           # C runtime replacement: OpenLibm (src/sim/libm wrappers), our shim headers first
           '-I', os.path.join(SRC, '..', 'third_party', 'openlibm', 'hg'), '-I', os.path.join(SRC, '..', 'third_party', 'openlibm', 'include'),
           '-Wl,--image-base=0x%x' % CBASE, '-Wl,--entry=_rt_entry', '-Wl,--section-alignment=0x1000',
           '-Wl,--file-alignment=0x1000', '-Wl,--disable-dynamicbase', '-Wl,--disable-reloc-section',
           
           '-o', exe, os.path.join(HERE, 'rt.c')] + list(cflags) + sources
    open(os.path.join(tmp, 'e.c'), 'w').write('void rt_entry(void){}\n')
    cmd.append(os.path.join(tmp, 'e.c'))
    # A spec's fixed source list that later rounds outgrew (a callee moved to a new file): the files defining the
    # missing symbols are added from the object cache (build/w10cache, w10_common.py) and the link retried.
    for _ in range(40):
        r = subprocess.run(cmd, capture_output=True, text=True)
        missing = sorted(set(re.findall(r"undefined reference to `_?([A-Za-z0-9_]+)'", r.stderr)))
        if r.returncode == 0 or not missing:
            break
        add = _defining_files(missing)
        if any(n.startswith('Plat_') for n in missing):    # the platform layer's checker stand-in (round 32)
            add.append(os.path.join(HERE, 'plat_stub.c'))
        if any(n.startswith('Music_') for n in missing):   # the music folder's stand-in (no CD audio)
            add.append(os.path.join(HERE, 'music_stub.c'))
        if any(n.startswith('Force_') for n in missing):   # force feedback without a device (portable C)
            add.append(os.path.join(SRC, 'platform', 'force_none.c'))
        if any(n.startswith('Pic_') for n in missing):     # the loading-screen picture (Vulkan): link targets only
            add = [f for f in add if not f.replace('\\', '/').endswith('platform/picture_hgvk.c')]
            add.append(os.path.join(HERE, 'picture_stub.c'))
        if any(n.startswith('Unpack_') for n in missing):  # the archive unpackers (zlib): checked natively instead
            add = [f for f in add if not f.replace('\\', '/').endswith('sim/util/unpack.c')]
            add.append(os.path.join(HERE, 'unpack_stub.c'))
        add = [f for f in add if f not in cmd]
        if not add:
            break
        cmd[-1:-1] = add
    if r.returncode != 0:
        sys.stderr.write(r.stderr)
        raise subprocess.CalledProcessError(r.returncode, cmd)
    nm = subprocess.check_output(['i686-w64-mingw32-nm', exe]).decode()
    syms = {}
    for line in nm.splitlines():
        p = line.split()
        if len(p) == 3:
            syms[p[2].lstrip('_') if p[2].startswith('_') else p[2]] = int(p[0], 16)
    pe = pefile.PE(exe)
    secs = [(CBASE + s.VirtualAddress, s.get_data().ljust(max(s.Misc_VirtualSize, len(s.get_data())), b'\0'))
            for s in pe.sections]
    return secs, syms


class TrackedUc:
    """Forwards to the real Unicorn object but notes which pages the host writes to."""
    def __init__(self, uc, dirty):
        self._uc, self._dirty = uc, dirty

    def __getattr__(self, name):
        return getattr(self._uc, name)

    def mem_write(self, addr, data):
        self._dirty.update(range(addr >> 12, (addr + max(len(data), 1) - 1 >> 12) + 1))
        return self._uc.mem_write(addr, data)


class Machine:
    def __init__(self, dll, csecs, syms, dll12=None):
        pe = pefile.PE(dll)
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.image = pe.get_memory_mapped_image()
        self.syms = syms
        self.csecs = csecs
        self.raw = Uc(UC_ARCH_X86, UC_MODE_32)
        self.dirty = set()
        self.uc = TrackedUc(self.raw, self.dirty)
        uc = self.uc
        dirty = self.dirty
        def note_write(_uc, access, address, size, value, ud):
            dirty.update(range(address >> 12, (address + size - 1 >> 12) + 1))
        # stack pushes are far the most frequent writes and the stack is always copied whole
        self.raw.hook_add(UC_HOOK_MEM_WRITE, note_write, None, 0, STACK_TOP - STACK_SIZE - 1)
        self.raw.hook_add(UC_HOOK_MEM_WRITE, note_write, None, STACK_TOP + 1, 0xFFFFFFFF)
        self._idx_snap = None
        size = (pe.OPTIONAL_HEADER.SizeOfImage + PAGE - 1) & ~(PAGE - 1)
        uc.mem_map(self.base, size)
        uc.mem_write(self.base, self.image)
        self.dll_size = size
        # writable range of the dll = .data/.bss sections
        self.wr = []
        for s in pe.sections:
            if s.Characteristics & 0x80000000:
                lo = self.base + s.VirtualAddress
                hi = lo + ((max(s.Misc_VirtualSize, s.SizeOfRawData) + PAGE - 1) & ~(PAGE - 1))
                self.wr.append((lo, min(hi, self.base + size) - lo))
        for va, data in csecs:
            n = (len(data) + PAGE - 1) & ~(PAGE - 1)
            uc.mem_map(va, n)
            uc.mem_write(va, data)
            self.wr.append((va, n))
        uc.mem_map(STACK_TOP - STACK_SIZE, STACK_SIZE)
        uc.mem_map(ARGMEM, ARGSIZE)
        uc.mem_map(RET_ADDR, PAGE)
        uc.mem_write(RET_ADDR, b'\xf4')
        self.exact_fmod()
        if self.base == 0x10000000:
            self.libm_redirect(self.LIBM10)
            self.libm_redirect(self.HEAP10, flag='heap_original')
            self.libm_watch(self.MATH10)
        self.libm_watch([syms[n] for n in self.MATH_C if n in syms])
        uc.mem_write(syms['g_simBase'], struct.pack('<I', self.base))
        self.wr += [(STACK_TOP - STACK_SIZE, STACK_SIZE), (ARGMEM, ARGSIZE)]
        self.api_tables = {}
        self.install_import_stubs(pe)
        self.range12 = None
        if dll12:
            self.load12(dll12)
        self.snap0 = self.dump()
        self.dirty.clear()

    # ---- C runtime replacement (DECISIONS.md): the original C runtime's fmod (0x100fcb30) runs x87 fprem, which the
    # emulator computes inexactly for larger quotients (real fprem is exact). The C now uses OpenLibm's exact fmod, so
    # for finite x and finite, non-zero y the original's call is answered with Python's exact math.fmod (loaded into
    # st(0) by a stub in the return page, not compared); NaN / infinite / zero-divisor calls still run the original.
    # The same for 1.2's copy (0x100dacd0) when 1.2's DLL is loaded.
    FMOD10 = 0x100fcb30
    FMOD12 = 0x100dacd0     # 1.2's copy (the same code), relocated to BASE12 by load12

    def exact_fmod(self, addr=None):
        if addr is None:
            if self.base != 0x10000000:
                return
            addr = self.FMOD10
        stub, val = RET_ADDR + 0x100, RET_ADDR + 0x200
        self.raw.mem_write(stub, b'\xdd\x05' + struct.pack('<I', val) + b'\xc3')     # fld qword [val]; ret

        def cb(uc, address, size, ud):
            esp = uc.reg_read(UC_X86_REG_ESP)
            x, y = struct.unpack('<2d', bytes(uc.mem_read(esp + 4, 16)))
            if not (math.isfinite(x) and math.isfinite(y)) or y == 0.0:
                return
            uc.mem_write(val, struct.pack('<d', math.fmod(x, y)))
            uc.reg_write(UC_X86_REG_EIP, stub)
        self.raw.hook_add(UC_HOOK_CODE, cb, None, addr, addr)

    # ---- C runtime replacement: pow / asin / sin / atan of OpenLibm differ from the original's x87 routines by up to
    # one unit in the last place for some arguments (measured on the real CPU, DECISIONS.md), and the emulator's
    # fyl2x / f2xm1 / fpatan / fsin are themselves inexact. So the original's calls of these routines jump to the same
    # OpenLibm function the C calls (same arguments on the stack, result in st(0)): a spec then compares the converted
    # code around them. A spec setting LIBM_ORIGINAL = True (spec_crt_libm.py) runs the original routines instead.
    LIBM10 = {0x100fed29: 'hg_pow', 0x100fcc74: 'hg_asin', 0x100ff644: 'hg_sin', 0x10100044: 'hg_atan'}
    LIBM12 = {0x100dd2e9: 'hg_pow', 0x100dae14: 'hg_asin'}     # 1.2's copies (sin / atan: not located in 1.2)
    libm_original = False

    def libm_redirect(self, table, delta=0, flag='libm_original'):
        for a, name in table.items():
            if name not in self.syms:
                continue
            target = self.syms[name]

            def cb(uc, address, size, ud, target=target):
                if not getattr(self, flag):
                    uc.reg_write(UC_X86_REG_EIP, target)
            self.raw.hook_add(UC_HOOK_CODE, cb, None, a + delta, a + delta)

    # ---- C runtime replacement, memory: the C's malloc / free / realloc / calloc are standard C now (crt/hg_heap.c;
    # rt.c's arena here), the original's run its own heap. The original's entry points jump to the same C functions,
    # so both runs get the same blocks and a spec compares the code around them. A spec setting HEAP_ORIGINAL = True
    # runs the original heap instead.
    HEAP10 = {0x100fef30: 'Heap_Malloc', 0x100fef50: 'Heap_MallocRetry', 0x100fefa0: 'Heap_AllocBlock',
              0x100fefe0: 'Heap_FreeBlock', 0x101003e0: 'Heap_Realloc', 0x10100580: 'Heap_Calloc'}
    HEAP12 = {0x100dd4f0: 'Heap_Malloc', 0x100dd510: 'Heap_MallocRetry', 0x100dd560: 'Heap_AllocBlock',
              0x100dd5a0: 'Heap_FreeBlock', 0x100de8a0: 'Heap_Realloc', 0x100dea40: 'Heap_Calloc'}   # 1.2's copies
    heap_original = False

    # ---- The original's maths routines write errno (0x1017e888: EDOM / ERANGE for NaN, infinite or out-of-range
    # arguments) and their own scratch (0x1017e900..0x1017e97f); OpenLibm writes neither, and nothing outside the C
    # runtime reads them (DECISIONS.md, "C runtime replacement"). In a trial where a maths routine ran (either side)
    # these two are not compared.
    MATH10 = (0x100fca04, 0x100fcb30, 0x100fcb70, 0x100fcc74, 0x100fed29, 0x100ff644, 0x100ff6f4, 0x10100044)
    MATH12 = (0x100daba4, 0x100dacd0, 0x100dad10, 0x100dae14, 0x100dd2e9)
    MATH_C = ('hg_sqrt', 'hg_fmod', 'hg_fabs', 'hg_pow', 'hg_asin', 'hg_acos', 'hg_sin', 'hg_cos', 'hg_atan')
    MATH_IGNORE = ((0x1017e888, 4), (0x1017e900, 0x80))
    libm_ran = False

    def libm_watch(self, addrs):
        def cb(uc, address, size, ud):
            self.libm_ran = True
        for a in addrs:
            self.raw.hook_add(UC_HOOK_CODE, cb, None, a, a)

    # ---- Windows imports: every import slot points at a tiny stub. Calls are logged (name and
    # arguments) and compared between the original and the C run, and the stub answers with a
    # fixed, repeatable value (a spec can supply its own answers in its `api` dict).
    CDECL_IMPORTS = {'USER32.dll!wsprintfA': 4}

    def install_import_stubs(self, pe, stub_base=STUB_BASE):
        table = json.load(open(os.path.join(HERE, 'winapi_args.json')))
        if stub_base == STUB_BASE:
            self.api_log, self.api_n, self.api_handlers = [], 0, {}
        names = self.api_tables.setdefault(stub_base, [])
        if stub_base == STUB_BASE:
            self.api_names = names
        if not hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
            return
        slots = []
        for d in pe.DIRECTORY_ENTRY_IMPORT:
            for i in d.imports:
                key = d.dll.decode() + '!' + (i.name.decode() if i.name else 'ord%d' % i.ordinal)
                slots.append((i.address, key))
        self.raw.mem_map(stub_base, (16 * len(slots) + PAGE - 1) & ~(PAGE - 1))
        for k, (iat, key) in enumerate(slots):
            nbytes = table.get(key, 0)
            code = b'\xc2' + struct.pack('<H', nbytes) if nbytes else b'\xc3'
            self.raw.mem_write(stub_base + 16 * k, code)
            self.raw.mem_write(iat, struct.pack('<I', stub_base + 16 * k))
            names.append((key, (nbytes // 4) or self.CDECL_IMPORTS.get(key, 0)))
        self.raw.hook_add(UC_HOOK_CODE, self._api_hook, None, stub_base, stub_base + 16 * len(slots) - 1)

    def load12(self, dll12):
        """1.2's DLL relocated to BASE12, its writable sections tracked, its imports stubbed (round 31)."""
        pe = pefile.PE(dll12)
        pe.relocate_image(BASE12)
        image = pe.get_memory_mapped_image()
        size = (pe.OPTIONAL_HEADER.SizeOfImage + PAGE - 1) & ~(PAGE - 1)
        self.uc.mem_map(BASE12, size)
        self.uc.mem_write(BASE12, image)
        for sec in pe.sections:
            if sec.Characteristics & 0x80000000:
                lo = BASE12 + sec.VirtualAddress
                hi = lo + ((max(sec.Misc_VirtualSize, sec.SizeOfRawData) + PAGE - 1) & ~(PAGE - 1))
                self.wr.append((lo, min(hi, BASE12 + size) - lo))
        self.range12 = (BASE12, BASE12 + size)
        pe12 = pefile.PE(dll12)     # import slots at their unrelocated addresses, moved by hand
        delta = BASE12 - pe12.OPTIONAL_HEADER.ImageBase
        for d in getattr(pe12, 'DIRECTORY_ENTRY_IMPORT', []):
            for i in d.imports:
                i.address += delta
        self.install_import_stubs(pe12, STUB_BASE12)
        self.exact_fmod(BASE12 + self.FMOD12 - 0x10000000)
        self.libm_redirect(self.LIBM12, BASE12 - 0x10000000)
        self.libm_redirect(self.HEAP12, BASE12 - 0x10000000, flag='heap_original')
        self.libm_watch([a + BASE12 - 0x10000000 for a in self.MATH12])

    def _api_hook(self, uc, addr, size, ud):
        base = STUB_BASE12 if addr >= STUB_BASE12 else STUB_BASE
        key, nargs = self.api_tables[base][(addr - base) >> 4]
        esp = uc.reg_read(UC_X86_REG_ESP)
        args = []
        for k in range(nargs):
            v = struct.unpack('<I', bytes(uc.mem_read(esp + 4 + 4 * k, 4)))[0]
            # addresses of stack locals differ between the two runs and mean nothing
            args.append('stack' if STACK_TOP - STACK_SIZE <= v <= STACK_TOP else v)
        name = key.split('!')[1]
        self.api_log.append((name, tuple(args)))
        h = self.api_handlers.get(name)
        if h is None:
            val = 1
        elif h.__code__.co_argcount >= 3:
            val = h(self.api_n, args, self.uc)      # a handler may write results into emulator memory
        else:
            val = h(self.api_n, args)
        uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
        self.api_n += 1

    # ---- page-level state handling: only pages written since the last reset are touched
    FULL = (STACK_TOP - STACK_SIZE, ARGMEM)     # small regions that are always copied whole

    def _index(self):
        if self._idx_snap is self.snap0 and self._idx_n == len(self.wr):
            return
        self.pagemap = {}                       # page number -> (snapshot bytes, offset)
        self.full = []                          # (region index) copied whole every time
        for i, ((a, n), d) in enumerate(zip(self.wr, self.snap0)):
            assert a % PAGE == 0 and n % PAGE == 0, 'region not page aligned'
            if a in self.FULL:
                self.full.append(i)
                continue
            for off in range(0, n, PAGE):
                self.pagemap[(a + off) >> 12] = (d, off)
        self._idx_snap, self._idx_n = self.snap0, len(self.wr)
        self.dirty.clear()

    def page_now(self, pg):
        return bytes(self.raw.mem_read(pg << 12, PAGE))

    def page_snap(self, pg):
        d, off = self.pagemap[pg]
        return d[off:off + PAGE]

    def full_now(self):
        return [bytes(self.raw.mem_read(*self.wr[i])) for i in self.full]

    def set_full(self, blobs):
        for i, b in zip(self.full, blobs):
            self.raw.mem_write(self.wr[i][0], b)

    def reset_to_snap0(self):
        self._index()
        for pg in self.dirty:
            if pg in self.pagemap:
                self.raw.mem_write(pg << 12, self.page_snap(pg))
        for i in self.full:
            self.raw.mem_write(self.wr[i][0], self.snap0[i])
        self.dirty.clear()

    def dump(self):
        return [bytes(self.raw.mem_read(a, n)) for a, n in self.wr]

    def restore(self, snap):
        for (a, n), d in zip(self.wr, snap):
            self.raw.mem_write(a, d)
        self.dirty.clear()

    def call(self, addr, args, limit=2_000_000):
        uc = self.uc
        self.api_log, self.api_n = [], 0
        sp = STACK_TOP - 0x100
        for a in reversed(args):
            sp -= 4
            uc.mem_write(sp, struct.pack('<I', a & 0xffffffff))
        sp -= 4
        uc.mem_write(sp, struct.pack('<I', RET_ADDR))
        uc.reg_write(UC_X86_REG_ESP, sp)
        uc.reg_write(UC_X86_REG_EBP, 0)
        for r in (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI):
            uc.reg_write(r, 0x5a5a5a5a)
        try:
            uc.emu_start(addr, RET_ADDR, count=limit)
        except UcError as e:
            return ('fault', str(e))
        if uc.reg_read(UC_X86_REG_EIP) != RET_ADDR:
            return ('timeout', None)
        return ('ok', (uc.reg_read(UC_X86_REG_EAX),
                       uc.reg_read(UC_X86_REG_EBX), uc.reg_read(UC_X86_REG_ESI),
                       uc.reg_read(UC_X86_REG_EDI), uc.reg_read(UC_X86_REG_ESP) - (sp + 4)))


class Ctx:
    """Handed to an argument generator: random source and argument memory."""
    def __init__(self, m, rng):
        self.m, self.rng = m, rng
        self.next = ARGMEM
        self.bufs = []

    def buf(self, size, fill='rand'):
        a = self.next
        self.next += (size + 63) & ~15
        if fill == 'rand':
            d = bytes(self.rng.getrandbits(8) for _ in range(size))
        elif fill == 'text':
            d = bytes(self.rng.choice(b'abcXYZ019 _.\\/') for _ in range(size - 1)) + b'\0'
        elif fill == 'zero':
            d = bytes(size)
        else:
            d = fill
        self.m.uc.mem_write(a, d)
        self.bufs.append((a, size))
        return a

    def u32(self):
        r = self.rng.random()
        if r < 0.2: return self.rng.choice([0, 1, 2, 3, 0xffffffff, 0x7fffffff, 0x80000000])
        if r < 0.6: return self.rng.randrange(0, 256)
        if r < 0.8: return self.rng.randrange(0, 1 << 16)
        return self.rng.getrandbits(32)

    def s32small(self):
        return self.rng.randrange(-300, 300) & 0xffffffff


fp_dev = [0.0]
fp_worst = [0.0]


def fp_close(x, y, tol):
    """Returns y with every 32-bit word that is a float within tol of x's word replaced by x's."""
    if x == y:
        return y
    yb = bytearray(y)
    for i in range(0, len(x) - 3, 4):
        if x[i:i+4] != y[i:i+4]:
            a = struct.unpack('<f', x[i:i+4])[0]
            b = struct.unpack('<f', y[i:i+4])[0]
            # only ordinary floats on both sides: near zero (denormals, small integers read as floats) must match
            if a == a and b == b and 1e-30 < abs(a) < 1e30 and 1e-30 < abs(b) < 1e30:
                d = abs(a - b) / max(abs(a), abs(b), 1e-30)
                if d <= tol:
                    if os.environ.get('DT_FPDEBUG'):
                        print('  fp word %d: %r vs %r (%.2e)' % (i, a, b, d))
                    fp_dev[0] = max(fp_dev[0], d)
                    yb[i:i+4] = x[i:i+4]
    return bytes(yb)


def fp_log_equal(a, b, tol):
    """The call logs a and b are equal, but for 32-bit words that are floats within tol of each other (fp specs)."""
    if a == b:
        return True
    if isinstance(a, (list, tuple)) and isinstance(b, (list, tuple)):
        return len(a) == len(b) and all(fp_log_equal(x, y, tol) for x, y in zip(a, b))
    if isinstance(a, int) and isinstance(b, int) and 0 <= a < 1 << 32 and 0 <= b < 1 << 32:
        x = fp_close(struct.pack('<I', a), struct.pack('<I', b), tol)
        if x == struct.pack('<I', a):
            fp_worst[0] = max(fp_worst[0], fp_dev[0]); fp_dev[0] = 0.0
            return True
    return False


def run_spec(m, spec, seed=1):
    name = spec['name']
    rng = random.Random(seed)
    bad = 0
    both_fault = 0
    details = []
    n = spec.get('trials', 300)
    m._index()
    m.api_handlers = spec.get('api', {})
    for t in range(n):
        m.reset_to_snap0()
        m.libm_ran = False
        m.heap_original = spec.get('heap_original', m.heap_spec_original)
        ctx = Ctx(m, rng)
        args = spec['gen'](ctx) if 'gen' in spec else [g(ctx) if callable(g) else g for g in spec['args']]
        if 'skip' in spec and spec['skip'](args):     # spec 'skip'(args): inputs a recorded exception leaves out
            continue
        # round 42: the renderer is always there ("3dfx" slot, [0x1017b940] and [0x1017b944] set; the C has no
        # software / D3D side left); a spec with 'renderer_free' keeps what its generator wrote
        if not spec.get('renderer_free'):
            m.uc.mem_write(0x1017b940, struct.pack('<2I', 1, 1))
        d0 = {pg for pg in m.dirty if pg in m.pagemap}
        pre_pages = {pg: m.page_now(pg) for pg in d0}
        pre_full = m.full_now()
        m.dirty.clear()
        if 'orig12' in spec:    # round 31: 1.2's code, state mapped across by the spec
            spec['to12'](m)
            ro, vo = m.call(BASE12 + spec['orig12'] - 0x10000000, args)
            log_o = m.api_log
            spec['from12'](m)
        else:
            ro, vo = m.call(spec['orig'], args)
            log_o = m.api_log
        d1 = {pg for pg in m.dirty if pg in m.pagemap}
        after_o = {pg: m.page_now(pg) for pg in d1}
        full_o = m.full_now()
        # back to the generated state
        for pg in d1:
            m.raw.mem_write(pg << 12, pre_pages[pg] if pg in pre_pages else m.page_snap(pg))
        m.set_full(pre_full)
        m.dirty.clear()
        if 'pre_c' in spec:     # spec 'pre_c'(m): C-side setup the original has no counterpart for (round 31)
            spec['pre_c'](m)
        rc, vc = m.call(m.syms[spec['c']], args)
        if 'post_c' in spec:    # spec 'post_c'(m): C-side state brought to the original's form (round 31)
            spec['post_c'](m)
        log_c = m.api_log
        if 'log_map' in spec:   # spec 'log_map'(log_o, log_c): the two call logs brought to one form (round 31)
            log_o, log_c = spec['log_map'](log_o, log_c)
        d2 = {pg for pg in m.dirty if pg in m.pagemap}
        m.dirty.update(d0 | d1)     # every page this trial touched is put back to the snapshot before the next one
        after_c = {pg: m.page_now(pg) for pg in d2}
        full_c = m.full_now()
        if ro == rc and ro in ('fault', 'timeout'):     # both hit the instruction limit: not a pass either
            both_fault += 1
            if os.environ.get('DT_DEBUG') and both_fault == 1: print('  fault:', vo, vc)
            continue
        rm = spec.get('retmask', 0xffffffff)     # a function returning a word in ax: only ax compared
        ok = ro == rc and (ro != 'ok' or ((vo[0] & rm) == (vc[0] & rm) if not spec.get('void') else True)
                           and vo[1:4] == vc[1:4] and vo[4] == vc[4])
        why = None
        if ok and log_o != log_c and not (spec.get('fp') and fp_log_equal(log_o, log_c, spec.get('tol', 4e-7))):
            ok = False
            k = next((i for i in range(min(len(log_o), len(log_c))) if log_o[i] != log_c[i]), min(len(log_o), len(log_c)))
            why = 'Windows calls differ at call %d: orig %s, c %s' % (k, log_o[k:k + 3], log_c[k:k + 3])
        # memory state of the original after its run = its written pages, else what was there before
        def orig_page(pg):
            if pg in after_o: return after_o[pg]
            return pre_pages[pg] if pg in pre_pages else m.page_snap(pg)
        def c_page(pg):
            if pg in after_c: return after_c[pg]
            return orig_after_unwritten(pg)
        def orig_after_unwritten(pg):       # C did not touch the page: it still has the generated content
            return pre_pages[pg] if pg in pre_pages else m.page_snap(pg)
        # spec 'ignore_syms' {C symbol: size}: C-side state the original has no counterpart for (round 22 mount table)
        ignore = [(m.syms[sym], size) for sym, size in spec.get('ignore_syms', {}).items()]
        # spec 'ignore_addrs' {address: size}: original-side scratch only the OS reads (round 28 clip box)
        ignore += list(spec.get('ignore_addrs', {}).items())
        if m.libm_ran and not m.libm_original:
            ignore += list(m.MATH_IGNORE)
        for pg in sorted(d0 | d1 | d2):
            if m.range12 and m.range12[0] <= (pg << 12) < m.range12[1]:
                continue        # 1.2's own pages: from12 copied what counts to the 1.0 / C side
            x, y = orig_page(pg), c_page(pg)
            for a, size in ignore:
                lo, hi = max(a, pg << 12), min(a + size, (pg + 1) << 12)
                if lo < hi:
                    y = y[:lo - (pg << 12)] + x[lo - (pg << 12):hi - (pg << 12)] + y[hi - (pg << 12):]
            if x == y:
                continue
            if spec.get('fp') and ro == 'ok' and rc == 'ok':
                y = fp_close(x, y, spec.get('tol', 4e-7))
                fp_worst[0] = max(fp_worst[0], fp_dev[0]); fp_dev[0] = 0.0
                if x == y:
                    continue
            if ok:
                ok = False
                i = next(i for i in range(PAGE) if x[i] != y[i])
                why = 'memory differs at 0x%x (orig %02x, c %02x)' % ((pg << 12) + i, x[i], y[i])
            break
        for i, xo, yc in zip(m.full, full_o, full_c):
            if m.wr[i][0] == STACK_TOP - STACK_SIZE:
                continue
            if spec.get('fp') and ro == 'ok' and rc == 'ok':
                yc = fp_close(xo, yc, spec.get('tol', 4e-7))
                fp_worst[0] = max(fp_worst[0], fp_dev[0]); fp_dev[0] = 0.0
            if xo != yc and ok:
                ok = False
                j = next(j for j in range(len(xo)) if xo[j] != yc[j])
                why = 'memory differs at 0x%x (orig %02x, c %02x)' % (m.wr[i][0] + j, xo[j], yc[j])
        if not ok:
            bad += 1
            if bad <= 3 or os.environ.get('DIFFTEST_ALL'):   # DIFFTEST_ALL=1: every mismatch to the log
                details.append('MISMATCH %s trial %d args=%s orig=%s c=%s %s' % (name, t, [hex(a & 0xffffffff) for a in args], (ro, vo), (rc, vc), why or ''))
    return bad, both_fault, n, details


# ---- quiet mode (default): one summary line per function on the console, the detail in a log file per function
# (buildlogs/round10/<address>.log for round-10 specs, else buildlogs/difftest/), an index line per function.
# --verbose or env DIFFTEST_VERBOSE=1 gives the old full console output.
VERBOSE = bool(os.environ.get('DIFFTEST_VERBOSE'))


def report_quiet(specfile, s, bad, bf, n, details, extra, cov):
    root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    base = os.path.basename(specfile)
    sub = 'round10' if base.startswith('spec_w10') else 'round11' if base.startswith('spec_w11') else \
        'round12' if base.startswith('spec_w12') else 'round13' if base.startswith('spec_w13') else \
        'round14' if base.startswith('spec_w14') else \
        'round15' if base.startswith('spec_w15') else \
        'round16' if base.startswith('spec_w16') else \
        'round17' if base.startswith('spec_w17') else \
        'round18' if base.startswith('spec_w18') else \
        'round19' if base.startswith('spec_w19') else \
        'round20' if base.startswith('spec_w20') else \
        'round21' if base.startswith('spec_w21') else \
        'round22' if base.startswith('spec_w22') else \
        'round30' if base.startswith('spec_w30') else \
        'round29' if base.startswith('spec_w29') else \
        'round28' if base.startswith('spec_w28') else \
        'round27' if base.startswith('spec_w27') else \
        'round26' if base.startswith('spec_w26') else \
        'round25' if base.startswith('spec_w25') else \
        'round24' if base.startswith('spec_w24') else \
        'round23' if base.startswith('spec_w23') else \
        'sound' if base.startswith('spec_snd') else 'difftest'
    logdir = os.environ.get('DIFFTEST_LOGDIR') or os.path.join(root, 'buildlogs', sub)
    os.makedirs(logdir, exist_ok=True)
    orig = s.get('orig', s.get('orig12', 0))   # 'orig12': 1.2's address (round 31)
    orig = orig if isinstance(orig, int) else 0
    key = '0x%08x' % orig if orig >= 0x10000000 else s['name']
    covtxt = ''
    if cov:
        covtxt = ', coverage %d/%d' % (cov[0], cov[1])
    status = 'PASS' if bad == 0 else 'FAIL'
    line = '%s %s %s %d trials, %d mismatches, %d both-fault%s%s' % (status, key, s['name'], n, bad, bf, covtxt, extra)
    path = os.path.join(logdir, '%s.log' % key)
    with open(path, 'w') as fo:
        if details:
            fo.write('=== FIRST MISMATCH ===\n%s\n=== END FIRST MISMATCH ===\n\n' % details[0])
        fo.write(line + '\nspec: %s\n' % specfile)
        if cov:
            fo.write('coverage %d/%d; not run: %s\n' % (cov[0], cov[1], ' '.join(cov[2])))
        for d in details[1:]:
            fo.write(d + '\n')
    idx = os.path.join(logdir, 'index.txt')
    rows = []
    if os.path.exists(idx):
        rows = [r for r in open(idx).read().splitlines() if r.split(' ')[1:2] != [key]]
    rows.append(line)
    with open(idx, 'w') as fo:
        fo.write('\n'.join(rows) + '\n')
    print(line)
    if details:
        print('  first: ' + details[0][:600])
        print('  log: ' + path)


def main():
    global VERBOSE
    args = [a for a in sys.argv[1:] if a != '--verbose']
    if '--verbose' in sys.argv[1:]:
        VERBOSE = True
        os.environ['DIFFTEST_VERBOSE'] = '1'
    jobs = 1
    if args and args[0].startswith('-j'):
        jobs = min(int(args[0][2:] or 16), 16)   # Diego, 2026-11-03 01:24: 16 processes
        args = args[1:]
    dll, specfile = args[0], args[1]
    want = set(args[2:])
    ns = {}
    exec(open(specfile).read(), ns)
    specs = [s for s in ns['SPECS'] if not want or s['name'] in want]
    if jobs > 1 and len(specs) > 1:
        # one process per share of the specs, balanced by trial count; output is printed per share
        shares = [[] for _ in range(min(jobs, len(specs)))]
        load = [0] * len(shares)
        for sp in sorted(specs, key=lambda x: -x.get('trials', 300)):
            i = load.index(min(load))
            shares[i].append(sp['name'])
            load[i] += sp.get('trials', 300)
        procs = [subprocess.Popen([sys.executable, '-u', os.path.abspath(__file__), dll, specfile] + names,
                                  stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True) for names in shares]
        fails = 0
        for pr in procs:
            out, _ = pr.communicate()
            print(out, end='')
            fails += pr.returncode != 0
        sys.exit(1 if fails else 0)
    srcs = [os.path.join(SRC, p) for p in ns['SOURCES']]
    csecs, syms = build_c(srcs, ns.get('CFLAGS', ()))
    if os.environ.get('DT_SYM'):    # where a C symbol landed (for W10_COV over C code)
        print('%s at 0x%x' % (os.environ['DT_SYM'], syms[os.environ['DT_SYM']]))
    m = Machine(dll, csecs, syms, V12_DLL if any('orig12' in sp for sp in specs) else None)
    m.libm_original = bool(ns.get('LIBM_ORIGINAL'))
    m.heap_spec_original = m.heap_original = bool(ns.get('HEAP_ORIGINAL'))   # a spec entry may set 'heap_original'
    for sp in specs:
        for a in sp.get('nop', []):
            m.uc.mem_write(a, b'\x90')
        m.snap0 = m.dump()
    fails = 0
    cov = ns.get('_cov_summary')
    for s in specs:
        bad, bf, n, details = run_spec(m, s)
        extra = ' (worst float deviation %.2e)' % fp_worst[0] if s.get('fp') else ''
        fp_worst[0] = 0.0
        if VERBOSE:
            for d in details:
                print('  ' + d)
            print('%s %-28s %d trials, %d mismatches, %d both-fault%s' % ('ok ' if bad == 0 else 'BAD', s['name'], n, bad, bf, extra))
        else:
            report_quiet(specfile, s, bad, bf, n, details, extra, cov() if cov and len(specs) == 1 else None)
        fails += bad > 0
    if not VERBOSE and len(specs) > 1:
        print('total: %d functions, %d failed' % (len(specs), fails))
    sys.exit(1 if fails else 0)


if __name__ == '__main__':
    main()
