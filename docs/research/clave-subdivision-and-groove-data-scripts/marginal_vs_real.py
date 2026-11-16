"""Does drawing each sixteenth independently from a style's per-position odds (what Bloodline Breaks
does) give bars that look like real beats? Compare with GMD bars.

Usage: python3 -I marginal_vs_real.py <path to unzipped 'groove' folder>
"""
import collections
import csv
import os
import random
import sys

import mido

ROOT = sys.argv[1]
W = [5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1]  # LHL-style weights (GrooveToolbox profile)


def lhl(bar):
    # syncopation of one 16-step line, looped: onset followed by a rest on a stronger position
    s = 0
    for i in bar:
        for d in (1, 2):
            j = (i + d) % 16
            if j not in bar and W[j] > W[i]:
                s += W[j] - W[i]
                break
            if j in bar:
                break
    return s


def jacc(a, b):
    u = a | b
    return 1.0 if not u else len(a & b) / len(u)


random.seed(1)
for style in ('hiphop', 'funk', 'rock', 'soul'):
    rows = [r for r in csv.DictReader(open(os.path.join(ROOT, 'info.csv')))
            if r['beat_type'] == 'beat' and r['time_signature'] == '4-4' and r['style'].split('/')[0] == style]
    files = []
    for r in rows:
        bpm = float(r['bpm'])
        bars = int(float(r['duration']) * bpm / 240.0)
        if bars < 4:
            continue
        ons = []
        t = 0.0
        for msg in mido.MidiFile(os.path.join(ROOT, r['midi_filename'])):
            t += msg.time
            if msg.type == 'note_on' and msg.velocity > 0 and msg.note in (36, 38, 40, 37):
                ons.append((t, msg.note, msg.velocity))
        sv = sorted(v for _, n, v in ons if n != 36)
        thr = 0.65 * sv[int(0.95 * (len(sv) - 1))] if sv else 999
        k = [set() for _ in range(bars)]
        s = [set() for _ in range(bars)]
        for t, n, v in ons:
            q = round(t * bpm / 15.0)
            b, p = divmod(q, 16)
            if b >= bars:
                continue
            if n == 36:
                k[b].add(p)
            elif v >= thr:
                s[b].add(p)
        bl = [frozenset([('k', p) for p in k[i]] + [('s', p) for p in s[i]]) for i in range(bars)]
        mode = collections.Counter(x for x in bl if x).most_common(1)[0][0]
        files.append((bl, mode, k, s))
    # pooled marginal per position
    nb = sum(len(f[0]) for f in files)
    pk = [sum(p in kb for f in files for kb in f[2]) / nb for p in range(16)]
    ps = [sum(p in sb for f in files for sb in f[3]) / nb for p in range(16)]
    modes = [f[1] for f in files]
    real_best, real_own, samp_best = [], [], []
    real_sync, samp_sync = [], []
    for i, (bl, mode, k, s) in enumerate(files):
        others = modes[:i] + modes[i + 1:]
        for b in bl:
            if not b:
                continue
            real_own.append(jacc(b, mode))
            real_best.append(max(jacc(b, m) for m in others))
        real_sync.append(lhl({p for (x, p) in mode if x == 'k'}))
    for _ in range(3000):
        kb = {p for p in range(16) if random.random() < pk[p]}
        sb = {p for p in range(16) if random.random() < ps[p]}
        b = frozenset([('k', p) for p in kb] + [('s', p) for p in sb])
        samp_best.append(max(jacc(b, m) for m in modes))
        samp_sync.append(lhl(kb))
    m = lambda x: sum(x) / len(x)
    print(f'{style:7s} files={len(files)} | real bar vs own modal bar J={m(real_own):.2f} | real bar vs best other '
          f'beat J={m(real_best):.2f} | independent draw vs best real beat J={m(samp_best):.2f} | '
          f'kick LHL: real modes {m(real_sync):.1f}, draws {m(samp_sync):.1f} | onsets/bar: kick {sum(pk):.1f}, snare acc {sum(ps):.1f}')
