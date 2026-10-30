# Round 17: which stack bytes a function (with everything it calls) can write, relative to its entry esp.
#   python -I tools/round17/footprint.py <HGSIM.DLL> <starts.txt> <function> <offset> [<offset> ...]
# Offsets are relative to the entry esp of <function> ([esp] = return address, so locals are negative). For each
# offset prints the call chains whose instructions can write it (direct [ebp/esp+d] stores, pushes, stos via a
# stack lea), and the flags that make the answer incomplete (indirect calls, lea of a local passed on).
import sys, bisect, capstone
from capstone import x86
import pefile
pe = pefile.PE(sys.argv[1])
base = pe.OPTIONAL_HEADER.ImageBase
img = pe.get_memory_mapped_image()
starts = sorted(int(l, 16) for l in open(sys.argv[2]) if l.strip())
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
md.detail = True
WRITES = {'mov', 'fstp', 'fst', 'fistp', 'fist', 'fnstsw', 'fnstcw', 'add', 'sub', 'and', 'or', 'xor', 'inc', 'dec',
          'movsx', 'movzx', 'neg', 'not', 'shl', 'shr', 'sar', 'adc', 'sbb', 'fstp', 'fbstp', 'setne', 'sete',
          'setl', 'setg', 'setle', 'setge', 'seta', 'setb', 'pop', 'xchg', 'fnstenv', 'fnsave', 'fsave', 'fstsw',
          'fstcw', 'imul', 'rol', 'ror'}


def thunk(t):
    i = next(md.disasm(img[t - base:t - base + 8], t), None)
    if i and i.mnemonic == 'jmp' and i.op_str.startswith('0x'):
        return int(i.op_str, 16)
    return t


def end_of(f):
    k = bisect.bisect_right(starts, f)
    return starts[k] if k < len(starts) else f + 0x4000


memo = {}


def fp(f, depth=0):
    """{offset: [chain]} of writable bytes relative to f's entry esp, and a set of flags."""
    if f in memo:
        return memo[f]
    memo[f] = ({}, {'recursion %x' % f})
    out, flags = {}, set()
    ins = list(md.disasm(img[f - base:end_of(f) - base], f))
    # esp offset at each instruction, following fall-through and jumps (first value wins)
    at = {i.address: i for i in ins}
    sp = {f: 0}
    ebp = None
    todo = [f]
    seen = set()
    while todo:
        a = todo.pop()
        if a in seen or a not in at:
            continue
        seen.add(a)
        i = at[a]
        s = sp[a]
        m = i.mnemonic
        nxt = a + i.size
        ops = i.operands

        def add(off, n, who):
            for k in range(n):
                out.setdefault(off + k, []).append(who)
        if m == 'push':
            s -= 4
            add(s, 4, '%x' % a)
        elif m == 'pop':
            s += 4
        elif m == 'sub' and ops[0].type == x86.X86_OP_REG and i.reg_name(ops[0].reg) == 'esp':
            s -= ops[1].imm if ops[1].type == x86.X86_OP_IMM else 0
        elif m == 'add' and ops[0].type == x86.X86_OP_REG and i.reg_name(ops[0].reg) == 'esp':
            s += ops[1].imm if ops[1].type == x86.X86_OP_IMM else 0
        elif m == 'mov' and i.op_str == 'ebp, esp':
            ebp = s
        elif m == 'mov' and i.op_str == 'esp, ebp':
            s = ebp
        elif m == 'call':
            if ops[0].type == x86.X86_OP_IMM:
                g = thunk(ops[0].imm)
                if g == 0x100fcb40 or g == 0x100fc9e0:     # __chkstk-like probes are not expected here
                    flags.add('probe %x' % g)
                sub, sf = fp(g, depth + 1)
                for off, ch in sub.items():
                    for c in ch[:6]:
                        out.setdefault(s - 4 + off, []).append('%x>%x:%s' % (a, g, c))
                flags |= {'%x>%s' % (g, x) for x in sf}
            else:
                flags.add('indirect call at %x' % a)
        elif m in ('ret',):
            continue
        elif m == 'lea' and ops[1].type == x86.X86_OP_MEM and i.reg_name(ops[1].mem.base) in ('ebp', 'esp'):
            flags.add('lea of a local at %x (%s)' % (a, i.op_str))
        elif m.startswith('rep') or m.startswith('stos') or m.startswith('movs'):
            flags.add('string op at %x' % a)
        if m in WRITES and m not in ('push', 'pop') and ops and ops[0].type == x86.X86_OP_MEM:
            r = i.reg_name(ops[0].mem.base) if ops[0].mem.base else None
            if r == 'ebp' and ebp is not None:
                add(ebp + ops[0].mem.disp, ops[0].size, '%x' % a)
            elif r == 'esp':
                add(s + ops[0].mem.disp, ops[0].size, '%x' % a)
        if m == 'jmp':
            if ops[0].type == x86.X86_OP_IMM:
                t = ops[0].imm
                sp.setdefault(t, s)
                todo.append(t)
            else:
                flags.add('indirect jmp at %x' % a)
            continue
        if m.startswith('j') or m.startswith('loop'):
            t = ops[0].imm
            sp.setdefault(t, s)
            todo.append(t)
        sp.setdefault(nxt, s)
        todo.append(nxt)
    memo[f] = (out, flags)
    return memo[f]


f = int(sys.argv[3], 16)
out, flags = fp(f)
for o in sys.argv[4:]:
    off = int(o, 0)
    ch = out.get(off, [])
    print('offset %#x: %d writer(s)' % (off & 0xffffffff if off >= 0 else -(-off), len(ch)))
    for c in ch[:400]:
        print('   ', c)
print('lowest written offset: %#x' % -min(out) if out and min(out) < 0 else '')
for x in sorted(flags)[:60]:
    print('  flag', x)
