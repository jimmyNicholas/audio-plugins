#include "loops.h"
#include "patch.h"
#include <algorithm>
#include <cmath>
#include <istream>
#include <ostream>
#include <string>

namespace bs2 {

namespace {

const TraitInfo kTraits[kNumTraits] = {
  { "Tonal", "Atonal" }, { "Mono", "Poly" }, { "Melodic", "Rhythmic" }, { "On beat", "Off beat" },
  { "Even", "Dynamic" }, { "Clustered", "Open" }, { "Sparse", "Dense" }, { "Short", "Long" },
};

struct ScaleInfo { const char* name; int count; int steps[12]; };
const ScaleInfo kScales[kNumScales] = {
  { "Minor", 7, { 0, 2, 3, 5, 7, 8, 10 } },
  { "Major", 7, { 0, 2, 4, 5, 7, 9, 11 } },
  { "Dorian", 7, { 0, 2, 3, 5, 7, 9, 10 } },
  { "Phrygian", 7, { 0, 1, 3, 5, 7, 8, 10 } },
  { "Minor pent", 5, { 0, 3, 5, 7, 10 } },
  { "Major pent", 5, { 0, 2, 4, 7, 9 } },
  { "Blues", 6, { 0, 3, 5, 6, 7, 10 } },
  { "Chromatic", 12, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 } },
};

const char* kKeys[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

constexpr double kEps = 1e-9;
constexpr double kSame = 1.0 / 64; // beats: two notes of one pitch closer than this are one

double wrap(double x, double len) {
  double v = std::fmod(x, len);
  if (v < 0) v += len;
  if (v >= len) v = 0;
  return v;
}

int foldPitch(int p, int lo, int hi) {
  while (p < lo) p += 12;
  while (p > hi) p -= 12;
  return p < lo ? lo : p; // only when the range is under an octave, which setOctaves prevents
}

// Moves by scale degrees (by semitones in Chromatic).
int scaleStep(int pitch, int steps, int key, Scale s) {
  const int dir = steps < 0 ? -1 : 1;
  for (int k = 0; k < std::abs(steps); ++k) {
    pitch += dir;
    for (int guard = 0; guard < 12 && !inScale(pitch, key, s); ++guard) pitch += dir;
  }
  return pitch;
}

int snapToScale(int pitch, int key, Scale s, int dir = 1) {
  for (int d = 0; d < 12; ++d) {
    if (inScale(pitch + dir * d, key, s)) return pitch + dir * d;
    if (inScale(pitch - dir * d, key, s)) return pitch - dir * d;
  }
  return pitch;
}

// How likely a sixteenth is to carry an onset, from On beat / Off beat.
double stepWeight(int step, double off) {
  if (step % 4 == 0) return 1.6 - 1.4 * off;
  if (step % 2 == 0) return 1.0;
  return 0.2 + 1.6 * off;
}

double meanWeight(double off) { return (4 * stepWeight(0, off) + 4 * stepWeight(2, off) + 8 * stepWeight(1, off)) / 16.0; }

double notesPerBar(double dense) { return 1.0 + 15.0 * dense; }

float drawVelocity(int step, double dyn, breed::Rng& rng) {
  double v = 0.72 + 0.18 * dyn * rng.gauss();
  if (step % 4 == 0) v += 0.25 * dyn;
  else if (step % 2 == 1) v -= 0.15 * dyn;
  return (float) std::clamp(v, 0.08, 1.0);
}

double drawLength(double gap, double lng, breed::Rng& rng) {
  const double f = (0.12 + 1.35 * lng) * (0.8 + 0.4 * rng.uniform());
  return std::max(kMinLength, gap * f);
}

std::uint32_t drawSeed(breed::Rng& rng) { return 1 + (std::uint32_t) (rng.uniform() * 2147483000.0); }

} // namespace

const TraitInfo& traitInfo(int t) { return kTraits[std::clamp(t, 0, kNumTraits - 1)]; }
const char* scaleName(Scale s) { return kScales[(int) s].name; }
const char* keyName(int key) { return kKeys[((key % 12) + 12) % 12]; }

bool inScale(int pitch, int key, Scale s) {
  const int pc = (((pitch - key) % 12) + 12) % 12;
  const ScaleInfo& sc = kScales[(int) s];
  for (int i = 0; i < sc.count; ++i) if (sc.steps[i] == pc) return true;
  return false;
}

int maxVoices(double poly) { return 1 + (int) std::floor(std::clamp(poly, 0.0, 1.0) * 4.999); }

int peakVoices(const Loop& l) {
  int peak = 0;
  for (const Note& n : l.notes) {
    int c = 0;
    for (const Note& m : l.notes)
      if (m.start <= n.start + kEps && m.start + m.length > n.start + kEps) ++c;
    peak = std::max(peak, c);
  }
  return peak;
}

bool LoopOps::lockedAt(double beat, double loopBeats) const {
  for (const Range& r : timeLocks)
    if (r.start < loopBeats && beat >= r.start - kEps && beat < std::min(r.end, loopBeats) - kEps) return true;
  return false;
}

// ---------------------------------------------------------------------------------------------
// Drawing from traits.

std::vector<Note> LoopOps::draw(const std::array<double, kNumTraits>& t, int bars, std::uint32_t seed) const {
  breed::Rng rng(seed);
  const double tonal = t[Tonal], poly = t[Poly], rhythmic = t[Rhythmic], off = t[OffBeat], dyn = t[Dynamic], open = t[Open],
               dense = t[Dense], lng = t[Long];
  const int motifBars = bars >= 8 ? 2 : 1, motifSteps = motifBars * 16;
  const double motifBeats = motifBars * 4.0;
  const int lo = lowNote(), hi = highNote();

  // Open uses more of the range; Clustered keeps to the middle of it.
  const double centre = (lo + hi) / 2.0, half = std::max(6.0, (hi - lo) / 2.0 * (0.35 + 0.65 * open));
  int useLo = std::max(lo, (int) std::lround(centre - half)), useHi = std::min(hi, (int) std::lround(centre + half));
  if (useHi - useLo < 12) { useLo = std::max(lo, useHi - 12); useHi = std::min(hi, useLo + 12); }
  int root = foldPitch(key + 12 * (int) std::round(centre / 12.0), useLo, useHi);

  // Onsets.
  struct Onset { double at; int step; };
  std::vector<Onset> onsets;
  const double per = notesPerBar(dense) / 16.0 / meanWeight(off);
  for (int s = 0; s < motifSteps; ++s) {
    const double u = rng.uniform(), nudge = rng.uniform();
    if (u >= std::min(1.0, per * stepWeight(s, off))) continue;
    double at = s * kSixteenth;
    if (nudge < off * 0.6) at += (rng.uniform() * 2.0 - 1.0) * 0.4 * kSixteenth * off; // off the grid
    onsets.push_back({ std::clamp(at, 0.0, motifBeats - kEps), s });
  }
  if (onsets.empty()) {
    const int s = off > 0.5 ? 2 * rng.below(motifSteps / 2) + 1 : 4 * rng.below(motifSteps / 4);
    onsets.push_back({ s * kSixteenth, s });
  }
  std::sort(onsets.begin(), onsets.end(), [](const Onset& a, const Onset& b) { return a.at < b.at; });

  // Pitches, chords, lengths and velocities.
  std::vector<Note> motif;
  int current = rng.chance(1.0 - tonal) ? root : useLo + rng.below(useHi - useLo + 1);
  if (!rng.chance(tonal)) current = snapToScale(current, key, scale);
  int other = current, dir = rng.chance(0.5) ? 1 : -1;
  const int voices = maxVoices(poly);
  for (size_t i = 0; i < onsets.size(); ++i) {
    const Onset& o = onsets[i];
    const bool chromatic = rng.chance(tonal);
    if (i > 0) {
      if (rng.chance(rhythmic * 0.9)) {
        if (rng.chance(0.3)) std::swap(current, other); // a second pitch to answer the first
      } else {
        other = current;
        const bool leap = rng.chance(0.15 + 0.6 * open);
        const int size = leap ? 2 + rng.below(1 + (int) std::lround(open * 5)) : 1;
        if (!rng.chance(0.7)) dir = -dir; // the contour mostly keeps its way
        if (current + dir * 4 > useHi) dir = -1;
        if (current - 4 < useLo && dir < 0) dir = 1;
        current = chromatic ? current + dir * (size * 2 - 1 + rng.below(2)) : scaleStep(current, dir * size, key, scale);
      }
    }
    if (o.step % 16 == 0 && rng.chance((1.0 - tonal) * 0.5)) current = root; // rooted
    if (i + 1 == onsets.size() && i > 0 && rng.chance((1.0 - tonal) * 0.6)) current = rng.chance(0.7) ? root : scaleStep(root, 4, key, scale);
    current = foldPitch(current, useLo, useHi);

    const double next = i + 1 < onsets.size() ? onsets[i + 1].at : onsets[0].at + motifBeats;
    const double length = std::min(motifBeats, drawLength(next - o.at, lng, rng));
    const float vel = drawVelocity(o.step, dyn, rng);
    motif.push_back({ o.at, length, current, vel });

    if (voices > 1 && rng.chance(poly * 0.85)) {
      const int size = 2 + rng.below(voices - 1);
      int prev = current;
      for (int k = 1; k < size; ++k) {
        const double target = 3.0 + open * 9.0 + rng.gauss() * 1.5;
        int p = prev + std::max(1, (int) std::lround(target));
        if (!rng.chance(tonal)) p = snapToScale(p, key, scale, 1);
        if (p > hi) break;
        motif.push_back({ o.at, length, p, (float) std::clamp(vel * 0.9, 0.08, 1.0) });
        prev = p;
      }
    }
  }

  // Fill the loop with the motif, each repeat varied a little so it develops.
  constexpr double kVary = 0.3;
  std::vector<Note> out = motif;
  const int repeats = bars / motifBars;
  for (int r = 1; r < repeats; ++r) {
    const double shift = r * motifBeats;
    for (const Note& n : motif) {
      Note m = n;
      m.start += shift;
      if (rng.chance(kVary)) {
        const int what = rng.below(4);
        if (what == 0) m.start = shift + wrap(n.start + (rng.chance(0.5) ? kSixteenth : -kSixteenth), motifBeats);
        else if (what == 1) m.pitch = foldPitch(rng.chance(tonal) ? m.pitch + (rng.chance(0.5) ? 1 : -1) : scaleStep(m.pitch, rng.chance(0.5) ? 1 : -1, key, scale), lo, hi);
        else if (what == 2) continue; // dropped
        else {
          const int s = rng.below(motifSteps);
          Note add = n;
          add.start = shift + s * kSixteenth;
          add.pitch = foldPitch(rng.chance(tonal) ? n.pitch + rng.below(5) - 2 : scaleStep(n.pitch, rng.below(5) - 2, key, scale), lo, hi);
          add.velocity = drawVelocity(s, dyn, rng);
          out.push_back(add);
        }
      }
      out.push_back(m);
    }
  }
  return out;
}

// ---------------------------------------------------------------------------------------------
// Locks and tidying.

void LoopOps::applyLocks(Loop& l, const Loop& from) const {
  const double L = l.beats();
  bool any = false;
  for (const Range& r : timeLocks) any |= r.start < L;
  if (!any) return;
  std::vector<Note> out;
  for (Note n : l.notes) {
    if (lockedAt(wrap(n.start, L), L)) continue;
    for (const Range& r : timeLocks)
      if (r.start < L && n.start < r.start && n.start + n.length > r.start) n.length = r.start - n.start;
    if (n.length >= kMinLength - kEps) out.push_back(n);
  }
  for (const Note& n : from.notes)
    if (n.start < L && lockedAt(n.start, L)) out.push_back(n);
  l.notes = out;
}

void LoopOps::tidy(Loop& l, bool respectLocks) const {
  const double L = l.beats();
  const int lo = lowNote(), hi = highNote();
  auto locked = [&](const Note& n) { return respectLocks && lockedAt(n.start, L); };
  for (Note& n : l.notes) {
    if (locked(n)) continue;
    n.start = wrap(n.start, L);
    if (locked(n)) { // wrapped into a lock: it may not start there
      n.length = -1;
      continue;
    }
    n.pitch = foldPitch(n.pitch, lo, hi);
    n.velocity = std::clamp(n.velocity, 0.08f, 1.0f);
    n.length = std::clamp(n.length, kMinLength, std::max(kMinLength, L - n.start));
    if (respectLocks)
      for (const Range& r : timeLocks)
        if (r.start < L && n.start < r.start && n.start + n.length > r.start) n.length = r.start - n.start;
    if (n.length < kMinLength - kEps) n.length = -1;
  }
  l.notes.erase(std::remove_if(l.notes.begin(), l.notes.end(), [](const Note& n) { return n.length < 0; }), l.notes.end());
  std::stable_sort(l.notes.begin(), l.notes.end(), [](const Note& a, const Note& b) { return a.start < b.start || (a.start == b.start && a.pitch < b.pitch); });

  const int voices = maxVoices(l.traits[Poly]);
  std::vector<bool> gone(l.notes.size(), false);
  for (size_t i = 0; i < l.notes.size(); ++i) {
    const Note& n = l.notes[i];
    for (size_t j = 0; j < i; ++j) { // one pitch, one voice: the earlier note ends where this starts
      if (gone[j]) continue;
      Note& m = l.notes[j];
      if (m.pitch != n.pitch || m.start + m.length <= n.start + kEps) continue;
      if (n.start - m.start < kSame) { if (!locked(n)) gone[i] = true; else if (!locked(m)) gone[j] = true; }
      else if (!locked(m)) m.length = n.start - m.start;
    }
    if (gone[i]) continue;
    for (;;) {
      std::vector<size_t> sounding;
      for (size_t j = 0; j < i; ++j)
        if (!gone[j] && l.notes[j].start + l.notes[j].length > n.start + kEps) sounding.push_back(j);
      if ((int) sounding.size() < voices) break;
      size_t pick = l.notes.size();
      for (size_t j : sounding)
        if (!locked(l.notes[j]) && l.notes[j].start < n.start - kEps) { pick = j; break; } // earliest first
      if (pick < l.notes.size()) {
        Note& m = l.notes[pick];
        m.length = n.start - m.start;
        if (m.length < kMinLength - kEps) gone[pick] = true;
        continue;
      }
      if (!locked(n)) gone[i] = true;
      break;
    }
  }
  std::vector<Note> out;
  for (size_t i = 0; i < l.notes.size(); ++i)
    if (!gone[i] && (int) out.size() < kMaxNotes) out.push_back(l.notes[i]);
  l.notes = out;
}

// ---------------------------------------------------------------------------------------------
// Breeding.

int LoopOps::chooseSound(const Loop& l, breed::Rng& rng) const {
  using synth::Zone;
  std::vector<std::vector<int>> tiers;
  const bool low = (lowNote() + highNote()) / 2.0 < octaveNote(3);
  const bool mono = l.traits[Poly] < 0.5, shortNotes = l.traits[Long] < 0.5;
  if (low) tiers.push_back({ (int) Zone::Bass });
  if (mono) tiers.push_back(shortNotes ? std::vector<int>{ (int) Zone::Pluck, (int) Zone::Lead } : std::vector<int>{ (int) Zone::Lead });
  else tiers.push_back(shortNotes ? std::vector<int>{ (int) Zone::Keys, (int) Zone::Pluck } : std::vector<int>{ (int) Zone::Pad, (int) Zone::Keys });
  tiers.push_back({ (int) Zone::None });
  for (const auto& zones : tiers) {
    std::vector<int> found;
    for (int p = 0; p < kKitPads; ++p)
      if (pads[(size_t) p] >= 0 && std::find(zones.begin(), zones.end(), pads[(size_t) p]) != zones.end()) found.push_back(p);
    if (!found.empty()) return found[(size_t) rng.below((int) found.size())];
  }
  std::vector<int> any;
  for (int p = 0; p < kKitPads; ++p) if (pads[(size_t) p] >= 0) any.push_back(p);
  return any.empty() ? -1 : any[(size_t) rng.below((int) any.size())];
}

Loop LoopOps::fresh(breed::Rng& rng) const {
  Loop l;
  l.bars = bars;
  for (double& t : l.traits) t = rng.uniform();
  l.seed = drawSeed(rng);
  l.notes = draw(l.traits, bars, l.seed);
  tidy(l, false);
  l.sound = chooseSound(l, rng);
  return l;
}

Loop LoopOps::redraw(const Loop& from) const {
  Loop l = from;
  l.notes = draw(l.traits, l.bars, l.seed);
  applyLocks(l, from);
  tidy(l, true);
  return l;
}

Loop LoopOps::mutate(const Loop& parent, double spread, breed::Rng& rng) const {
  const Loop from = resize(parent, bars);
  Loop l = from;
  for (int t = 0; t < kNumTraits; ++t)
    if (!traitLocks[(size_t) t]) l.traits[(size_t) t] = breed::fold(l.traits[(size_t) t] + rng.gauss() * spread);
  const double L = l.beats(), tonal = l.traits[Tonal], off = l.traits[OffBeat];
  const int lo = lowNote(), hi = highNote();

  auto inBar = [&](int b) {
    std::vector<size_t> idx;
    for (size_t i = 0; i < l.notes.size(); ++i)
      if (l.notes[i].start >= b * 4.0 - kEps && l.notes[i].start < b * 4.0 + 4.0 - kEps && !lockedAt(l.notes[i].start, L)) idx.push_back(i);
    return idx;
  };
  auto freeStep = [&](int b, double& at, int& step) {
    double total = 0;
    double w[16];
    for (int s = 0; s < 16; ++s) total += (w[s] = lockedAt(b * 4.0 + s * kSixteenth, L) ? 0.0 : stepWeight(s, off));
    if (total <= 0) return false;
    double u = rng.uniform() * total;
    step = 15;
    for (int s = 0; s < 16; ++s) { if (u < w[s]) { step = s; break; } u -= w[s]; }
    at = b * 4.0 + step * kSixteenth;
    return true;
  };
  auto addNote = [&](int b) {
    double at = 0;
    int step = 0;
    if (!freeStep(b, at, step)) return false;
    Note n;
    n.start = at;
    const auto here = inBar(b);
    const int near = here.empty() ? (lo + hi) / 2 : l.notes[here[(size_t) rng.below((int) here.size())]].pitch;
    n.pitch = rng.chance(tonal) ? near + rng.below(7) - 3 : scaleStep(snapToScale(near, key, scale), rng.below(5) - 2, key, scale);
    n.pitch = foldPitch(n.pitch, lo, hi);
    n.length = std::min(4.0, drawLength(4.0 / notesPerBar(l.traits[Dense]), l.traits[Long], rng));
    n.velocity = drawVelocity(step, l.traits[Dynamic], rng);
    l.notes.push_back(n);
    return true;
  };

  for (int b = 0; b < l.bars; ++b) {
    bool open = false;
    for (int s = 0; s < 16 && !open; ++s) open = !lockedAt(b * 4.0 + s * kSixteenth, L);
    if (!open) continue;
    const double extra = spread * 8.0;
    const int changes = 1 + (int) std::floor(extra) + (rng.chance(extra - std::floor(extra)) ? 1 : 0);
    for (int c = 0; c < changes; ++c) {
      for (int attempt = 0; attempt < 8; ++attempt) {
        const auto here = inBar(b);
        const double target = notesPerBar(l.traits[Dense]), count = (double) here.size();
        const bool some = !here.empty();
        const double w[6] = { 1.0 + std::max(0.0, target - count) * 0.3, some ? 1.0 + std::max(0.0, count - target) * 0.3 : 0.0,
                              some ? 1.2 : 0.0, some ? 1.2 : 0.0, some ? 0.6 : 0.0, some ? 0.4 : 0.0 };
        double total = 0;
        for (double x : w) total += x;
        double u = rng.uniform() * total;
        int what = 0;
        for (; what < 5; ++what) { if (u < w[what]) break; u -= w[what]; }
        if (what == 0) { if (addNote(b)) break; continue; }
        Note& n = l.notes[here[(size_t) rng.below((int) here.size())]];
        const Note was = n;
        if (what == 1) { n.length = -1; break; } // removed below
        if (what == 2) {
          const double d = rng.gauss() * spread;
          double at = wrap(n.start + std::copysign(std::max(std::abs(d), 0.5 * kSixteenth), d), L);
          if (rng.chance(1.0 - off)) at = wrap(std::round(at / kSixteenth) * kSixteenth, L);
          if (lockedAt(at, L) || std::abs(at - n.start) < 0.5 * kSixteenth - kEps) continue;
          n.start = at;
        } else if (what == 3) {
          const int steps = (rng.chance(0.5) ? 1 : -1) * (1 + rng.below(2));
          n.pitch = foldPitch(rng.chance(tonal) ? n.pitch + steps : scaleStep(n.pitch, steps, key, scale), lo, hi);
        } else if (what == 4) {
          n.length = std::max(kMinLength, n.length * (rng.chance(0.5) ? 0.5 : 2.0));
        } else {
          n.velocity = (float) std::clamp(n.velocity + (rng.chance(0.5) ? 1 : -1) * (0.15 + 0.2 * rng.uniform()), 0.08, 1.0);
        }
        if (n == was) continue;
        break;
      }
      l.notes.erase(std::remove_if(l.notes.begin(), l.notes.end(), [](const Note& n) { return n.length < 0; }), l.notes.end());
    }
  }
  applyLocks(l, from);
  tidy(l, true);
  // Every child must differ from its parent: if tidying undid the changes, add notes until it does.
  for (int guard = 0; guard < 32 && l.notes == from.notes; ++guard) {
    addNote(rng.below(l.bars));
    tidy(l, true);
  }

  if (!plays(l.sound) || rng.chance(std::min(0.6, spread * 1.5))) l.sound = chooseSound(l, rng);
  return l;
}

Loop LoopOps::cross(const Loop& pa, const Loop& pb, breed::Rng& rng) const {
  const Loop a = resize(pa, bars), b = resize(pb, bars);
  Loop l = a;
  for (int t = 0; t < kNumTraits; ++t) {
    if (traitLocks[(size_t) t]) continue;
    const double r = rng.uniform();
    l.traits[(size_t) t] = r < 0.4 ? a.traits[(size_t) t] : r < 0.8 ? b.traits[(size_t) t] : a.traits[(size_t) t] + (b.traits[(size_t) t] - a.traits[(size_t) t]) * rng.uniform();
  }
  l.notes.clear();
  for (int bar = 0; bar < l.bars; ++bar) {
    const Loop& src = rng.chance(0.5) ? a : b;
    for (const Note& n : src.notes)
      if (n.start >= bar * 4.0 - kEps && n.start < bar * 4.0 + 4.0 - kEps) l.notes.push_back(n);
  }
  l.sound = rng.chance(0.5) ? a.sound : b.sound;
  applyLocks(l, a);
  tidy(l, true);
  return l;
}

Loop resize(const Loop& l, int bars) {
  if (bars == l.bars || bars <= 0) return l;
  Loop out = l;
  out.bars = bars;
  const double L = out.beats(), old = l.beats();
  out.notes.clear();
  if (bars < l.bars) {
    for (Note n : l.notes)
      if (n.start < L) { n.length = std::max(kMinLength, std::min(n.length, L - n.start)); out.notes.push_back(n); }
  } else {
    for (double shift = 0; shift < L - kEps; shift += old)
      for (Note n : l.notes) {
        n.start += shift;
        if (n.start < L && (int) out.notes.size() < kMaxNotes) out.notes.push_back(n);
      }
  }
  return out;
}

// ---------------------------------------------------------------------------------------------
// The right panel.

namespace {
breed::Litter<Loop> makeLitter(std::uint32_t seed, LoopOps& ops) {
  return breed::Litter<Loop>(Loops::kLitter, seed, ops, breed::Spread{ 0.2, 0.02, 0.5, 0.6, 1.6 });
}
bool validSlot(int s) { return s >= 0 && s < Loops::kSlots; }
bool validBars(int b) { return b == 2 || b == 4 || b == 8; }
} // namespace

Loops::Loops(std::uint32_t seed) : litter_(makeLitter(seed, ops_)) {}

const Loop& Loops::playing() const {
  if (playingSlot_ >= 0 && slots_[(size_t) playingSlot_].full) return slots_[(size_t) playingSlot_].loop;
  return litter_.heard();
}

template <class F> void Loops::change(F f) {
  if (playingSlot_ >= 0 && slots_[(size_t) playingSlot_].full) { f(slots_[(size_t) playingSlot_].loop); return; }
  Loop l = litter_.heard();
  f(l);
  litter_.set(litter_.selected(), l);
}

void Loops::breedFrom(int i) { litter_.breed(i, ops_); playingSlot_ = -1; }
void Loops::cross(int i, int j) { litter_.cross(i, j, ops_); playingSlot_ = -1; }

void Loops::setBars(int bars) {
  if (!validBars(bars)) return;
  ops_.bars = bars;
  for (int i = 0; i < litter_.size(); ++i) litter_.set(i, resize(litter_.candidate(i), bars));
}

void Loops::setOctaves(int low, int high) {
  low = std::clamp(low, 0, 7);
  high = std::clamp(high, 1, 8);
  if (high - low < 1) { if (low == ops_.octLow) low = high - 1; else high = low + 1; }
  ops_.octLow = std::clamp(low, 0, 7);
  ops_.octHigh = std::clamp(high, ops_.octLow + 1, 8);
}

void Loops::addTimeLock(double start, double end) {
  if (end < start) std::swap(start, end);
  start = std::max(0.0, start);
  if (end - start < kMinLength) return;
  Range r{ start, end };
  std::vector<Range> out;
  for (const Range& o : ops_.timeLocks) {
    if (o.end < r.start - kEps || o.start > r.end + kEps) out.push_back(o);
    else r = { std::min(r.start, o.start), std::max(r.end, o.end) };
  }
  out.push_back(r);
  std::sort(out.begin(), out.end(), [](const Range& a, const Range& b) { return a.start < b.start; });
  ops_.timeLocks = out;
}

bool Loops::removeTimeLockAt(double beat) {
  for (auto it = ops_.timeLocks.begin(); it != ops_.timeLocks.end(); ++it)
    if (beat >= it->start - kEps && beat < it->end + kEps) { ops_.timeLocks.erase(it); return true; }
  return false;
}

void Loops::rechooseSilent() {
  for (int i = 0; i < litter_.size(); ++i)
    if (litter_.candidate(i).sound < 0) {
      Loop l = litter_.candidate(i);
      l.sound = ops_.chooseSound(l, litter_.rng());
      if (l.sound >= 0) litter_.set(i, l);
    }
  for (Slot& s : slots_)
    if (s.full && s.loop.sound < 0) s.loop.sound = ops_.chooseSound(s.loop, litter_.rng());
}

void Loops::setPads(const std::array<int, kKitPads>& zones) {
  ops_.pads = zones;
  rechooseSilent();
}

void Loops::swapPadIndex(int a, int b) {
  auto fix = [&](Loop& l) { if (l.sound == a) l.sound = b; else if (l.sound == b) l.sound = a; };
  for (int i = 0; i < litter_.size(); ++i) { Loop l = litter_.candidate(i); fix(l); litter_.set(i, l); }
  for (Slot& s : slots_) fix(s.loop);
}

void Loops::setTrait(int t, double v) {
  if (t < 0 || t >= kNumTraits) return;
  change([&](Loop& l) { l.traits[(size_t) t] = std::clamp(v, 0.0, 1.0); });
}

void Loops::redraw() { change([&](Loop& l) { l = ops_.redraw(l); }); }

int Loops::addNote(double start, int pitch) {
  int at = -1;
  change([&](Loop& l) {
    if ((int) l.notes.size() >= kMaxNotes) return;
    l.notes.push_back({ wrap(start, l.beats()), kSixteenth, std::clamp(pitch, 0, 127), 0.8f });
    at = (int) l.notes.size() - 1;
  });
  return at;
}

void Loops::setNote(int i, const Note& n) {
  change([&](Loop& l) {
    if (i < 0 || i >= (int) l.notes.size()) return;
    Note m = n;
    m.start = wrap(m.start, l.beats());
    m.length = std::clamp(m.length, kMinLength, l.beats());
    m.pitch = std::clamp(m.pitch, 0, 127);
    m.velocity = std::clamp(m.velocity, 0.08f, 1.0f);
    l.notes[(size_t) i] = m;
  });
}

void Loops::removeNote(int i) {
  change([&](Loop& l) { if (i >= 0 && i < (int) l.notes.size()) l.notes.erase(l.notes.begin() + i); });
}

void Loops::cycleSound() {
  change([&](Loop& l) {
    for (int k = 1; k <= kKitPads; ++k) {
      const int p = ((l.sound < 0 ? -1 : l.sound) + k + kKitPads) % kKitPads;
      if (ops_.plays(p)) { l.sound = p; return; }
    }
    l.sound = -1;
  });
}

void Loops::setSound(int pad) {
  if (ops_.plays(pad)) change([&](Loop& l) { l.sound = pad; });
}

void Loops::keep(int tile, int s) {
  if (!validSlot(s) || tile < 0 || tile >= kLitter) return;
  slots_[(size_t) s] = { true, litter_.candidate(tile) };
}

void Loops::swapSlots(int a, int b) {
  if (!validSlot(a) || !validSlot(b) || a == b) return;
  std::swap(slots_[(size_t) a], slots_[(size_t) b]);
  if (playingSlot_ == a) playingSlot_ = b;
  else if (playingSlot_ == b) playingSlot_ = a;
}

void Loops::clearSlot(int s) {
  if (!validSlot(s)) return;
  slots_[(size_t) s] = Slot{};
  if (playingSlot_ == s) playingSlot_ = -1;
}

void Loops::playSlot(int s) { if (validSlot(s) && slots_[(size_t) s].full) playingSlot_ = s; }

void Loops::breedFromSlot(int s) {
  if (!validSlot(s) || !slots_[(size_t) s].full) return;
  const int at = litter_.selected();
  litter_.set(at, resize(slots_[(size_t) s].loop, ops_.bars));
  litter_.breed(at, ops_);
  playingSlot_ = -1;
}

// loops <bars> <key> <scale> <octave low> <octave high> <trait locks as 0s and 1s> <playing slot>
// timelocks <count> then <start> <end> for each
// litter <rng> <selected> <parent> <newInARow> <generation> <spread>
// then four loops, then eight slots as <full> and a loop. A loop is
// loop <bars> <sound> <seed> <8 traits> <note count>, then <start> <length> <pitch> <velocity> for each
namespace {
void saveLoop(std::ostream& out, const Loop& l) {
  out << "loop " << l.bars << ' ' << l.sound << ' ' << l.seed;
  for (double t : l.traits) out << ' ' << t;
  out << ' ' << l.notes.size();
  for (const Note& n : l.notes) out << ' ' << n.start << ' ' << n.length << ' ' << n.pitch << ' ' << n.velocity;
  out << '\n';
}

bool loadLoop(std::istream& in, Loop& l) {
  std::string word;
  int count = 0;
  if (!(in >> word >> l.bars >> l.sound >> l.seed) || word != "loop" || !validBars(l.bars) || l.sound < -1 || l.sound >= kKitPads) return false;
  for (double& t : l.traits) { if (!(in >> t) || !std::isfinite(t)) return false; t = std::clamp(t, 0.0, 1.0); }
  if (!(in >> count) || count < 0 || count > kMaxNotes) return false;
  l.notes.assign((size_t) count, {});
  for (Note& n : l.notes) {
    if (!(in >> n.start >> n.length >> n.pitch >> n.velocity) || !std::isfinite(n.start) || !std::isfinite(n.length)) return false;
    if (n.start < 0 || n.start >= l.beats() || n.length <= 0 || n.pitch < 0 || n.pitch > 127) return false;
    n.velocity = std::clamp(n.velocity, 0.08f, 1.0f);
  }
  return true;
}
} // namespace

void Loops::save(std::ostream& out) const {
  const auto st = litter_.state();
  out << "loops " << ops_.bars << ' ' << ops_.key << ' ' << (int) ops_.scale << ' ' << ops_.octLow << ' ' << ops_.octHigh << ' ';
  for (bool b : ops_.traitLocks) out << (b ? '1' : '0');
  out << ' ' << playingSlot_ << "\ntimelocks " << ops_.timeLocks.size();
  for (const Range& r : ops_.timeLocks) out << ' ' << r.start << ' ' << r.end;
  out << "\nlitter " << st.rng << ' ' << st.selected << ' ' << st.parent << ' ' << st.newCount << ' ' << st.generation << ' ' << st.spread << '\n';
  for (const Loop& l : st.candidates) saveLoop(out, l);
  for (const Slot& s : slots_) {
    out << (s.full ? 1 : 0) << ' ';
    saveLoop(out, s.loop);
  }
}

bool Loops::load(std::istream& in) {
  std::string word, locks;
  LoopOps ops = ops_;
  int scale = 0, playing = -1, count = 0;
  if (!(in >> word >> ops.bars >> ops.key >> scale >> ops.octLow >> ops.octHigh >> locks >> playing) || word != "loops") return false;
  if (!validBars(ops.bars) || ops.key < 0 || ops.key > 11 || scale < 0 || scale >= kNumScales || ops.octLow < 0 || ops.octHigh > 8 ||
      ops.octHigh - ops.octLow < 1 || locks.size() != (size_t) kNumTraits || playing < -1 || playing >= kSlots)
    return false;
  ops.scale = (Scale) scale;
  for (int t = 0; t < kNumTraits; ++t) {
    if (locks[(size_t) t] != '0' && locks[(size_t) t] != '1') return false;
    ops.traitLocks[(size_t) t] = locks[(size_t) t] == '1';
  }
  if (!(in >> word >> count) || word != "timelocks" || count < 0 || count > 256) return false;
  ops.timeLocks.assign((size_t) count, {});
  for (Range& r : ops.timeLocks)
    if (!(in >> r.start >> r.end) || !std::isfinite(r.start) || !std::isfinite(r.end) || r.start < 0 || r.end <= r.start) return false;
  breed::Litter<Loop>::State st;
  if (!(in >> word >> st.rng >> st.selected >> st.parent >> st.newCount >> st.generation >> st.spread) || word != "litter") return false;
  st.candidates.resize(kLitter);
  for (Loop& l : st.candidates) if (!loadLoop(in, l)) return false;
  std::array<Slot, kSlots> slots{};
  for (Slot& s : slots) {
    int full = 0;
    if (!(in >> full) || !loadLoop(in, s.loop)) return false;
    s.full = full == 1;
  }
  breed::Litter<Loop> litter = litter_;
  if (!litter.restore(st)) return false;
  litter_ = litter;
  ops_ = ops;
  slots_ = slots;
  playingSlot_ = playing >= 0 && slots_[(size_t) playing].full ? playing : -1;
  return true;
}

// ---------------------------------------------------------------------------------------------
// The player.

void Player::stop(std::vector<Event>* out) {
  if (out)
    for (const Active& a : active_) out->push_back({ 0, a.pitch, a.tag, 0.0f, false });
  active_.clear();
  last_ = -1.0;
}

void Player::advance(const Loop& l, int tag, int n, double bpm, double hostBeat, bool run, std::vector<Event>& out) {
  const size_t first = out.size();
  const bool host = hostBeat >= 0.0;
  if (!run || n <= 0) {
    stop(&out);
    running_ = false;
    return;
  }
  const double perSample = std::clamp(bpm, 20.0, 400.0) / 60.0 / sr_;
  if (!running_ || host != wasHost_) { stop(&out); free_ = 0.0; }
  const double start = host ? hostBeat : free_;
  if (last_ >= 0.0 && std::abs(start - last_) > 0.05) stop(&out); // the host jumped
  const double end = start + n * perSample;
  running_ = true;
  wasHost_ = host;
  auto at = [&](double t) { return std::clamp((int) std::floor((t - start) / perSample), 0, n - 1); };

  for (auto it = active_.begin(); it != active_.end();) {
    if (it->off < end) { out.push_back({ at(it->off), it->pitch, it->tag, 0.0f, false }); it = active_.erase(it); }
    else ++it;
  }
  const double L = l.beats();
  for (const Note& note : l.notes) {
    if (note.start >= L) continue;
    for (double t = note.start + L * std::ceil((start - note.start) / L - 1e-12); t < end; t += L) {
      if (t < start - 1e-12) continue;
      const int on = at(t);
      for (auto it = active_.begin(); it != active_.end();) // the same note again: it retriggers
        if (it->pitch == note.pitch && it->tag == tag) it = active_.erase(it);
        else ++it;
      out.push_back({ on, note.pitch, tag, note.velocity, true });
      const double off = t + note.length;
      if (off < end && at(off) > on) out.push_back({ at(off), note.pitch, tag, 0.0f, false });
      else active_.push_back({ note.pitch, tag, off });
    }
  }
  std::stable_sort(out.begin() + (long) first, out.end(), [](const Event& a, const Event& b) {
    return a.offset < b.offset || (a.offset == b.offset && !a.on && b.on);
  });
  free_ = host ? 0.0 : end;
  last_ = end;
  beat_ = wrap(end, L);
}

} // namespace bs2
