import sys, capstone, pefile
p = {'10': '../GameRip/HGSIM.DLL', '12': '../GameRip_12/hgsim.dll'}[sys.argv[1]]
pe = pefile.PE(p); img = pe.get_memory_mapped_image(); b = pe.OPTIONAL_HEADER.ImageBase
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
a, e = int(sys.argv[2], 16), int(sys.argv[3], 16)
for i in md.disasm(img[a - b:e - b], a):
    print('%x  %s %s' % (i.address, i.mnemonic, i.op_str))
