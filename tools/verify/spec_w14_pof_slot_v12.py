# Round 33, round 4: the gear set-up (1.0 0x1005bb7a, 1.2 0x1004e6a0) after 1.2's one change, the selected weapon
# slot +0x5fb started at -1 (1.2 0x1004e837, right after +0xc15 = 0 and before the 0x28-byte clear of +0x498; the
# kind-10 part still sets it to its mount number later, 1.2 0x1004f0ba, as 1.0). 1.2's other differences in this
# function are its re-packed part tables (same values) and one debug print through its empty log function, so the
# 1.0 spec is the comparison: with W14PW_SLOT unset the C is built with W14PW_MUTATE (the store left out) and must
# match 1.0 exactly (nothing else changed); with W14PW_SLOT=1 the store is in and the gear's +0x5fb must differ
# from 1.0 exactly where no kind-10 part sets it (the negative: mismatches expected).
import os
exec(open('tools/verify/spec_w14_1005bb7a.py').read())
if not os.environ.get('W14PW_SLOT'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW14PW_MUTATE']
