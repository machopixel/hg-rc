import json,re,glob,os
HERE=os.path.dirname(os.path.abspath(__file__))
A=json.load(open(os.path.join(HERE,'audit.json'))); C=json.load(open(os.path.join(HERE,'cside.json')))
S={k:set(int(x,16) for x in v) for k,v in C.items()}
idx={}
for p in glob.glob('src/**/*.c',recursive=True):
    t=open(p,encoding='utf-8',errors='replace').read()
    for m in re.finditer(r'FUNCTION: HGSIM (0x1[0-9a-f]{7})',t): idx[int(m.group(1),16)]=p.split('src')[1]
K={k['a12']:k for k in json.load(open(os.path.join(HERE,'classify.json')))}
out=[]
for r in A['rows']:
    if not r['logic']: continue
    a10=int(r['a10'],16); a12=int(r['a12'],16); e=[]
    if a10 in S['conv'] or a10 in S['marks']: e.append('conv')
    if a12 in S['cite12'] or a12 in S['sim12']: e.append('cite12')
    k=K[r['a12']]; k['ev']=e; k['c']=idx.get(a10,'-'); k['sim']=r['sim']; out.append(k)
json.dump(out,open(os.path.join(HERE,'classify.json'),'w'),indent=0)
from collections import Counter
print(Counter((','.join(k['ev']),k['kind']) for k in out))
