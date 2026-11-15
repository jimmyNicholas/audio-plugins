#include "loops.h"
#include "settings.h"
#include <algorithm>
#include <cmath>
#include <istream>
#include <ostream>
#include <string>

namespace bb2 {

namespace {

// A first guess at each style, by ear. Digits are tenths: 9 is nearly always, 0 is never (but a
// free draw can still land there). Steps run 1 e & a 2 e & a ... Lanes: kick, snare, hat, perc.
// Strung: loose and drunk, kicks pushed about, snares late. Neo soul: slow, swung, ghost snares.
const StyleInfo kStyles[kNumStyles] = {
  { "Boom bap", { "9012003421810130", "0001900200019102", "8383838383838383", "0000000100000030" }, 0.58, 0.02, 0.04, 0.00, 1.00, 90 },
  { "Lo-fi",    { "9002001020900100", "0000900100009001", "7272727272727272", "0000000000000020" }, 0.60, 0.05, 0.05, 0.00, 0.80, 80 },
  { "Strung",   { "8003002050300201", "0000920100009201", "6464646464646464", "0010000000100000" }, 0.54, 0.07, 0.14, 0.04, 0.90, 88 },
  { "Neo soul", { "9001002080020100", "0010903001109031", "7575757575757575", "0000001000000010" }, 0.63, 0.04, 0.07, 0.02, 0.85, 76 },
};

constexpr double kFlat[kLanes] = { 0.2, 0.12, 0.45, 0.1 }; // the chance of a hit with no style at all
constexpr double kLayer[kLanes] = { 0.1, 0.35, 0.15, 0.2 }; // the chance a lane layers two pads

// Which zone of pad each lane is for: Kick, Snare, Hats, and None (Drum) for perc.
constexpr int kLaneZone[kLanes] = { (int) bloodline::Zone::Kick, (int) bloodline::Zone::Snare, (int) bloodline::Zone::Hats,
                                    (int) bloodline::Zone::None };

double styleOdds(Style s, int lane, int step) { return (kStyles[(int) s].odds[lane][step] - '0') / 10.0; }
double clampSwing(double s) { return std::clamp(s, kSwingMin, kSwingMax); }

std::uint8_t drawRoll(const StyleInfo& st, int lane, breed::Rng& rng) {
  if (lane != HatLane || !rng.chance(st.rolls)) return 1;
  return (std::uint8_t) (2 + rng.below(3));
}

float drawNudge(const StyleInfo& st, double pull, breed::Rng& rng) {
  const double late = st.late * pull, loose = 0.02 + (st.loose - 0.02) * pull;
  return (float) std::clamp(late + rng.gauss() * loose, -0.45, 0.45);
}

std::uint8_t swapBits(std::uint8_t m, int a, int b) {
  const bool x = (m >> a) & 1, y = (m >> b) & 1;
  m = (std::uint8_t) (m & ~((1 << a) | (1 << b)));
  return (std::uint8_t) (m | (y << a) | (x << b));
}

} // namespace

const StyleInfo& styleInfo(Style s) { return kStyles[(int) s]; }

int Pattern::hits() const {
  int n = 0;
  for (const auto& lane : vel)
    for (float v : lane) n += v > 0.0f;
  return n;
}

double PatternOps::odds(int lane, int step) const {
  const double flat = kFlat[lane];
  return std::clamp(flat + (styleOdds(style, lane, step) - flat) * pull, 0.0, 0.97);
}

// Louder on the beat than on the e and a; a snare off the backbeat is a ghost note.
float PatternOps::velocityFor(int lane, int step, breed::Rng& rng) const {
  const double metric = step % 4 == 0 ? 1.0 : step % 2 == 0 ? 0.8 : 0.6;
  double v = 0.7;
  switch (lane) {
    case KickLane: v = 0.7 + 0.25 * metric; break;
    case SnareLane: v = styleOdds(style, lane, step) >= 0.5 ? 0.95 : 0.35; break;
    case HatLane: v = 0.35 + 0.55 * (metric - 0.5) * 2.0; break;
    default: v = 0.7; break;
  }
  v *= styleInfo(style).softness;
  v *= 0.88 + 0.24 * rng.uniform();
  return (float) std::clamp(v, 0.08, 1.0);
}

int PatternOps::choices(int lane, int* out) const {
  int n = 0;
  for (int p = 0; p < kKitPads; ++p)
    if (pads[(size_t) p] == kLaneZone[lane]) out[n++] = p;
  for (int p = 0; p < kKitPads; ++p)
    if (pads[(size_t) p] >= 0 && pads[(size_t) p] != kLaneZone[lane]) out[n++] = p;
  return n;
}

std::uint8_t PatternOps::drawSound(int lane, breed::Rng& rng) const {
  int match[kKitPads], n = 0;
  for (int p = 0; p < kKitPads; ++p)
    if (pads[(size_t) p] == kLaneZone[lane]) match[n++] = p;
  if (n == 0 && lane == PercLane) // perc borrows any kept sound
    for (int p = 0; p < kKitPads; ++p)
      if (pads[(size_t) p] >= 0) match[n++] = p;
  if (n == 0) return 0;
  const int a = match[rng.below(n)];
  std::uint8_t mask = (std::uint8_t) (1 << a);
  if (n >= 2 && rng.chance(kLayer[lane])) {
    int b = match[rng.below(n - 1)];
    if (b == a) b = match[n - 1];
    mask = (std::uint8_t) (mask | (1 << b));
  }
  return mask;
}

bool PatternOps::plays(std::uint8_t mask) const {
  for (int p = 0; p < kKitPads; ++p)
    if (((mask >> p) & 1) && pads[(size_t) p] >= 0) return true;
  return false;
}

Pattern PatternOps::fresh(breed::Rng& rng) const {
  const StyleInfo& st = styleInfo(style);
  Pattern p;
  const double free = kSwingMin + (kSwingMax - kSwingMin) * rng.uniform();
  p.swing = clampSwing(free + (swingDial - free) * (0.4 + 0.6 * pull) + rng.gauss() * 0.01);
  for (int l = 0; l < kLanes; ++l) {
    p.sound[(size_t) l] = drawSound(l, rng);
    for (int s = 0; s < kSteps; ++s) {
      p.roll[(size_t) l][(size_t) s] = 1;
      if (!rng.chance(odds(l, s))) continue;
      p.vel[(size_t) l][(size_t) s] = velocityFor(l, s, rng);
      p.nudge[(size_t) l][(size_t) s] = drawNudge(st, pull, rng);
      p.roll[(size_t) l][(size_t) s] = drawRoll(st, l, rng);
    }
  }
  return p;
}

// Each step may be redrawn from the odds; hits that stay move a little. A lane's sound is
// sometimes redrawn, and always when none of its pads is kept any more (or a pad has been kept
// since). Locked lanes copy whole.
Pattern PatternOps::mutate(const Pattern& from, double spread, breed::Rng& rng) const {
  const StyleInfo& st = styleInfo(style);
  Pattern p = from;
  double s = from.swing + rng.gauss() * spread * 0.08;
  s += (swingDial - s) * pull * 0.3;
  p.swing = clampSwing(s);
  for (int l = 0; l < kLanes; ++l) {
    if (locks[(size_t) l]) continue;
    std::uint8_t& sound = p.sound[(size_t) l];
    if (!plays(sound) || rng.chance(std::min(0.6, spread * 1.5))) sound = drawSound(l, rng);
    for (int k = 0; k < kSteps; ++k) {
      float& vel = p.vel[(size_t) l][(size_t) k];
      float& nudge = p.nudge[(size_t) l][(size_t) k];
      std::uint8_t& roll = p.roll[(size_t) l][(size_t) k];
      if (rng.chance(std::min(0.9, spread * 0.8))) {
        const bool on = rng.chance(odds(l, k));
        if (on && vel <= 0.0f) {
          vel = velocityFor(l, k, rng);
          nudge = drawNudge(st, pull, rng);
          roll = drawRoll(st, l, rng);
        } else if (!on) {
          vel = 0.0f;
        }
        continue;
      }
      if (vel <= 0.0f) continue;
      vel = (float) std::clamp(vel + rng.gauss() * spread * 0.3, 0.08, 1.0);
      double n = nudge + rng.gauss() * spread * 0.1;
      n += (st.late - n) * pull * 0.2;
      nudge = (float) std::clamp(n, -0.45, 0.45);
      if (rng.chance(spread * 0.3)) roll = drawRoll(st, l, rng);
    }
  }
  return p;
}

Pattern PatternOps::cross(const Pattern& a, const Pattern& b, breed::Rng& rng) const {
  Pattern p = a;
  for (int l = 0; l < kLanes; ++l) {
    if (locks[(size_t) l] || !rng.chance(0.5)) continue;
    p.vel[(size_t) l] = b.vel[(size_t) l];
    p.nudge[(size_t) l] = b.nudge[(size_t) l];
    p.roll[(size_t) l] = b.roll[(size_t) l];
    p.sound[(size_t) l] = b.sound[(size_t) l];
  }
  p.swing = rng.chance(0.5) ? a.swing : rng.chance(0.5) ? b.swing : (a.swing + b.swing) / 2;
  return p;
}

int hitsOf(const Pattern& p, Hit* out, int max) {
  int n = 0;
  for (int l = 0; l < kLanes; ++l)
    for (int s = 0; s < kSteps; ++s) {
      const float v = p.vel[(size_t) l][(size_t) s];
      if (v <= 0.0f) continue;
      double pos = (s % 2 == 0) ? s : (s - 1) + 2.0 * p.swing;
      pos += p.nudge[(size_t) l][(size_t) s];
      const int rolls = std::clamp((int) p.roll[(size_t) l][(size_t) s], 1, 4);
      for (int r = 0; r < rolls && n < max; ++r) {
        double beat = (pos + r / (double) rolls) / 4.0;
        beat = std::fmod(beat + 4.0, 4.0);
        out[n++] = { beat, l, r == 0 ? v : v * 0.8f };
      }
    }
  std::sort(out, out + n, [](const Hit& a, const Hit& b) { return a.beat < b.beat; });
  return n;
}

namespace {
breed::Litter<Pattern> makeLitter(std::uint32_t seed, PatternOps& ops) { return breed::Litter<Pattern>(Loops::kLitter, seed, ops); }
bool validSlot(int s) { return s >= 0 && s < Loops::kSlots; }
} // namespace

Loops::Loops(std::uint32_t seed) : litter_(makeLitter(seed, ops_)) {}

const Pattern& Loops::playing() const {
  return playingSlot_ >= 0 && slots_[(size_t) playingSlot_].full ? slots_[(size_t) playingSlot_].loop : litter_.heard();
}

template <class F> void Loops::change(F f) {
  if (playingSlot_ >= 0 && slots_[(size_t) playingSlot_].full) { f(slots_[(size_t) playingSlot_].loop); return; }
  Pattern p = litter_.heard();
  f(p);
  litter_.set(litter_.selected(), p);
}

void Loops::setStyle(Style s) {
  ops_.style = s;
  ops_.swingDial = styleInfo(s).swing;
}
void Loops::setSwingDial(double s) { ops_.swingDial = clampSwing(s); }
void Loops::setPull(double p) { ops_.pull = std::clamp(p, 0.0, 1.0); }

void Loops::swapPadBits(int a, int b) {
  if (a < 0 || b < 0 || a >= kKitPads || b >= kKitPads || a == b) return;
  auto fix = [=](Pattern& p) { for (auto& m : p.sound) m = swapBits(m, a, b); };
  for (int i = 0; i < kLitter; ++i) {
    Pattern p = litter_.candidate(i);
    fix(p);
    litter_.set(i, p);
  }
  for (Slot& s : slots_) fix(s.loop);
}

void Loops::cycleStep(int lane, int step) {
  if (lane < 0 || lane >= kLanes || step < 0 || step >= kSteps) return;
  change([=](Pattern& p) {
    float& v = p.vel[(size_t) lane][(size_t) step];
    if (v <= 0.0f) { v = 0.9f; p.nudge[(size_t) lane][(size_t) step] = 0.0f; p.roll[(size_t) lane][(size_t) step] = 1; }
    else if (v > 0.5f) v = 0.35f;
    else v = 0.0f;
  });
}

void Loops::setSwing(double s) { change([=](Pattern& p) { p.swing = clampSwing(s); }); }

void Loops::cycleSound(int lane) {
  if (lane < 0 || lane >= kLanes) return;
  int c[kKitPads];
  const int n = ops_.choices(lane, c);
  change([&](Pattern& p) {
    std::uint8_t& m = p.sound[(size_t) lane];
    int at = -1; // where the current sound is in the list: 0 to n-1 a single pad, n built-in
    if (m == 0) at = n;
    for (int i = 0; i < n; ++i)
      if (m == (1 << c[i])) at = i;
    const int next = at < 0 ? 0 : (at + 1) % (n + 1);
    m = next == n ? 0 : (std::uint8_t) (1 << c[next]);
  });
}

void Loops::keep(int tile, int s) {
  if (!validSlot(s) || tile < 0 || tile >= kLitter) return;
  slots_[(size_t) s] = { true, litter_.candidate(tile) };
}

void Loops::swapSlots(int a, int b) {
  if (!validSlot(a) || !validSlot(b)) return;
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
  litter_.set(at, slots_[(size_t) s].loop);
  litter_.breed(at, ops_);
  playingSlot_ = -1;
}

namespace {
void writePattern(std::ostream& out, const Pattern& p) {
  out << p.swing;
  for (auto m : p.sound) out << ' ' << (int) m;
  for (const auto& lane : p.vel) for (float v : lane) out << ' ' << v;
  for (const auto& lane : p.nudge) for (float v : lane) out << ' ' << v;
  for (const auto& lane : p.roll) for (auto v : lane) out << ' ' << (int) v;
  out << '\n';
}
bool readPattern(std::istream& in, Pattern& p) {
  if (!(in >> p.swing) || !std::isfinite(p.swing)) return false;
  p.swing = clampSwing(p.swing);
  for (auto& m : p.sound) { int k = 0; if (!(in >> k)) return false; m = (std::uint8_t) std::clamp(k, 0, 255); }
  for (auto& lane : p.vel) for (float& v : lane) { if (!(in >> v) || !std::isfinite(v)) return false; v = std::clamp(v, 0.0f, 1.0f); }
  for (auto& lane : p.nudge) for (float& v : lane) { if (!(in >> v) || !std::isfinite(v)) return false; v = std::clamp(v, -0.45f, 0.45f); }
  for (auto& lane : p.roll) for (auto& v : lane) { int k = 0; if (!(in >> k)) return false; v = (std::uint8_t) std::clamp(k, 1, 4); }
  return true;
}
} // namespace

// loops <style> <swing dial> <pull> <4 locks as 0s and 1s> <playing slot>
// litter <rng> <selected> <parent> <newInARow> <generation> <spread>
// then four candidates, then eight slots as <full> and a pattern:
// <swing> <4 sound masks> <64 velocities> <64 nudges> <64 rolls>
void Loops::save(std::ostream& out) const {
  const auto st = litter_.state();
  out << "loops " << (int) ops_.style << ' ' << ops_.swingDial << ' ' << ops_.pull << ' ';
  for (bool b : ops_.locks) out << (b ? '1' : '0');
  out << ' ' << playingSlot_ << "\nlitter " << st.rng << ' ' << st.selected << ' ' << st.parent << ' ' << st.newCount << ' '
      << st.generation << ' ' << st.spread << '\n';
  for (const Pattern& p : st.candidates) writePattern(out, p);
  for (const Slot& s : slots_) {
    out << (s.full ? 1 : 0) << ' ';
    writePattern(out, s.loop);
  }
}

bool Loops::load(std::istream& in) {
  std::string word, locks;
  int style = 0, playing = -1;
  PatternOps ops = ops_;
  if (!(in >> word >> style >> ops.swingDial >> ops.pull >> locks >> playing) || word != "loops") return false;
  if (style < 0 || style >= kNumStyles || !std::isfinite(ops.swingDial) || !std::isfinite(ops.pull) || locks.size() != (size_t) kLanes)
    return false;
  ops.style = (Style) style;
  ops.swingDial = clampSwing(ops.swingDial);
  ops.pull = std::clamp(ops.pull, 0.0, 1.0);
  for (int l = 0; l < kLanes; ++l) ops.locks[(size_t) l] = locks[(size_t) l] == '1';
  breed::Litter<Pattern>::State st;
  if (!(in >> word >> st.rng >> st.selected >> st.parent >> st.newCount >> st.generation >> st.spread) || word != "litter") return false;
  st.candidates.resize(kLitter);
  for (Pattern& p : st.candidates) if (!readPattern(in, p)) return false;
  std::array<Slot, kSlots> slots{};
  for (Slot& s : slots) {
    int full = 0;
    if (!(in >> full) || !readPattern(in, s.loop)) return false;
    s.full = full == 1;
  }
  breed::Litter<Pattern> litter = litter_;
  if (!litter.restore(st)) return false;
  litter_ = litter;
  ops_ = ops;
  slots_ = slots;
  playingSlot_ = validSlot(playing) && slots_[(size_t) playing].full ? playing : -1;
  return true;
}

} // namespace bb2
