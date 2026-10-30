# Round 16: can the call to `target` be reached from the function entry without passing the call to `via`?
#   python -I tools/round16/reach.py <HGSIM.DLL> <function> <end> <via callee> <target callee>
# Follows jumps and fall-through within [function, end); a call to `via` blocks its path.
import sys, re, pefile, capstone
pe = pefile.PE(sys.argv[1])
base = pe.OPTIONAL_HEADER.ImageBase
img = pe.get_memory_mapped_image()
fa, end, via, target = (int(x, 16) for x in sys.argv[2:6])
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
ins = {i.address: i for i in md.disasm(img[fa - base:end - base], fa)}
order = sorted(ins)


def callee(i):
    if i.mnemonic != 'call' or not i.op_str.startswith('0x'):
        return None
    t = int(i.op_str, 16)
    j = next(md.disasm(img[t - base:t - base + 8], t), None)
    return int(j.op_str, 16) if j and j.mnemonic == 'jmp' and j.op_str.startswith('0x') else t


seen, todo, hits = set(), [fa], []
while todo:
    a = todo.pop()
    if a in seen or a not in ins:
        continue
    seen.add(a)
    i = ins[a]
    c = callee(i)
    if c == via:
        continue
    if c == target:
        hits.append(hex(a))
    nxt = order[order.index(a) + 1] if order.index(a) + 1 < len(order) else None
    if i.mnemonic == 'ret':
        continue
    if i.mnemonic.startswith('j') and i.op_str.startswith('0x'):
        todo.append(int(i.op_str, 16))
        if i.mnemonic == 'jmp':
            continue
    if nxt:
        todo.append(nxt)
print('target reachable without via:', hits or 'no')
