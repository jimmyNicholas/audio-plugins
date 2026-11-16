"""Timeline catalogue checks (Euclidean generation, evenness, rotations) and a search for clave
patterns in GMD latin / afrocuban cross-stick (37) and snare parts.

Usage: python3 -I timelines_and_gmd_clave.py <path to unzipped 'groove' folder>
"""
import collections
import csv
import math
import os
import sys

import mido


def bjorklund(k, n):
    # Bresenham form: onset where floor((i*k)/n) steps; gives a rotation of E(k,n)
    return sorted({(i * n) // k for i in range(k)})


def rot(p, n, r):
    return sorted((x - r) % n for x in p)


def necklace_eq(a, b, n):
    return any(rot(a, n, r) == sorted(b) for r in range(n))


def iois(p, n):
    return [(p[(i + 1) % len(p)] - p[i]) % n or n for i in range(len(p))]


TL16 = {  # 16-step, one 4/4 bar of sixteenths per 2-bar clave cycle written in 2/2 (cut time)
    'son clave': [0, 3, 6, 10, 12],
    'rumba clave': [0, 3, 7, 10, 12],
    'bossa nova clave': [0, 3, 6, 10, 13],
    'gahu': [0, 3, 6, 10, 14],
    'shiko': [0, 4, 6, 10, 12],
    'soukous': [0, 3, 6, 10, 11],
    'tresillo x2 (3-3-2)': [0, 3, 6, 8, 11, 14],
    'dembow snare (reggaeton)': [3, 6, 11, 14],
    'backbeat': [4, 12],
    'four on the floor': [0, 4, 8, 12],
}
TL12 = {
    'standard bell (bembe, 7 of 12)': [0, 2, 4, 5, 7, 9, 11],
    'fume-fume / soli (5 of 12)': [0, 2, 4, 7, 9],
}

print('## Timelines: IOIs, Euclidean necklace match, onsets 8 (or n/2) apart')
for name, p in list(TL16.items()) + list(TL12.items()):
    n = 16 if name in TL16 else 12
    k = len(p)
    e = bjorklund(k, n)
    half = any(((x + n // 2) % n) in p for x in p)
    print(f'{name:32s} n={n} k={k} onsets={p} IOI={iois(p, n)} '
          f'E({k},{n})-necklace={necklace_eq(p, e, n)} has-antipodal-pair={half}')
print('E(3,8)=', bjorklund(3, 8), ' E(5,8)=', bjorklund(5, 8), ' E(5,16)=', bjorklund(5, 16),
      ' E(7,12)=', bjorklund(7, 12), ' E(5,12)=', bjorklund(5, 12), ' E(7,16)=', bjorklund(7, 16),
      ' E(9,16)=', bjorklund(9, 16))

# ---- GMD search ----
ROOT = sys.argv[1]
rows = [r for r in csv.DictReader(open(os.path.join(ROOT, 'info.csv')))
        if r['beat_type'] == 'beat' and r['time_signature'] == '4-4'
        and r['style'].split('/')[0] in ('latin', 'afrocuban')]
CL = {k: v for k, v in TL16.items() if 'clave' in k or k in ('gahu', 'shiko', 'soukous')}
# 2-bar cycle in 4/4 sixteenths: each clave step = an eighth note (32 sixteenths per cycle)
# and also the 1-bar "cut-time" reading (16 sixteenths per cycle). Test both, both directions.
print('\n## GMD latin/afrocuban beats: best clave match per part (2-bar eighth-note reading and 1-bar sixteenth reading)')
summary = collections.Counter()
for r in rows:
    bpm = float(r['bpm'])
    bars = int(float(r['duration']) * bpm / 240.0)
    parts = collections.defaultdict(lambda: collections.Counter())
    nb = collections.defaultdict(set)
    for msg in mido.MidiFile(os.path.join(ROOT, r['midi_filename'])):
        pass
    t = 0.0
    for msg in mido.MidiFile(os.path.join(ROOT, r['midi_filename'])):
        t += msg.time
        if msg.type == 'note_on' and msg.velocity > 0:
            q = round(t * bpm / 60.0 * 4.0)
            name = {37: 'xstick', 38: 'snare', 40: 'rim', 36: 'kick', 51: 'ride', 53: 'bell', 59: 'ride'}.get(msg.note)
            if name and q // 16 < bars:
                parts[name][q % 32] += 1  # 2-bar cycle position (32 sixteenths)
                nb[name].add(q // 32)
    best = []
    ncyc = max(1, bars // 2)
    for name, c in parts.items():
        prof = {p: c[p] / ncyc for p in range(32)}
        if sum(prof.values()) < 2:
            continue
        for cname, cp in CL.items():
            for orient in (0, 8):  # 3-2 or 2-3 (start on the other half)
                tmpl = {((x * 2) + orient * 2) % 32 for x in cp}  # eighth-note reading over two bars
                hit = sum(prof[p] for p in tmpl) / len(tmpl)
                extra = sum(v for p, v in prof.items() if p not in tmpl)
                best.append((hit - 0.25 * extra, hit, extra, name, cname, '3-2' if orient == 0 else '2-3', '2-bar'))
            for orient in (0, 8):
                tmpl = {(x + orient) % 16 for x in cp}
                tm = tmpl | {x + 16 for x in tmpl}
                hit = sum(prof[p] for p in tm) / len(tm)
                extra = sum(v for p, v in prof.items() if p not in tm)
                best.append((hit - 0.25 * extra, hit, extra, name, cname, '3-2' if orient == 0 else '2-3', '1-bar'))
    best.sort(reverse=True)
    if best and best[0][1] >= 0.7:
        s, hit, extra, name, cname, o, rd = best[0]
        summary[(cname, rd)] += 1
        print(f"{r['midi_filename'][:60]:60s} {r['bpm']:>4s} {name:6s} {cname} {o} {rd}: "
              f'template hit rate {hit:.2f}, other onsets per cycle {extra:.1f}')
    else:
        summary['no clave part'] += 1
print('\nSummary:', dict(summary), 'of', len(rows), 'files')
