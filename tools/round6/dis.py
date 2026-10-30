"""Disassemble functions of the original HGSIM.DLL: python tools/round6/dis.py 0x100097d3 [...]
Follows a jmp thunk at the start; marks calls that go through a thunk."""
import re, struct, sys
import capstone, pefile

DLL = r"C:\Coding\HeavyGear\Project\HGSIM.DLL"
pe = pefile.PE(DLL, fast_load=True)
base = pe.OPTIONAL_HEADER.ImageBase
img = pe.get_memory_mapped_image()
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)


def thunk(t):
    d = img[t - base:t - base + 5]
    return t + 5 + struct.unpack('<i', d[1:5])[0] if d[0] == 0xE9 else t


def fn(a, maxn=400):
    a = thunk(a)
    n = 0
    for c in md.disasm(img[a - base:a - base + maxn * 8], a):
        s = f'{c.address:x}: {c.mnemonic} {c.op_str}'
        m = re.search(r'call 0x([0-9a-f]+)', s)
        if m:
            t = int(m.group(1), 16)
            if thunk(t) != t:
                s += f'   ; -> {thunk(t):x}'
        print(s)
        n += 1
        if c.mnemonic == 'int3' or n >= maxn:
            break


for x in sys.argv[1:]:
    print('=====', x)
    fn(int(x, 16))
