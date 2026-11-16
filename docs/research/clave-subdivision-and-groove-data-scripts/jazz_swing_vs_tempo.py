"""Jazz ride/hat eighth-note swing against tempo in GMD (pairwise measure). Usage: python3 -I jazz_swing_vs_tempo.py <groove dir>"""
import csv, math, os, statistics as st, sys
import mido
ROOT = sys.argv[1]
out = []
for r in csv.DictReader(open(os.path.join(ROOT, 'info.csv'))):
    if r['beat_type'] != 'beat' or r['time_signature'] != '4-4' or r['style'].split('/')[0] != 'jazz':
        continue
    bpm = float(r['bpm']); t = 0.0; tb = []
    for msg in mido.MidiFile(os.path.join(ROOT, r['midi_filename'])):
        t += msg.time
        if msg.type == 'note_on' and msg.velocity > 0 and msg.note in (51, 59, 53):
            tb.append(t * bpm / 60.0)
    starts = {}
    for b in tb:
        k = round(b)
        if abs(b - k) < 0.12: starts[k] = b
    v = [(b - starts[math.floor(b)]) / (starts[math.floor(b) + 1] - starts[math.floor(b)])
         for b in tb if 0.38 <= b % 1 <= 0.82 and math.floor(b) in starts and math.floor(b) + 1 in starts]
    if len(v) >= 12:
        out.append((bpm, st.median(v), len(v)))
out.sort()
for b, s, n in out: print(f'{b:5.0f} BPM  swing {s:.3f}  ratio {s/(1-s):.2f}:1  upbeats {n}')
xs = [o[0] for o in out]; ys = [o[1] for o in out]
mx, my = st.mean(xs), st.mean(ys)
r = sum((x-mx)*(y-my) for x, y in zip(xs, ys)) / math.sqrt(sum((x-mx)**2 for x in xs) * sum((y-my)**2 for y in ys))
slope = sum((x-mx)*(y-my) for x, y in zip(xs, ys)) / sum((x-mx)**2 for x in xs)
print(f'n={len(out)} r={r:.2f} slope={slope*100:.3f} swing-points per 100 BPM')
