# Equivalence checking for HGSIM's sound code with SDL3 (src/sim/sound/sdl_sound.c; DECISIONS.md "HGSIM sound:
# DirectSound replaced by SDL3"). A spec does
#     exec(open('tools/verify/w10_common.py').read()); exec(open('tools/verify/sound_common.py').read())
# The original runs against fake DirectSound objects (method tables of stubs), the C against stubs of the
# audio.h functions (SOURCES gets tools/verify/audio_stub.c). Both sides log the same events
#     ('Create', bytes, rate, channels, bits) ('Release', voice) ('Play', voice, loop) ('Stop', voice)
#     ('Volume', voice, v) ('Pan', voice, v) ('Rate', voice, hz) ('Status', voice) ('Lock', voice, offset, bytes,
#     flags) ('Unlock', voice, p1, n1, p2, n2) ('Open',) ('Close',)
# with the same answers (status flags, new voice handles, clip memory for Lock), so the game logic is compared
# exactly: same events in the same order, same memory, same return value. DirectSound calls that have no SDL3
# counterpart (SetCooperativeLevel, SetSpeakerConfig, the 3D interfaces, A3D) log nothing.
AUDIO_STUB = os.path.abspath(os.path.join('tools', 'verify', 'audio_stub.c'))
SND_TABLE = 0x00F00600          # the fake DirectSound buffer method table
SND_CODE = 0x00F00700           # stub code addresses: one per method (hooks), 8 bytes apart
DS_BUF = {          # offset: (event, number of stack args incl. this)
    0x08: ('Release', 1), 0x24: ('Status', 2), 0x2c: ('Lock', 8), 0x30: ('Play', 4), 0x3c: ('Volume', 2),
    0x40: ('Pan', 2), 0x44: ('Rate', 2), 0x48: ('Stop', 1), 0x4c: ('Unlock', 5),
}
AUDIO = {           # symbol: (event, number of args)
    'Audio_VoiceDestroy': ('Release', 1), 'Audio_VoiceGetStatus': ('Status', 1), 'Audio_VoiceLock': ('Lock', 8),
    'Audio_VoicePlay': ('Play', 2), 'Audio_VoiceSetVolume': ('Volume', 2), 'Audio_VoiceSetPan': ('Pan', 2),
    'Audio_VoiceSetRate': ('Rate', 2), 'Audio_VoiceStop': ('Stop', 1), 'Audio_VoiceUnlock': ('Unlock', 5),
    'Audio_VoiceCreate': ('Create', 4), 'Audio_Open': ('Open', 0), 'Audio_Close': ('Close', 0),
}
DSERR_INVALIDPARAM = 0x80070057
RANGE = {'Volume': (-10000, 0), 'Pan': (-10000, 10000), 'Rate': (100, 100000)}   # Rate 0: the clip's own rate
SND = {}            # per trial: 'status' {voice: flags}, 'clip' {voice: (address, size)}, 'new' [handles]


def _arg(uc, esp, k):
    return struct.unpack('<I', bytes(uc.mem_read(esp + 4 + 4 * k, 4)))[0]


def _event(m, uc, name, a):
    """Logs one event and answers it (eax). a = the arguments without DirectSound's own shape."""
    if ST.get('slog') is not m.api_log:
        ST['slog'] = m.api_log
        ST['snew'] = 0
    if name == 'Status':
        m.api_log.append(('Status', a[0]))
        return SND['status'].get(a[0], 0)
    if name == 'Create':
        m.api_log.append(('Create',) + tuple(a))
        k = ST['snew']
        ST['snew'] = k + 1
        return SND['new'][k] if k < len(SND['new']) else 0
    if name == 'Lock':
        voice, offset, nbytes, p1, n1, p2, n2, flags = a
        m.api_log.append(('Lock', voice, offset, nbytes, flags))
        addr, size = SND['clip'].get(voice, (0, 0))
        if flags & 2:
            offset, nbytes = 0, size
        ok = offset < size and nbytes <= size
        first = min(nbytes, size - offset) if ok else 0
        uc.mem_write(p1, struct.pack('<I', addr + offset if ok else 0))
        uc.mem_write(n1, struct.pack('<I', first))
        if p2:
            uc.mem_write(p2, struct.pack('<I', addr if ok and nbytes > first else 0))
        if n2:
            uc.mem_write(n2, struct.pack('<I', nbytes - first if ok else 0))
        return 0 if ok else DSERR_INVALIDPARAM
    if name == 'Play':
        m.api_log.append(('Play', a[0], a[1] & 1))
        return 0
    if name in RANGE:                   # DirectSound refuses these values (and keeps the old one)
        lo, hi = RANGE[name]
        v = a[1] - (1 << 32) if name != 'Rate' and a[1] >= 1 << 31 else a[1]
        m.api_log.append((name,) + tuple(a))
        return 0 if lo <= v <= hi or (name == 'Rate' and v == 0) else DSERR_INVALIDPARAM
    m.api_log.append((name,) + tuple(a))
    return SND.get('open', 1) if name == 'Open' else 0


def _ds_cb(m, off):
    name, n = DS_BUF[off]

    def cb(uc, address, size, ud):
        uc = m.uc                            # host writes noted as dirty pages, so they are compared
        esp = uc.reg_read(UC_X86_REG_ESP)
        raw = [_arg(uc, esp, k) for k in range(n)]
        if name == 'Play':
            a = [raw[0], raw[3]]
        elif name == 'Lock':
            a = raw[:8]
        else:
            a = raw
        eax = _event(m, uc, name, a)
        if name == 'Status':                      # DirectSound writes the flags through the pointer
            uc.mem_write(raw[1], struct.pack('<I', eax))
            eax = 0
        uc.reg_write(UC_X86_REG_EAX, eax & 0xffffffff)
        uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4 + 4 * n)       # __stdcall
    return cb


def _audio_cb(m, sym):
    name, n = AUDIO[sym]

    def cb(uc, address, size, ud):
        uc = m.uc                            # host writes noted as dirty pages, so they are compared
        esp = uc.reg_read(UC_X86_REG_ESP)
        a = [_arg(uc, esp, k) for k in range(n)]
        eax = _event(m, uc, name, a)
        uc.reg_write(UC_X86_REG_EAX, eax & 0xffffffff)
        uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)                 # cdecl
    return cb


SND3D_TABLE = 0x00F00780       # the 3D listener / 3D buffer methods: answered (0) but not logged, because the
SND3D_CODE = 0x00F00800 - 0x30  # SDL3 code has nothing in their place (8-byte stubs ret N below 0x00F00800)
DS_3D = {0x28: 3, 0x30: 3, 0x44: 1}   # SetAllParameters (listener +0x28, 3D buffer +0x30), CommitDeferredSettings


SND_DEV_TABLE = 0x00F00680      # the fake DirectSound device's method table (CreateSoundBuffer +0xc)
SND_DEV_CODE = 0x00F00760       # its stub, and the buffers' QueryInterface stub at +8


def _create_cb(m):
    def cb(uc, address, size, ud):           # CreateSoundBuffer(device, desc, &buffer, outer)
        uc = m.uc                            # host writes noted as dirty pages, so they are compared
        esp = uc.reg_read(UC_X86_REG_ESP)
        desc, out = _arg(uc, esp, 1), _arg(uc, esp, 2)
        fmt = ur32(uc, desc + 0x10)
        word = lambda a: struct.unpack('<H', bytes(uc.mem_read(a, 2)))[0]
        v = _event(m, uc, 'Create', [ur32(uc, desc + 8), ur32(uc, fmt + 4), word(fmt + 2), word(fmt + 0xe)])
        uc.mem_write(out, struct.pack('<I', v))
        uc.reg_write(UC_X86_REG_EAX, 0 if v else 0x8007000e)
        uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4 + 16)
    return cb


def _qi_cb(m):
    def cb(uc, address, size, ud):           # QueryInterface(buffer, iid, &out): no 3D interface, as with SDL3
        uc = m.uc                            # host writes noted as dirty pages, so they are compared
        esp = uc.reg_read(UC_X86_REG_ESP)
        uc.mem_write(_arg(uc, esp, 2), struct.pack('<I', 0))
        uc.reg_write(UC_X86_REG_EAX, 0x80004002)
        uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4 + 12)
    return cb


def sound_install(ctx):
    """Once per machine: the fake buffer method tables and the hooks on both sides."""
    m = ctx.m
    if getattr(m, '_snd', False):
        return
    m._snd = True
    m.raw.mem_write(SND_DEV_CODE, b'\xc3')
    m.raw.mem_write(SND_DEV_TABLE + 0xc, struct.pack('<I', SND_DEV_CODE))
    m.raw.hook_add(UC_HOOK_CODE, _create_cb(m), None, SND_DEV_CODE, SND_DEV_CODE)
    m.raw.mem_write(SND_DEV_CODE + 8, b'\xc3')
    m.raw.mem_write(SND_TABLE, struct.pack('<I', SND_DEV_CODE + 8))
    m.raw.hook_add(UC_HOOK_CODE, _qi_cb(m), None, SND_DEV_CODE + 8, SND_DEV_CODE + 8)
    for k, (off, n) in enumerate(sorted(DS_3D.items())):
        code = SND3D_CODE + 8 * k
        m.raw.mem_write(code, b'\x31\xc0\xc2' + struct.pack('<H', 4 * n))     # xor eax, eax; ret 4n
        m.raw.mem_write(SND3D_TABLE + off, struct.pack('<I', code))
    for k, off in enumerate(sorted(DS_BUF)):
        code = SND_CODE + 8 * k
        m.raw.mem_write(code, b'\xc3')
        m.raw.mem_write(SND_TABLE + off, struct.pack('<I', code))
        m.raw.hook_add(UC_HOOK_CODE, _ds_cb(m, off), None, code, code)
    for sym in AUDIO:
        if sym in m.syms:
            m.raw.hook_add(UC_HOOK_CODE, _audio_cb(m, sym), None, m.syms[sym], m.syms[sym])


def voice(ctx):
    """A voice handle: on the original's side a DirectSound buffer object (its first dword the method table)."""
    v = newbuf(ctx, 0x10)
    w32(ctx, v, SND_TABLE)
    return v


def device(ctx):
    """The DirectSound device on the original's side (on the C side only a token)."""
    d = newbuf(ctx, 0x10)
    w32(ctx, d, SND_DEV_TABLE)
    return d


def object3d(ctx):
    """A 3D listener / 3D buffer object for the original's 3D path (silent methods)."""
    v = newbuf(ctx, 0x10)
    w32(ctx, v, SND3D_TABLE)
    return v


def sound_block(ctx, r, three_d=None):
    """A random sound block [0x1015d0a4] with up to 16 slots of voices (states 0..5, flags, volumes, pans, ids, the
    next sound), the device token 0x109e369c, statuses and clip memory for every voice; returns the slot count."""
    blk = newbuf(ctx, 0x400)
    n = r.choice([0, 1, 4, 11, r.randrange(1, 17)])
    w32(ctx, blk + 4, n)
    if three_d is None:
        three_d = r.random() < 0.2
    w32(ctx, blk + 0x10, 1 if three_d else 0)
    w32(ctx, blk + 0xc, object3d(ctx) if three_d and r.random() < 0.8 else 0)
    SND['status'], SND['clip'], SND['new'] = {}, {}, [voice(ctx) for _ in range(3)]
    for i in range(16):
        v = voice(ctx)
        w32(ctx, blk + 0x94 + 8 * i, v)
        w32(ctx, blk + 0x114 + 4 * i, object3d(ctx))
        w32(ctx, blk + 0x54 + 4 * i, r.choice([0, 1, 2, 2, 3, 4, 5]))
        w32(ctx, blk + 0x154 + 4 * i, r.choice([0, 0x00a10000, 0x00a10000]))
        w32(ctx, blk + 0x14 + 4 * i, r.choice([0, 1, 2]))
        unit = newbuf(ctx, 0x70)
        w32(ctx, unit + 0x28, newbuf(ctx, 0x560))
        w32(ctx, blk + 0x194 + 4 * i, unit)
        w32(ctx, blk + 0x294 + 4 * i, r.choice([0, -600, -10000, r.randrange(-10000, 1), r.randrange(1, 7501),
                                              -10001, r.randrange(-30000, -10000)]) & 0xffffffff)
        w32(ctx, blk + 0x254 + 4 * i, r.choice([0xffffffff, 0, r.randrange(-10000, 10001) & 0xffffffff, 10001,
                                              r.randrange(-30000, -10000) & 0xffffffff]))
        w32(ctx, blk + 0x214 + 4 * i, r.randrange(-700, 700) & 0xffffffff)
        w32(ctx, blk + 0x2d4 + 4 * i, r.randrange(0, 50))
        w16(ctx, blk + 0x314 + 2 * i, r.choice([0, 0x2000, 0x100, 0x2103, r.getrandbits(16)]))
        w32(ctx, blk + 0x334 + 4 * i, r.choice([0xffffffff, 0xffffffff, r.randrange(0, 100)]))
        SND['status'][v] = r.choice([0, 1, 5, 4])
        size = r.choice([0x100, 0x1000])
        SND['clip'][v] = (newbuf(ctx, size), size)
    for v in SND['new']:
        size = r.choice([0x100, 0x800])
        SND['clip'][v] = (newbuf(ctx, size), size)
    w32(ctx, 0x1015d0a4, blk)
    w32(ctx, 0x109e369c, r.choice([0x00c00000, 0x00c00000, 0]))
    return n


# Sound start and shutdown (0x1003a0e4 / 0x1003a548). On the original's side the device is the C's token object
# SndSdl_Device with a silent method table (SetCooperativeLevel +0x18, SetSpeakerConfig +0x24) whose Release (+8)
# is the 'Close' event, DirectSoundCreate (0x1010ce76) is the 'Open' event (it writes the token), CoInitialize /
# CoUninitialize answer silently and the A3D CoCreateInstance fails (no A3D, as SDL3 never uses it).
SND_TOK_TABLE = 0x00F00A00
SND_TOK_CODE = 0x00F00A80
DS_CREATE = 0x1010ce76
IAT_CO = {0x10ac58a8: b'\x31\xc0\xc2\x04\x00',                 # CoInitialize: S_OK
          0x10ac58ac: b'\xb8\x54\x01\x04\x80\xc2\x14\x00',      # CoCreateInstance: REGDB_E_CLASSNOTREG
          0x10ac58a4: b'\xc3'}                                  # CoUninitialize


def _dscreate_cb(m):
    def cb(uc, address, size, ud):           # DirectSoundCreate(guid, &device, outer), __stdcall
        uc = m.uc                            # host writes noted as dirty pages, so they are compared
        esp = uc.reg_read(UC_X86_REG_ESP)
        ok = _event(m, uc, 'Open', [])
        if ok:
            uc.mem_write(_arg(uc, esp, 1), struct.pack('<I', m.syms['SndSdl_Device']))
        uc.reg_write(UC_X86_REG_EAX, 0 if ok else 0x88780078)
        uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4 + 12)
    return cb


def _close_cb(m):
    def cb(uc, address, size, ud):           # Release(device), __stdcall
        uc = m.uc                            # host writes noted as dirty pages, so they are compared
        esp = uc.reg_read(UC_X86_REG_ESP)
        _event(m, uc, 'Close', [])
        uc.reg_write(UC_X86_REG_EAX, 0)
        uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4 + 4)
    return cb


def sound_device_install(ctx):
    """Once per machine: the hooks above. Every trial: the token's method table and the Co* import slots."""
    m = ctx.m
    if not getattr(m, '_snd_dev', False):
        m._snd_dev = True
        m.raw.mem_write(SND_TOK_CODE, b'\xc3')
        m.raw.hook_add(UC_HOOK_CODE, _close_cb(m), None, SND_TOK_CODE, SND_TOK_CODE)
        m.raw.mem_write(SND_TOK_CODE + 8, b'\x31\xc0\xc2\x0c\x00')          # SetCooperativeLevel: S_OK
        m.raw.mem_write(SND_TOK_CODE + 0x10, b'\x31\xc0\xc2\x08\x00')       # SetSpeakerConfig: S_OK
        m.raw.mem_write(SND_TOK_TABLE + 8, struct.pack('<I', SND_TOK_CODE))
        m.raw.mem_write(SND_TOK_TABLE + 0x18, struct.pack('<I', SND_TOK_CODE + 8))
        m.raw.mem_write(SND_TOK_TABLE + 0x24, struct.pack('<I', SND_TOK_CODE + 0x10))
        for k, (iat, code) in enumerate(sorted(IAT_CO.items())):
            m.raw.mem_write(SND_TOK_CODE + 0x20 + 0x10 * k, code)
        m.raw.hook_add(UC_HOOK_CODE, _dscreate_cb(m), None, DS_CREATE, DS_CREATE)
    for k, (iat, code) in enumerate(sorted(IAT_CO.items())):
        w32(ctx, iat, SND_TOK_CODE + 0x20 + 0x10 * k)
    w32(ctx, m.syms['SndSdl_Device'], SND_TOK_TABLE)