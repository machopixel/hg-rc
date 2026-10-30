# Round 16: can instruction `target` be reached from the function entry without passing instruction `via`?
#   python -I tools/round16/reach_addr.py <HGSIM.DLL> <function> <end> <via> <target> [<target> ...]
import sys, pefile, capstone
pe = pefile.PE(sys.argv[1])
base = pe.OPTIONAL_HEADER.ImageBase
img = pe.get_memory_mapped_image()
fa, end, via = (int(x, 16) for x in sys.argv[2:5])
targets = [int(x, 16) for x in sys.argv[5:]]
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
ins = {i.address: i for i in md.disasm(img[fa - base:end - base], fa)}
order = sorted(ins)
idx = {a: n for n, a in enumerate(order)}
seen, todo = set(), [fa]
while todo:
    a = todo.pop()
    if a in seen or a not in ins or a == via:
        continue
    seen.add(a)
    i = ins[a]
    if i.mnemonic == 'ret':
        continue
    if i.mnemonic.startswith('j') and i.op_str.startswith('0x'):
        todo.append(int(i.op_str, 16))
        if i.mnemonic == 'jmp':
            continue
    elif i.mnemonic == 'jmp':
        print('indirect jump at', hex(a))
        continue
    if idx[a] + 1 < len(order):
        todo.append(order[idx[a] + 1])
for t in targets:
    print(hex(t), 'reachable without via' if t in seen else 'not reachable without via')
