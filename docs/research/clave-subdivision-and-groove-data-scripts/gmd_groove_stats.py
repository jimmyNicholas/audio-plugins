"""Groove statistics from the Magenta Groove MIDI Dataset (GMD v1.0.0, CC BY 4.0).

Usage: python3 -I gmd_groove_stats.py <path to unzipped 'groove' folder> > out.txt

Uses only 'beat' files in 4-4. Grid: the MIDI tempo (equal to info.csv bpm), with beat 1 at time 0
(the GMD MIDI is aligned to the metronome; first notes land a few ticks after 0).
Pitch classes follow Magenta's ROLAND_DRUM_PITCH_CLASSES (music_vae/data.py).
"""
import collections
import csv
import math
import os
import statistics as st
import sys

import mido

ROOT = sys.argv[1]
CLS = {36: 'kick', 38: 'snare', 37: 'snare', 40: 'snare', 42: 'hhc', 22: 'hhc', 44: 'hhp',
       46: 'hho', 26: 'hho', 51: 'ride', 53: 'ride', 59: 'ride', 43: 'tom', 58: 'tom', 47: 'tom',
       45: 'tom', 50: 'tom', 48: 'tom', 49: 'crash', 52: 'crash', 55: 'crash', 57: 'crash'}
STYLES = ['rock', 'funk', 'hiphop', 'soul', 'latin', 'jazz', 'afrobeat', 'pop', 'neworleans', 'afrocuban']


def onsets(path):
    t = 0.0
    out = []
    for msg in mido.MidiFile(path):
        t += msg.time
        if msg.type == 'note_on' and msg.velocity > 0 and msg.note in CLS:
            out.append((t, CLS[msg.note], msg.velocity, msg.note))
    return out


def mean(x):
    return sum(x) / len(x) if x else float('nan')


def sd(x):
    return st.pstdev(x) if len(x) > 1 else float('nan')


def jacc(a, b):
    u = a | b
    return 1.0 if not u else len(a & b) / len(u)


rows = [r for r in csv.DictReader(open(os.path.join(ROOT, 'info.csv')))
        if r['beat_type'] == 'beat' and r['time_signature'] == '4-4']

# accumulators
dev = collections.defaultdict(list)       # (style, inst, pos16) -> list of (dev16, devms)
vel = collections.defaultdict(list)       # (style, inst, pos16) -> velocities
present = collections.defaultdict(lambda: [0, 0])  # (style, inst, pos16) -> [bars with onset, bars]
dens = collections.defaultdict(list)      # (style, inst) -> onsets per bar
swing = collections.defaultdict(list)     # (style, kind) -> per-file swing (0.5 straight)
consist = collections.defaultdict(list)   # (style, layer, metric) -> per-file values
filemeans = collections.defaultdict(list)  # (style, inst, posclass) -> per-file mean dev ms
within = collections.defaultdict(list)    # (style, inst, posclass) -> per-file within sd ms
tempos = collections.defaultdict(list)
relative = collections.defaultdict(list)
pairsw = collections.defaultdict(list)
recall = collections.defaultdict(list)
phrase = collections.defaultdict(lambda: collections.defaultdict(list))
changehist = collections.defaultdict(collections.Counter)
nfiles = collections.Counter()
nbars = collections.Counter()

POSCLASS = lambda p: 'beat' if p % 4 == 0 else ('and' if p % 2 == 0 else 'e/a')

for r in rows:
    style = r['style'].split('/')[0]
    bpm = float(r['bpm'])
    dur = float(r['duration'])
    ms16 = 60000.0 / bpm / 4.0
    ons = onsets(os.path.join(ROOT, r['midi_filename']))
    bars = int(math.floor(dur * bpm / 60.0 / 4.0))
    if bars < 4 or not ons:
        continue
    nfiles[style] += 1
    nbars[style] += bars
    tempos[style].append(bpm)
    snare_v = sorted(v for _, i, v, _ in ons if i == 'snare')
    acc_thr = 0.65 * snare_v[int(0.95 * (len(snare_v) - 1))] if snare_v else 999
    layers = collections.defaultdict(lambda: [set() for _ in range(bars)])
    count = collections.defaultdict(lambda: [0] * bars)
    fdev = collections.defaultdict(list)
    # timekeeper = the most used of hhc / hho / ride
    tk_counts = collections.Counter(i for _, i, _, _ in ons if i in ('hhc', 'hho', 'ride'))
    tk = 'ride' if tk_counts['ride'] > tk_counts['hhc'] + tk_counts['hho'] else 'hat'
    tk_phase = []
    for t, inst, v, note in ons:
        b = t * bpm / 60.0
        q = round(b * 4.0)
        bar, pos = divmod(q, 16)
        if bar >= bars:
            continue
        d16 = b * 4.0 - q
        dev[(style, inst, pos)].append((d16, d16 * ms16))
        vel[(style, inst, pos)].append(v)
        count[inst][bar] += 1
        fdev[(inst, POSCLASS(pos))].append(d16 * ms16)
        hatlike = inst in ('hhc', 'hho') if tk == 'hat' else inst == 'ride'
        if hatlike:
            tk_phase.append(b)
            layers['timekeeper'][bar].add(pos)
        if inst == 'kick':
            layers['kick'][bar].add(pos)
            layers['skeleton'][bar].add(('k', pos))
        if inst == 'snare':
            if v >= acc_thr:
                layers['snare_accent'][bar].add(pos)
                layers['skeleton'][bar].add(('s', pos))
            else:
                layers['snare_ghost'][bar].add(pos)
    for inst, c in count.items():
        dens[(style, inst)].extend(c)
    # reference: the file's mean timekeeper deviation on the beat
    ref = fdev.get(('ride' if tk == 'ride' else 'hhc', 'beat'), [])
    if len(ref) >= 8:
        r0 = mean(ref)
        for (inst, pc), xs in fdev.items():
            if len(xs) >= 8:
                relative[(style, inst, pc)].append(mean(xs) - r0)
    # pairwise swing: an onset near an eighth (or quarter) start, one in the middle region, and the next start
    tb = sorted(tk_phase)
    for unit, key in ((0.5, 'pair16'), (1.0, 'pair8')):
        starts = {}
        for b in tb:
            k = round(b / unit)
            if abs(b / unit - k) < 0.12:
                starts[k] = b
        vals = []
        for b in tb:
            k = math.floor(b / unit)
            f = b / unit - k
            if 0.38 <= f <= 0.82 and k in starts and (k + 1) in starts:
                vals.append((b - starts[k]) / (starts[k + 1] - starts[k]))
        if len(vals) >= 12:
            pairsw[(style, key, tk)].append(st.median(vals))
    for (inst, pc), xs in fdev.items():
        if len(xs) >= 8:
            filemeans[(style, inst, pc)].append(mean(xs))
            within[(style, inst, pc)].append(sd(xs))
    # presence per position
    for inst in ('kick', 'snare', 'hhc', 'hho', 'ride', 'hhp'):
        hit = [set() for _ in range(bars)]
        for t, i, v, n in ons:
            if i != inst:
                continue
            q = round(t * bpm / 60.0 * 4.0)
            bar, pos = divmod(q, 16)
            if bar < bars:
                hit[bar].add(pos)
        for pos in range(16):
            k = (style, inst, pos)
            present[k][0] += sum(pos in h for h in hit)
            present[k][1] += bars
    # swing of the timekeeper
    if len(tk_phase) >= 16:
        ph = [b % 1.0 for b in tk_phase]
        mid = [p for p in ph if 0.12 < p < 0.38 or 0.80 < p < 0.88]
        if len(mid) / len(ph) < 0.12:  # eighth-based timekeeper
            ups = [p for p in ph if 0.4 <= p <= 0.8]
            if len(ups) >= 8:
                swing[(style, '8th:' + tk)].append(st.median(ups))
        else:  # sixteenth-based: phase within each eighth
            q8 = [(b * 2.0) % 1.0 for b in tk_phase]
            ups = [p for p in q8 if 0.35 <= p <= 0.85]
            if len(ups) >= 8:
                swing[(style, '16th:' + tk)].append(st.median(ups))
    # consistency of each layer against the file's modal bar (1-bar and 2-bar units)
    for layer, bl in layers.items():
        fz = [frozenset(x) for x in bl]
        nonempty = [x for x in fz if x]
        if len(nonempty) < 4:
            continue
        mode, mc = collections.Counter(nonempty).most_common(1)[0]
        consist[(style, layer, 'jacc1')].append(mean([jacc(x, mode) for x in nonempty]))
        consist[(style, layer, 'same1')].append(mc / len(nonempty))
        consist[(style, layer, 'symdiff1')].append(mean([len(x ^ mode) for x in nonempty]))
        consist[(style, layer, 'size')].append(mean([len(x) for x in nonempty]))
        pairs = [frozenset([(0, p) for p in fz[i]] + [(1, p) for p in fz[i + 1]])
                 for i in range(0, len(fz) - 1, 2)]
        pairs = [p for p in pairs if p]
        if len(pairs) >= 3:
            m2, c2 = collections.Counter(pairs).most_common(1)[0]
            consist[(style, layer, 'jacc2')].append(mean([jacc(x, m2) for x in pairs]))
        # share of the layer's onsets on positions used in >= 75% of bars
        pc = collections.Counter(p for x in fz for p in x)
        tot = sum(pc.values())
        anchors = {p for p, c in pc.items() if c >= 0.75 * len(fz)}
        if anchors:
            recall[(style, layer, 'recall')].append(mean([len(x & anchors) / len(anchors) for x in fz]))
            recall[(style, layer, 'extra')].append(mean([len(x - anchors) for x in fz]))
            recall[(style, layer, 'nanchors')].append(len(anchors))
        for bi, x in enumerate(fz):
            d = len(x ^ mode)
            phrase[(style, layer)][bi % 8].append(d)
            changehist[layer][min(d, 4)] += 1
        anchor = sum(c for p, c in pc.items() if c >= 0.75 * len(fz))
        consist[(style, layer, 'anchorshare')].append(anchor / tot if tot else float('nan'))

# ---- output ----
P = print
P('## Files and bars (beat files, 4-4, >= 4 bars)')
P('| style | files | bars | median BPM | BPM range |')
P('|---|---|---|---|---|')
for s in STYLES:
    if nfiles[s]:
        P(f'| {s} | {nfiles[s]} | {nbars[s]} | {st.median(tempos[s]):.0f} | {min(tempos[s]):.0f} to {max(tempos[s]):.0f} |')

P('\n## Onsets per bar (mean, SD across bars)')
insts = ['kick', 'snare', 'hhc', 'hho', 'hhp', 'ride', 'tom', 'crash']
P('| style | ' + ' | '.join(insts) + ' | total |')
P('|' + '---|' * (len(insts) + 2))
for s in STYLES:
    if not nfiles[s]:
        continue
    cells = []
    tot = 0
    for i in insts:
        x = dens.get((s, i), [])
        # bars without the instrument count as 0
        m = sum(x) / nbars[s]
        tot += m
        cells.append(f'{m:.1f}')
    P(f'| {s} | ' + ' | '.join(cells) + f' | {tot:.1f} |')

P('\n## Onset probability by sixteenth (tenths, like the Bloodline odds strings; 9 = 85% or more)')
for s in STYLES:
    if not nfiles[s]:
        continue
    for i in ('kick', 'snare', 'hhc', 'ride'):
        pr = [present[(s, i, p)][0] / max(1, present[(s, i, p)][1]) for p in range(16)]
        if max(pr) < 0.1:
            continue
        digits = ''.join(str(min(9, int(x * 10 + 0.5))) for x in pr)
        P(f'{s:10s} {i:6s} {digits}   ' + ' '.join(f'{x:.2f}' for x in pr))

P('\n## Mean velocity by sixteenth (blank: < 20 onsets)')
for s in STYLES:
    if not nfiles[s]:
        continue
    for i in ('kick', 'snare', 'hhc', 'ride'):
        cells = []
        for p in range(16):
            v = vel.get((s, i, p), [])
            cells.append(f'{mean(v):3.0f}' if len(v) >= 20 else '  .')
        if any(c.strip() != '.' for c in cells):
            P(f'{s:10s} {i:6s} ' + ' '.join(cells))

P('\n## Mean onset deviation from the nearest sixteenth, ms (positive = late), by position (blank: < 20)')
for s in STYLES:
    if not nfiles[s]:
        continue
    for i in ('kick', 'snare', 'hhc', 'ride'):
        cells = []
        for p in range(16):
            d = dev.get((s, i, p), [])
            cells.append(f'{mean([x[1] for x in d]):+4.0f}' if len(d) >= 20 else '   .')
        if any(c.strip() != '.' for c in cells):
            P(f'{s:10s} {i:6s} ' + ' '.join(cells))

P('\n## Deviation by position class: mean ms, SD ms, mean fraction of a 16th, n')
P('| style | inst | class | mean ms | SD ms | mean /16th | SD /16th | n |')
P('|---|---|---|---|---|---|---|---|')
for s in STYLES:
    if not nfiles[s]:
        continue
    for i in ('kick', 'snare', 'hhc', 'ride'):
        for pc in ('beat', 'and', 'e/a'):
            d = [x for p in range(16) if POSCLASS(p) == pc for x in dev.get((s, i, p), [])]
            if len(d) < 30:
                continue
            P(f'| {s} | {i} | {pc} | {mean([x[1] for x in d]):+.1f} | {sd([x[1] for x in d]):.1f} | '
              f'{mean([x[0] for x in d]):+.3f} | {sd([x[0] for x in d]):.3f} | {len(d)} |')

P('\n## Systematic vs jitter: SD across files of the per-file mean offset, and median within-file SD (ms)')
P('| style | inst | class | mean of file means | SD of file means | median within-file SD | files |')
P('|---|---|---|---|---|---|---|')
for s in STYLES:
    for i in ('kick', 'snare', 'hhc', 'ride'):
        for pc in ('beat', 'and'):
            fm = filemeans.get((s, i, pc), [])
            if len(fm) < 5:
                continue
            P(f'| {s} | {i} | {pc} | {mean(fm):+.1f} | {sd(fm):.1f} | {st.median(within[(s, i, pc)]):.1f} | {len(fm)} |')

P('\n## Timekeeper swing (median upbeat position per file; 0.50 straight, 0.667 triplet)')
P('| style | kind | files | median | IQR | as ratio |')
P('|---|---|---|---|---|---|')
for s in STYLES:
    for k in sorted(set(k for (ss, k) in swing if ss == s)):
        x = sorted(swing[(s, k)])
        if len(x) < 3:
            continue
        m = st.median(x)
        q1 = x[len(x) // 4]
        q3 = x[(3 * len(x)) // 4]
        P(f'| {s} | {k} | {len(x)} | {m:.3f} | {q1:.3f} to {q3:.3f} | {m / (1 - m):.2f}:1 |')

P('\n## Bar-to-bar consistency per layer (means over files)')
P('jacc1: mean Jaccard of each bar with the file modal bar; same1: share of bars identical to the mode;')
P('symdiff1: mean onsets added or removed per bar vs the mode; jacc2: same with 2-bar units;')
P('anchorshare: share of the layer onsets on positions used in >= 75% of bars; size: onsets per bar')
P('| style | layer | files | jacc1 | same1 | symdiff1 | jacc2 | anchorshare | size |')
P('|---|---|---|---|---|---|---|---|---|')
for s in STYLES:
    for layer in ('skeleton', 'kick', 'snare_accent', 'snare_ghost', 'timekeeper'):
        j = consist.get((s, layer, 'jacc1'), [])
        if len(j) < 3:
            continue
        g = lambda m: mean([x for x in consist.get((s, layer, m), []) if not math.isnan(x)])
        P(f'| {s} | {layer} | {len(j)} | {g("jacc1"):.2f} | {g("same1"):.2f} | {g("symdiff1"):.2f} | '
          f'{g("jacc2"):.2f} | {g("anchorshare"):.2f} | {g("size"):.1f} |')
P('\n## All styles pooled')
P('| layer | files | jacc1 | same1 | symdiff1 | jacc2 | anchorshare | size |')
P('|---|---|---|---|---|---|---|---|')
for layer in ('skeleton', 'kick', 'snare_accent', 'snare_ghost', 'timekeeper'):
    allv = lambda m: [x for (s, l, mm), xs in consist.items() if l == layer and mm == m for x in xs if not math.isnan(x)]
    P(f'| {layer} | {len(allv("jacc1"))} | {mean(allv("jacc1")):.2f} | {mean(allv("same1")):.2f} | '
      f'{mean(allv("symdiff1")):.2f} | {mean(allv("jacc2")):.2f} | {mean(allv("anchorshare")):.2f} | {mean(allv("size")):.1f} |')

P('\n## Pairwise swing: (upbeat - start) / (next start - start), median per file, then median and IQR across files')
P('| style | unit | timekeeper | files | median | IQR | as ratio |')
P('|---|---|---|---|---|---|---|')
for s_ in STYLES:
    for (ss, key, tk_), x in sorted(pairsw.items()):
        if ss != s_ or len(x) < 3:
            continue
        x = sorted(x); m = st.median(x)
        P(f'| {ss} | {key} | {tk_} | {len(x)} | {m:.3f} | {x[len(x)//4]:.3f} to {x[(3*len(x))//4]:.3f} | {m/(1-m):.2f}:1 |')

P('\n## Offsets relative to the file timekeeper on the beat (mean over files of per-file mean, ms; + = later than the hats/ride)')
P('| style | kick beat | kick and | snare beat | snare and | snare e/a | hhc and | hhc e/a | ride and |')
P('|---|---|---|---|---|---|---|---|---|')
for s_ in STYLES:
    cells = []
    for k in (('kick','beat'),('kick','and'),('snare','beat'),('snare','and'),('snare','e/a'),('hhc','and'),('hhc','e/a'),('ride','and')):
        x = relative.get((s_,)+k, [])
        cells.append(f'{mean(x):+.1f} (n={len(x)})' if len(x) >= 5 else '.')
    P(f'| {s_} | ' + ' | '.join(cells) + ' |')

P('\n## Anchors (positions used in >= 75% of bars): mean count, per-bar recall of anchors, extra onsets per bar off the anchors')
P('| style | layer | anchors | recall | extra per bar |')
P('|---|---|---|---|---|')
for s_ in STYLES:
    for layer in ('kick', 'snare_accent', 'snare_ghost', 'timekeeper'):
        n = recall.get((s_, layer, 'nanchors'), [])
        if len(n) < 3:
            continue
        P(f'| {s_} | {layer} | {mean(n):.1f} | {mean(recall[(s_, layer, "recall")]):.2f} | {mean(recall[(s_, layer, "extra")]):.2f} |')

P('\n## Changes per bar vs the modal bar, all styles: share of bars with 0, 1, 2, 3, 4+ onsets changed')
for layer, c in changehist.items():
    t = sum(c.values())
    P(f'{layer:13s} ' + ' '.join(f'{c[i]/t:.2f}' for i in range(5)))

P('\n## Skeleton changes per bar by bar index mod 8 (from the file start), pooled over styles')
agg = collections.defaultdict(list)
for (s_, layer), d in phrase.items():
    if layer == 'skeleton':
        for i, xs in d.items():
            agg[i].extend(xs)
P(' '.join(f'bar{i+1}:{mean(agg[i]):.2f}' for i in range(8)))
