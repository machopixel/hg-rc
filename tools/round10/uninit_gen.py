import pefile
pe = pefile.PE(r'C:\Coding\HeavyGear\Project\HGSIM.DLL', fast_load=True)
B = pe.OPTIONAL_HEADER.ImageBase
img = pe.get_memory_mapped_image()
F = [  # addr, prologue length, offsets (negative = word), what
 (0x10020055, 6, [4], 'zeroes a sound slot volume'),
 (0x10021e8c, 9, [0x8c, 0x94, 8], 'camera target'),
 (0x100232d7, 9, [0x2c, 0xdc], 'camera tracking'),
 (0x100687a3, 6, [0x18, 0x1c, 4], 'within-radius test'),
 (0x100b66a0, 6, [0x10], 'HUD drawing (divides by it)'),
 (0x100c2ae5, 6, [0xc], 'hit test'),
 (0x1000aa9e, 6, [0x64, 4], 'hit / geometry lookup'),
 (0x10055048, 6, [-4], 'input device'),
 (0x1005ab44, 6, [0x10], 'node-name search'),
 (0x1005bb7a, 9, [0xc0, 0xa8, 0x84], 'POF model loader'),
 (0x10067ec6, 6, [4], 'kill credit'),
 (0x10089066, 6, [4], 'node-tree bounds'),
 (0x1008950e, 6, [4], 'node-tree bounds 2'),
 (0x100918e6, 9, [], 'renderer vertices'),
 (0x1009221d, 9, [], 'renderer vertices'),
 (0x1009353e, 9, [], 'renderer vertices'),
 (0x100945b8, 9, [], 'renderer vertices'),
 (0x1009596a, 9, [], 'renderer vertices'),
 (0x1009fada, 8, [], 'renderer vertex buffer'),
 (0x100a0880, 9, [], 'clip vertices'),
 (0x100ac9ab, 8, [], 'clip vertices'),
 (0x100a65e7, 9, [0x78], 'terrain?'),
 (0x100b8d6c, 6, [-0x10], 'resource name lookup'),
 (0x100c0158, 6, [0x14], 'selection'),
 (0x100c7120, 9, [0x28, 0x70, 0x74, 0x78, 0x7c], 'HUD range readout'),
]
S = [  # function index, tap address, stolen bytes, read instruction, offsets
 (0, 0x1002007a, 7, 0x1002007d, [4]),
 (1, 0x1002258a, 10, 0x1002258a, [8, 0x94, 0x8c]),
 (2, 0x10023d35, 9, 0x10023d35, [0x2c]),
 (2, 0x10023ad6, 6, 0x10023ad6, [0xdc]),
 (3, 0x100687df, 9, 0x100687df, [0x18, 0x1c, 4]),
 (4, 0x100b676b, 7, 0x100b676e, [0x10]),
 (4, 0x100b6796, 6, 0x100b679c, [0x10]),
 (5, 0x100c2d40, 7, 0x100c2d60, [0xc]),
 (5, 0x100c2f60, 7, 0x100c2f7c, [0xc]),
 (6, 0x1000aafd, 7, 0x1000aafd, [4, 0x64]),
 (7, 0x100556a0, 10, 0x100556a0, [-4]),
 (8, 0x1005acb8, 7, 0x1005acb8, [0x10]),
 (8, 0x1005ad1f, 5, 0x1005ad20, [0x10]),
 (8, 0x1005ad94, 5, 0x1005ad95, [0x10]),
 (9, 0x1005bf24, 6, 0x1005bf24, [0xc0]),
 (9, 0x1005c4f0, 6, 0x1005c4f0, [0x84]),
 (9, 0x1005c8a9, 6, 0x1005c8a9, [0xa8, 0xc0]),
 (10, 0x10067f4d, 5, 0x10067f4e, [4]),
 (10, 0x1006805e, 5, 0x1006805f, [4]),
 (10, 0x100680b2, 5, 0x100680b3, [4]),
 (11, 0x1008926a, 6, 0x1008926a, [4]),
 (11, 0x1008930f, 7, 0x1008930f, [4]),
 (12, 0x10089712, 6, 0x10089712, [4]),
 (12, 0x100897b1, 7, 0x100897b1, [4]),
 (21, 0x100a695f, 6, 0x100a695f, [0x78]),
 (22, 0x100b8e7e, 7, 0x100b8e7e, [-0x10]),
 (23, 0x100c034c, 5, 0x100c034e, [0x14]),
 (24, 0x100c786a, 7, 0x100c786d, [0x28]),
 (24, 0x100c778b, 6, 0x100c778e, [0x70]),
]
def hexb(a, n):
    return '{' + ', '.join('0x%02X' % b for b in img[a - B:a - B + n]) + '}'
def offs(o):
    o = list(o) + [0] * (5 - len(o))
    return '{' + ', '.join(('-0x%X' % -x if x < 0 else '0x%X' % x) for x in o) + '}'
out = ['/* Generated from HGSIM.DLL by tools/round10/uninit_gen.py: the watched functions (prologue bytes as in the DLL). */',
       'static UninitFn g_fns[] = {']
for a, n, o, w in F:
    out.append('    {0x%08X, %d, %s, %d, %s, "%s"},' % (a, n, hexb(a, n), len(o), offs(o), w))
out.append('};')
out.append('')
out.append('/* The bad-read tap points: function index, tap address (whole instructions, no relative jumps, no branch or')
out.append(' * jump-table target inside), the read instruction, and the locals that instruction reads. */')
out.append('static UninitSite g_sites[] = {')
for f, a, n, r, o in S:
    out.append('    {%d, 0x%08X, %d, %s, 0x%08X, %d, %s},' % (f, a, n, hexb(a, n), r, len(o), offs(o)))
out.append('};')
open(r'src\hooks\uninit_table.inc', 'w', newline='\n').write('\n'.join(out) + '\n')
print(len(F), 'functions', len(S), 'sites')
