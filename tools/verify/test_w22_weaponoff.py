# Round 22: proof of the deliberate dropped-gun fix (DECISIONS.md round 22). There is no original to compare the
# new behaviour with, so this runs the C alone in the checker's emulator (difftest.Machine, the real HGSIM.DLL
# mapped) and checks the expected result of each case:
#   1 a weapon recorded on part 10 (as Sim_1005bb7a records it): Sim_1006bf33(code 10) switches exactly that slot off
#     (+0 = 0, +8 = -1, +0x114 = 1, +0x20 = -2), every other byte of the slots unchanged;
#   2 another part (11): nothing changes;
#   3 the slot's weapon kind +0x34 changed since it was recorded (a reloaded design): nothing changes;
#   4 a slot whose own code +0x30 matches (the original's rule, old-loader designs): only that slot, as before;
#   5 the record forgotten (Sim_ForgetWeaponMount, the old loader): nothing changes;
#   6 a disabled slot (+4 < 0): nothing changes.
#   python -I tools/verify/test_w22_weaponoff.py <HGSIM.DLL>
import os, sys, struct, runpy

HERE = os.path.dirname(os.path.abspath(__file__))
dt = runpy.run_path(os.path.join(HERE, 'difftest.py'), run_name='difftest_lib')
os.chdir(os.path.join(HERE, '..', '..'))
w10 = {}
exec(open(os.path.join(HERE, 'w10_common.py')).read(), w10)
sources = w10['closure'](['sim/unsorted_1006/w9c_event.c', 'sim/unsorted_1006/w22_mounts.c'])
csecs, syms = dt['build_c']([os.path.join(dt['SRC'], p) for p in sources], w10['CFLAGS'])
m = dt['Machine'](sys.argv[1], csecs, syms)
A = dt['ARGMEM']
GEAR, UNIT, SLOTS = A, A + 0x100, A + 0x200


def w32(a, v):
    m.uc.mem_write(a, struct.pack('<I', v & 0xffffffff))


def r32(a):
    return struct.unpack('<I', bytes(m.uc.mem_read(a, 4)))[0]


def setup():
    m.reset_to_snap0()
    m.uc.mem_write(A, bytes(0x200 + 8 * 0x128))
    w32(GEAR, UNIT)
    w32(UNIT + 4, 3)                    # not the player: no cockpit message
    w32(GEAR + 0x84, SLOTS)
    for i in range(8):
        s = SLOTS + i * 0x128
        w32(s, 1)
        w32(s + 4, i + 2)               # weapon kind, >= 0
        w32(s + 8, 1)
        w32(s + 0x20, 0x40)
        w32(s + 0x30, [3, 4, 2, 7, 3, 4, 2, 7][i])     # the codes Sim_1005b923 gives
        w32(s + 0x34, 100 + i)


def slots():
    return bytes(m.uc.mem_read(SLOTS, 8 * 0x128))


def call(name, *args):
    r, v = m.call(syms[name], list(args))
    assert r == 'ok', (name, r, v)


def expect_off(before, after, idx):
    b, a = bytearray(before), bytearray(after)
    s = idx * 0x128
    for off, val in ((0, 0), (8, 0xffffffff), (0x114, 1), (0x20, 0xfffffffe)):
        b[s + off:s + off + 4] = struct.pack('<I', val)
    return bytes(b) == bytes(a)


fails = 0
# 1
setup(); call('Sim_NoteWeaponMount', GEAR, SLOTS + 2 * 0x128, 10); b = slots(); call('Sim_1006bf33', 0, 10, GEAR)
ok1 = expect_off(b, slots(), 2)
# 2
setup(); call('Sim_NoteWeaponMount', GEAR, SLOTS + 2 * 0x128, 10); b = slots(); call('Sim_1006bf33', 0, 11, GEAR)
ok2 = slots() == b
# 3
setup(); call('Sim_NoteWeaponMount', GEAR, SLOTS + 2 * 0x128, 10); w32(SLOTS + 2 * 0x128 + 0x34, 999); b = slots()
call('Sim_1006bf33', 0, 10, GEAR)
ok3 = slots() == b
# 4: the original rule first: code 4 matches slot 1's +0x30
setup(); call('Sim_NoteWeaponMount', GEAR, SLOTS + 5 * 0x128, 4); b = slots(); call('Sim_1006bf33', 0, 4, GEAR)
ok4 = expect_off(b, slots(), 1)
# 5
setup(); call('Sim_NoteWeaponMount', GEAR, SLOTS + 2 * 0x128, 10); call('Sim_ForgetWeaponMount', SLOTS + 2 * 0x128)
b = slots(); call('Sim_1006bf33', 0, 10, GEAR)
ok5 = slots() == b
# 6
setup(); call('Sim_NoteWeaponMount', GEAR, SLOTS + 2 * 0x128, 10); w32(SLOTS + 2 * 0x128 + 4, 0xffffffff); b = slots()
call('Sim_1006bf33', 0, 10, GEAR)
ok6 = slots() == b
for i, ok in enumerate((ok1, ok2, ok3, ok4, ok5, ok6), 1):
    print('case %d %s' % (i, 'ok' if ok else 'FAILED'))
    fails += not ok
print('PASS' if fails == 0 else 'FAIL %d' % fails)
sys.exit(1 if fails else 0)
