# Round 22: reads of [ebp-x] locals that are not written on every path from the function entry (must-be-written
# data flow over the basic blocks), for the renderer group. A local whose address is taken (lea) is listed apart:
# a callee may fill it.
#   python -I tools/round22/unsetscan.py <HGSIM.DLL> <start hex> <end hex> [...]
import sys, re, pefile, capstone
from capstone.x86 import X86_OP_MEM, X86_REG_EBP

pe = pefile.PE(sys.argv[1])
base = pe.OPTIONAL_HEADER.ImageBase
img = pe.get_memory_mapped_image()
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
md.detail = True


def scan(start, end):
    ins = list(md.disasm(img[start - base:end - base], start))
    at = {i.address: k for k, i in enumerate(ins)}
    leaders = {start}
    for k, i in enumerate(ins):
        if i.group(capstone.CS_GRP_JUMP) or i.mnemonic.startswith('ret'):
            if k + 1 < len(ins):
                leaders.add(ins[k + 1].address)
            if i.operands and i.operands[0].type == capstone.x86.X86_OP_IMM:
                leaders.add(i.operands[0].imm)
        if i.mnemonic == 'jmp' and i.operands[0].type != capstone.x86.X86_OP_IMM:
            pass                                       # jump tables: targets come from the table scan below
    # jump tables: dword tables right after the code that hold addresses inside the function
    for k, i in enumerate(ins):
        m = re.search(r'\[(e\w\w)\*4 \+ (0x[0-9a-f]+)\]', i.op_str)
        if i.mnemonic == 'jmp' and m:
            t = int(m.group(2), 16)
            while True:
                v = int.from_bytes(img[t - base:t - base + 4], 'little')
                if not start <= v < end:
                    break
                leaders.add(v)
                t += 4
    leaders = sorted(a for a in leaders if a in at)
    blocks = {}
    for n, a in enumerate(leaders):
        b = at[a]
        e = at[leaders[n + 1]] if n + 1 < len(leaders) else len(ins)
        blocks[a] = ins[b:e]
    succ = {}
    for a, bl in blocks.items():
        last = bl[-1]
        s = []
        if last.mnemonic.startswith('ret'):
            pass
        elif last.group(capstone.CS_GRP_JUMP):
            if last.operands[0].type == capstone.x86.X86_OP_IMM:
                s.append(last.operands[0].imm)
            if last.mnemonic != 'jmp':
                s.append(last.address + last.size)
            m = re.search(r'\[(e\w\w)\*4 \+ (0x[0-9a-f]+)\]', last.op_str)
            if last.mnemonic == 'jmp' and m:
                t = int(m.group(2), 16)
                while True:
                    v = int.from_bytes(img[t - base:t - base + 4], 'little')
                    if not start <= v < end:
                        break
                    s.append(v)
                    t += 4
        else:
            s.append(last.address + last.size)
        succ[a] = [x for x in s if x in blocks]

    def slots(i):
        """(reads, writes, addr-taken) as sets of ebp-relative byte offsets (negative)."""
        rd, wr, lea = set(), set(), set()
        ops = i.operands
        for n, op in enumerate(ops):
            if op.type != X86_OP_MEM or op.mem.base != X86_REG_EBP or op.mem.index != 0 or op.mem.disp >= 0:
                continue
            rng = set(range(op.mem.disp, op.mem.disp + op.size))
            if i.mnemonic == 'lea':
                lea |= rng
                continue
            dst = n == 0 and len(ops) >= 1
            mnem = i.mnemonic
            if mnem in ('mov', 'movsx', 'movzx') or mnem.startswith('fst') or mnem.startswith('fist') or \
                    mnem.startswith('set') or mnem == 'fnstcw' or mnem == 'fnstsw' or mnem == 'pop':
                (wr if dst else rd).update(rng)
            elif mnem in ('cmp', 'test', 'push') or mnem.startswith('fld') or mnem.startswith('fild') or \
                    mnem.startswith('fadd') or mnem.startswith('fsub') or mnem.startswith('fmul') or \
                    mnem.startswith('fdiv') or mnem.startswith('fcom') or mnem.startswith('ficom') or \
                    mnem == 'fldcw' or mnem.startswith('fi'):
                rd |= rng
            else:                                      # add, sub, and, or, inc, ... read and write
                rd |= rng
                if dst:
                    wr |= rng
        if i.mnemonic == 'fstp' or i.mnemonic == 'fst' or i.mnemonic == 'fistp' or i.mnemonic == 'fist':
            pass
        return rd, wr, lea

    ALL = None
    inn = {a: ALL for a in blocks}
    inn[start] = frozenset()
    out = {}
    changed = True
    while changed:
        changed = False
        for a in sorted(blocks):
            preds = [p for p in blocks if a in succ[p]]
            if a == start:
                cur = frozenset()
            else:
                vals = [out[p] for p in preds if p in out]
                if not vals:
                    continue
                cur = frozenset.intersection(*vals)
            w = set(cur)
            for i in blocks[a]:
                rd, wr, lea = slots(i)
                w |= wr | lea
            w = frozenset(w)
            if out.get(a) != w:
                out[a] = w
                changed = True
            inn[a] = cur
    leaked = set()
    hits = []
    for a in sorted(blocks):
        if inn[a] is None:
            continue
        w = set(inn[a])
        for i in blocks[a]:
            rd, wr, lea = slots(i)
            bad = rd - w
            if bad:
                hits.append((i.address, '%s %s' % (i.mnemonic, i.op_str), sorted(bad)))
            w |= wr | lea
            leaked |= lea
    return hits, leaked


for k in range(2, len(sys.argv), 2):
    s, e = int(sys.argv[k], 16), int(sys.argv[k + 1], 16)
    hits, leaked = scan(s, e)
    print('== %08x' % s)
    for a, txt, bad in hits:
        lo, hi = min(bad), max(bad)
        print('  %08x  %-40s unset ebp%+#x..%+#x' % (a, txt, lo, hi))
    if leaked:
        print('  address taken:', ', '.join('%#x' % -x for x in sorted({x & ~3 for x in leaked}, reverse=True)[:30]))


# ---- second pass: the bytes of each address-taken record that the function never writes (any path), between the
# record's lea offset and the next local the function names; a callee (or the renderer) may read them.
def never_written(start, end):
    ins = list(md.disasm(img[start - base:end - base], start))
    named, written, leas = set(), set(), set()
    for i in ins:
        for n, op in enumerate(i.operands):
            if op.type != X86_OP_MEM or op.mem.base != X86_REG_EBP or op.mem.index != 0 or op.mem.disp >= 0:
                continue
            named.add(op.mem.disp)
            if i.mnemonic == 'lea':
                leas.add(op.mem.disp)
            elif n == 0 and i.mnemonic not in ('cmp', 'test', 'push') and not i.mnemonic.startswith('fld') \
                    and not i.mnemonic.startswith('fild') and not i.mnemonic.startswith('fcom'):
                written.update(range(op.mem.disp, op.mem.disp + op.size))
    out = []
    for b in sorted(leas):
        nxt = min([d for d in named if d > b] + [0])
        hole = [x for x in range(b, nxt) if x not in written]
        if hole:
            out.append((b, nxt, hole))
    return out


if __name__ == '__main__' and '--records' in sys.argv:
    pass
