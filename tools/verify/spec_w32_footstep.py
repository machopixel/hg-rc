# Round 33: only the footstep dust's specs of spec_w32_explode.py (1.2 0x10086007, 0x10085992, 0x10085b34) and the
# skate dust's spawn they share code with (0x1007e25c, changed into W32e_Dust). Same negative checks (W32_NEG=1).
exec(open('tools/verify/spec_w32_explode.py').read())
SPECS = [s for s in SPECS if s['name'] in ('Sim12_10086007', 'Sim12_10085992', 'Sim12_10085b34', 'Sim12_1007e25c')]
