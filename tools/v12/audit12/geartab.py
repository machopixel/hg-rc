# compare the row / column tables the 1.0 and 1.2 gear-sum functions read: sum = row[r] + sum col[idx_k]
import re, struct, sys, capstone, pefile
def L(p):
    pe = pefile.PE(p); return pe.get_memory_mapped_image(), pe.OPTIONAL_HEADER.ImageBase
I10, b = L('../GameRip/HGSIM.DLL'); I12, _ = L('../GameRip_12/hgsim.dll')
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
def scan(img, a):
    ins = list(md.disasm(img[a - b:a - b + 400], a))
    row = col = None; rs = cs = None
    for k, i in enumerate(ins):
        if i.mnemonic == 'imul' and row is None and rs is None:
            rs = int(i.op_str.split(',')[-1], 16)
        m = re.search(r'\[(\w+)(?:\*(\d))? \+ (0x1[0-9a-f]{7})\]', i.op_str)
        if m and i.mnemonic in ('mov', 'add'):
            if row is None:
                row = int(m.group(3), 16)
            elif col is None:
                col = int(m.group(3), 16)
                cs = int(m.group(2)) if m.group(2) else 0x98
        if i.mnemonic == 'ret':
            break
    return row, rs, col, cs
d32 = lambda img, a: struct.unpack_from('<i', img, a - b)[0]
pairs = [x.split(':') for x in sys.argv[1:]]
for p10, p12 in pairs:
    r10 = scan(I10, int(p10, 16)); r12 = scan(I12, int(p12, 16))
    bad = []
    for r in range(0, 64):
        if d32(I10, r10[0] + r * r10[1]) != d32(I12, r12[0] + r * r12[1]):
            bad.append(('row', r))
    for i in range(0, 300):
        if d32(I10, r10[2] + i * r10[3]) != d32(I12, r12[2] + i * r12[3]):
            bad.append(('col', i))
    print(p10, p12, [hex(x) if x and x > 0x1000 else x for x in r10], [hex(x) if x and x > 0x1000 else x for x in r12],
          'mismatches', len(bad), bad[:6])
