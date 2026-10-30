"""Disassemble one function of the original HGSIM.DLL in full: python -I tools/round10/dis_full.py 0xADDR SIZE
(SIZE in bytes, from the work list). tools/round6/dis.py stops after 400 instructions; this one does not.
Marks calls that go through a jmp thunk with '; -> target'."""
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


a = thunk(int(sys.argv[1], 16))
size = int(sys.argv[2], 0)
print('=====', hex(a), size, 'bytes')
for c in md.disasm(img[a - base:a - base + size], a):
    s = f'{c.address:x}: {c.mnemonic} {c.op_str}'
    m = re.search(r'call 0x([0-9a-f]+)', s)
    if m:
        t = int(m.group(1), 16)
        if thunk(t) != t:
            s += f'   ; -> {thunk(t):x}'
    print(s)
