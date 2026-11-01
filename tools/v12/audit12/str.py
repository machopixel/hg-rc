import sys, pefile
p = {'10': '../GameRip/HGSIM.DLL', '12': '../GameRip_12/hgsim.dll'}[sys.argv[1]]
pe = pefile.PE(p); img = pe.get_memory_mapped_image(); b = pe.OPTIONAL_HEADER.ImageBase
for x in sys.argv[2:]:
    a = int(x, 16) - b
    e = img.find(b'\0', a)
    print(x, repr(img[a:e][:120]))
