# Round 31: run an existing 1.0 spec against 1.2's version of the function (difftest's 'orig12': 1.2's DLL relocated
# to BASE12 in the same machine). exec after w10_common.py and the spec; then
#     SPECS = [v12_spec(SPECS[0], f12, ...)]
# The 1.0 and 1.2 listings of the function are aligned instruction by instruction (absolute addresses and stack
# offsets ignored); the absolute addresses of the aligned instructions give the pairs 1.0 -> 1.2 (globals, callees).
#   to12    copies every paired writable 1.0 global (4 bytes, or the spec's window) to its 1.2 address;
#   from12  copies them back after 1.2's run, so the comparison sees 1.2's results at the 1.0 addresses.
# The w10_common stubs (helper()) are hooked at the paired 1.2 callee addresses too and log under the 1.0 name.
# A spec adds what the pairing cannot know: windows (arrays), pairs of its own, and its own to12 / from12 steps
# (e.g. pointers into 1.2's data turned into the C's).
import difflib, re, struct
import pefile as _pefile
import capstone as _capstone

BASE12 = 0x20000000
V12_DLL = '../GameRip_12/hgsim.dll'
V10_DLL = '../GameRip/HGSIM.DLL'
_md = _capstone.Cs(_capstone.CS_ARCH_X86, _capstone.CS_MODE_32)
_PE = {}


def A12(a):
    return BASE12 + a - 0x10000000


def _img(path):
    if path not in _PE:
        pe = _pefile.PE(path)
        b = pe.OPTIONAL_HEADER.ImageBase
        wr = [(b + s.VirtualAddress, b + s.VirtualAddress + max(s.Misc_VirtualSize, s.SizeOfRawData))
              for s in pe.sections if s.Characteristics & 0x80000000]
        _PE[path] = (b, pe.get_memory_mapped_image(), wr)
    return _PE[path]


def _thunk(path, a):
    b, img, _ = _img(path)
    o = a - b
    if 0 <= o < len(img) and img[o] == 0xe9:
        return a + 5 + struct.unpack_from('<i', img, o + 1)[0]
    return a


def _listing(path, a):
    b, img, _ = _img(path)
    far, out = a, []
    for i in _md.disasm(img[a - b:a - b + 0x8000], a):
        if (i.mnemonic.startswith('j') or i.mnemonic == 'loop') and i.op_str.startswith('0x'):
            t = int(i.op_str, 16)
            if a <= t < a + 0x8000:
                far = max(far, t)
        nums = [int(x, 16) for x in re.findall(r'0x1[0-9a-f]{7}', i.op_str)]
        if i.mnemonic == 'call' and nums:
            nums = [_thunk(path, nums[0])]
        key = re.sub(r'0x1[0-9a-f]{7}', 'G', i.op_str)
        key = re.sub(r'ebp - (0x[0-9a-f]+|\d+)', 'ebp-L', key)
        key = re.sub(r'ebp \+ \w+ - (0x[0-9a-f]+|\d+)', 'ebp+r-L', key)
        out.append((i.mnemonic + ' ' + key, nums))
        if i.mnemonic == 'ret' and i.address >= far:
            break
    return out


def v12_pairs(f10, f12):
    """{1.0 address: 1.2 address} from the aligned listings of f10 / f12"""
    A, B = _listing(V10_DLL, f10), _listing(V12_DLL, f12)
    sm = difflib.SequenceMatcher(None, [x[0] for x in A], [x[0] for x in B], autojunk=False)
    pairs = {}
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag != 'equal':
            continue
        for k in range(i2 - i1):
            for u, v in zip(A[i1 + k][1], B[j1 + k][1]):
                pairs.setdefault(u, v)
    return pairs


def _writable(path, a):
    return any(lo <= a < hi for lo, hi in _img(path)[2])


def v12_spec(spec, f12, windows=None, extra_pairs=None, skip=(), to12_more=None, from12_more=None, name=None):
    """spec run against 1.2's f12 (the C side unchanged)"""
    pairs = v12_pairs(spec['orig'], f12)
    pairs.update(extra_pairs or {})
    windows = dict(windows or {})
    glob = sorted((a10, a12) for a10, a12 in pairs.items()
                  if a10 not in skip and _writable(V10_DLL, a10) and _writable(V12_DLL, a12))
    calls = {a10: a12 for a10, a12 in pairs.items() if a10 in HELPERS}

    def hook12(m):
        if getattr(m, '_v12hooks', None) is calls:
            return
        m._v12hooks = calls
        for a10, a12 in calls.items():
            cb = _stub_cb(m, a10)
            m.raw.hook_add(UC_HOOK_CODE, cb, None, A12(a12), A12(a12))

    def to12(m):
        hook12(m)
        for a10, a12 in glob:
            n = windows.get(a10, 4)
            m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(a10, n)))
        if to12_more:
            to12_more(m)

    def from12(m):
        for a10, a12 in glob:
            n = windows.get(a10, 4)
            m.uc.mem_write(a10, bytes(m.uc.mem_read(A12(a12), n)))
        if from12_more:
            from12_more(m)

    s = dict(spec)
    s.pop('orig', None)
    s.update(orig12=f12, to12=to12, from12=from12, name=name or spec['name'] + '_v12')
    s['_v12_pairs'] = pairs
    return s


def painter_slot(ctx):
    """Round 32: the sim's own renderer state call around 1.2's particle squares (W32_PAINTER, slot [0x109e3d40]) is
    not in 1.2's code; the C side gets a bare ret there, so the logs compare the rest."""
    b = newbuf(ctx, 4)
    ctx.m.uc.mem_write(b, b'\xc3')
    w32(ctx, 0x109e3d40, b)
