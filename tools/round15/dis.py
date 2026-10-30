# Round 15: instruction listing of one HGSIM.DLL range (capstone), with thunks resolved to their targets.
#   python -I tools/round15/dis.py <HGSIM.DLL> <start hex> <end hex> > out.txt
import sys, pefile, capstone
pe = pefile.PE(sys.argv[1])
base = pe.OPTIONAL_HEADER.ImageBase
img = pe.get_memory_mapped_image()
a, b = int(sys.argv[2], 16), int(sys.argv[3], 16)
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)


def thunk(t):
    ins = next(md.disasm(img[t - base:t - base + 8], t), None)
    if ins and ins.mnemonic == 'jmp' and ins.op_str.startswith('0x'):
        return int(ins.op_str, 16)
    return None


for i in md.disasm(img[a - base:b - base], a):
    s = '%08x  %-6s %s' % (i.address, i.mnemonic, i.op_str)
    if i.mnemonic == 'call' and i.op_str.startswith('0x'):
        t = thunk(int(i.op_str, 16))
        if t:
            s += '    ; -> %08x' % t
    print(s)
