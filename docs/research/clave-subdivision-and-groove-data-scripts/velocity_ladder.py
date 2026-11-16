"""Velocity ladder per style in GMD beats: kick on 1 / other kicks, snare backbeat (2 and 4) /
snare ghosts (below 0.65 x the file's 95th percentile snare velocity), closed hat on beat / & / e,a.
Usage: python3 -I velocity_ladder.py <groove dir>"""
import collections, csv, os, statistics as st, sys
import mido
ROOT = sys.argv[1]
acc = collections.defaultdict(list)
for r in csv.DictReader(open(os.path.join(ROOT, 'info.csv'))):
    if r['beat_type'] != 'beat' or r['time_signature'] != '4-4':
        continue
    style = r['style'].split('/')[0]; bpm = float(r['bpm']); t = 0.0; ons = []
    for msg in mido.MidiFile(os.path.join(ROOT, r['midi_filename'])):
        t += msg.time
        if msg.type == 'note_on' and msg.velocity > 0:
            ons.append((round(t * bpm / 15.0) % 16, msg.note, msg.velocity))
    sv = sorted(v for p, n, v in ons if n in (38, 40, 37))
    thr = 0.65 * sv[int(0.95 * (len(sv) - 1))] if sv else 999
    for p, n, v in ons:
        if n == 36: acc[(style, 'kick on 1' if p == 0 else 'kick other')].append(v)
        elif n in (38, 40, 37):
            if p in (4, 12) and v >= thr: acc[(style, 'snare 2 and 4')].append(v)
            elif v < thr: acc[(style, 'snare ghost')].append(v)
        elif n in (42, 22):
            acc[(style, 'hat beat' if p % 4 == 0 else 'hat &' if p % 2 == 0 else 'hat e/a')].append(v)
cols = ['kick on 1', 'kick other', 'snare 2 and 4', 'snare ghost', 'hat beat', 'hat &', 'hat e/a']
print('| style | ' + ' | '.join(cols) + ' | ghost / backbeat | hat e,a / beat |')
print('|' + '---|' * (len(cols) + 3))
for s in ['rock', 'funk', 'hiphop', 'soul', 'latin', 'jazz', 'afrobeat', 'pop', 'neworleans']:
    m = {c: st.mean(acc[(s, c)]) if len(acc[(s, c)]) >= 30 else float('nan') for c in cols}
    print(f'| {s} | ' + ' | '.join(f'{m[c]:.0f}' for c in cols) + f" | {m['snare ghost']/m['snare 2 and 4']:.2f} | {m['hat e/a']/m['hat beat']:.2f} |")
