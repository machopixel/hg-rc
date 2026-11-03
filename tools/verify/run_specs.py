"""Runs many spec files at once, one difftest process per spec file, on all CPU cores.

    python -I tools/verify/run_specs.py [-jN] LIST LOGDIR

LIST: one spec per line, "tools/verify/spec_X.py[<TAB>name name ...]" (no names: every entry of the file).
Each spec file logs into LOGDIR/<spec name>/ (its own index, so parallel runs never share a file); the one-line
results are collected, in LIST order, into LOGDIR/index.txt and printed. Exit status 1 if any entry failed.
"""
import os, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
DLL = os.path.join(HERE, '..', '..', '..', 'GameRip', 'HGSIM.DLL')


def run(item, logroot):
    spec, names = item
    stem = os.path.splitext(os.path.basename(spec))[0]
    env = dict(os.environ, DIFFTEST_LOGDIR=os.path.join(logroot, stem))
    t = time.time()
    r = subprocess.run([sys.executable, '-I', os.path.join(HERE, 'difftest.py'), DLL, spec] + names,
                       capture_output=True, text=True, env=env)
    lines = [l for l in r.stdout.splitlines() if l.startswith(('PASS ', 'FAIL '))]
    if not lines:       # build error or crash: keep the tail of the output
        lines = ['FAIL %s (no result) %s' % (stem, (r.stdout + r.stderr).strip().splitlines()[-1:] or '')]
    return spec, lines, time.time() - t


def main():
    args = sys.argv[1:]
    jobs = 8            # Diego, 2026-11-03: at most 8 processes
    if args and args[0].startswith('-j'):
        jobs = min(int(args.pop(0)[2:]), 8)
    lst, logroot = args
    items = []
    for line in open(lst):
        line = line.strip()
        if line and not line.startswith('#'):
            p = line.split('\t')
            items.append((p[0], p[1].split() if len(p) > 1 else []))
    os.makedirs(logroot, exist_ok=True)
    t0 = time.time()
    # bring the shared object cache (build/w10cache) up to date once, so the parallel runs never rebuild it together
    subprocess.check_call([sys.executable, '-I', '-c',
                           "exec(open('tools/verify/w10_common.py').read()); closure([])"])
    # longest first is unknown; start them in list order
    with ThreadPoolExecutor(jobs) as ex:
        res = list(ex.map(lambda it: run(it, logroot), items))
    out = []
    for spec, lines, dt in res:
        for l in lines:
            out.append('%s  [%s, %.0fs]' % (l, os.path.basename(spec), dt))
    with open(os.path.join(logroot, 'index.txt'), 'w') as fo:
        fo.write('\n'.join(out) + '\n')
    print('\n'.join(out))
    fails = sum(l.startswith('FAIL') for l in out)
    print('%d spec files, %d entries, %d failed, %.0f s on %d processes' % (len(items), len(out), fails,
                                                                           time.time() - t0, jobs))
    sys.exit(1 if fails else 0)


if __name__ == '__main__':
    main()
