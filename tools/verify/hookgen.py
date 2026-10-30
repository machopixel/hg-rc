#!/usr/bin/env python3
"""
Prints hook table entries (src/hooks/conv_*.inc) for converted functions:
CONV(rva, FunctionName, expected original bytes...).

  hookgen.py HGSIM.DLL Name=0x100b9748 [Name=0x... ...]

For every function it copies the first bytes of the original (enough whole
instructions to cover the 5-byte jump) as the expected prologue, so a
different build of the game is never patched. Functions whose first
instructions contain an absolute address stay valid at the preferred base
only; Hook_InstallModule skips them when the DLL is relocated.
"""
import sys
import capstone
import pefile

pe = pefile.PE(sys.argv[1])
base = pe.OPTIONAL_HEADER.ImageBase
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
for arg in sys.argv[2:]:
    name, addr = arg.split('=')
    a = int(addr, 16)
    off = pe.get_offset_from_rva(a - base)
    data = pe.__data__[off:off + 32]
    n = 0
    for ins in md.disasm(data, a):
        n += ins.size
        if n >= 5:
            break
    prologue = ', '.join('0x%02X' % b for b in data[:n])
    print('CONV(0x%X, %s, %s)' % (a - base, name, prologue))
