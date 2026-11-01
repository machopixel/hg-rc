# Sound round (1.2): the sound start SndSdl_1003a0e4 against 1.2's 0x1002e159, on spec_snd_1003a0e4.py's
# generator (HGSND.CFG's 3D word off, no A3D). 1.2's changes have no SDL3 counterpart and are answered silently on
# its side, so the rest is compared (the Open event, the device token, the block's +0 / +0x10, the return value):
#   - the primary buffer it now always makes (the token's CreateSoundBuffer +0xc: a silent primary object) and its
#     SetFormat 22050 Hz 16-bit stereo (+0x38, S_OK);
#   - its music stream (0x1002e918 answers a handle; [0x10131310] is 1.2's own memory, not compared).
# 1.2's own imports get the same silent CoInitialize / failing CoCreateInstance / CoUninitialize as 1.0's, and its
# DirectSoundCreate thunk (0x100ea67c) is the 'Open' event.
# Negative check: SND_NEG12=1 builds the C with SND_MUTATE12_START (block +0 not set).
import os
exec(open('tools/verify/spec_snd_1003a0e4.py').read())
exec(open('tools/verify/v12_common.py').read())
if os.environ.get('SND_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DSND_MUTATE12_START']
PRIM_TABLE = 0x00F00B00         # the silent primary buffer: SetFormat (+0x38) answers S_OK
PRIM_CODE = 0x00F00B80
PRIM = 0x00F00BC0               # the primary buffer object (its first dword the table)
CO12 = {0x100eb2a0: 0, 0x100eb2a8: 1, 0x100eb2a4: 2}     # 1.2's slots -> the 1.0 stubs (sorted IAT_CO order)


def _prim_create_cb(m):
    def cb(uc, address, size, ud):           # CreateSoundBuffer(device, desc, &buffer, outer) for the primary
        uc = m.uc
        esp = uc.reg_read(UC_X86_REG_ESP)
        uc.mem_write(_arg(uc, esp, 2), struct.pack('<I', PRIM))
        uc.reg_write(UC_X86_REG_EAX, 0)
        uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4 + 16)
    return cb


def install12(m):
    if getattr(m, '_snd12', False):
        return
    m._snd12 = True
    m.raw.mem_write(PRIM_CODE, b'\xc3')
    m.raw.hook_add(UC_HOOK_CODE, _prim_create_cb(m), None, PRIM_CODE, PRIM_CODE)
    m.raw.mem_write(PRIM_CODE + 8, b'\x31\xc0\xc2\x08\x00')            # SetFormat: S_OK
    m.raw.mem_write(PRIM_TABLE + 0x38, struct.pack('<I', PRIM_CODE + 8))
    m.raw.mem_write(PRIM, struct.pack('<I', PRIM_TABLE))
    m.raw.mem_write(SND_TOK_TABLE + 0xc, struct.pack('<I', PRIM_CODE))
    m.raw.hook_add(UC_HOOK_CODE, _dscreate_cb(m), None, A12(0x100ea67c), A12(0x100ea67c))


def to12_more(m):
    install12(m)
    for slot, k in CO12.items():
        m.uc.mem_write(A12(slot), struct.pack('<I', SND_TOK_CODE + 0x20 + 0x10 * k))
    m.uc.mem_write(A12(0x1002e918), b'\xb8\x00\x00\xae\x00\xc3')      # music stream: mov eax, handle; ret


SPECS = [v12_spec(SPECS[0], 0x1002e159, to12_more=to12_more,
                  extra_pairs={0x1015d1b8: 0x1012fe4c, 0x1015d0a4: 0x1012d488})]
