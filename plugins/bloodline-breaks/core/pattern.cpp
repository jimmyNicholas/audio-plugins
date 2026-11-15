#include "pattern.h"
#include <algorithm>
#include <cmath>

namespace breaks {

namespace {

// A first guess at each style, by ear and by the usual records. Digits are tenths: 9 is nearly
// always, 0 is never (but a free draw can still land there). Steps run 1 e & a 2 e & a ...
const StyleInfo kStyles[kNumStyles] = {
  { "Free",       { "3333333333333333", "2222222222222222", "5555555555555555", "2222222222222222" }, 0.54, 0.00, 0.03, 0.05, 0, 1.00, 90 },
  { "Boom bap",   { "9012003421810130", "0001900200019102", "8383838383838383", "0000000100000030" }, 0.58, 0.02, 0.04, 0.00, 0, 1.00, 90 },
  { "Lo-fi",      { "9002001020900100", "0000900100009001", "7272727272727272", "0000000000000020" }, 0.60, 0.05, 0.05, 0.00, 0, 0.80, 80 },
  { "Dilla",      { "8003002050300201", "0000920100009201", "6464646464646464", "0010000000100000" }, 0.54, 0.06, 0.12, 0.00, 0, 0.90, 88 },
  { "Trap",       { "9000003000900020", "0000000090000000", "9999999999999999", "0000000080000001" }, 0.50, 0.00, 0.01, 0.30, 0, 1.00, 140 },
  { "Drill",      { "9000002000300200", "0000000090000300", "9393939393939393", "0000300000000000" }, 0.50, 0.00, 0.02, 0.18, 3, 1.00, 142 },
  { "West coast", { "9000002090200000", "0000900000009000", "9595959595959595", "0000800000008000" }, 0.56, 0.01, 0.02, 0.00, 0, 1.00, 95 },
};

constexpr double kFlat[4] = { 0.2, 0.12, 0.45, 0.1 }; // the chance of a hit with no style at all

int roleIndex(Role r) { return static_cast<int>(r); }

double styleOdds(Style s, Role r, int step) {
  if (r == Role::Off) return 0.0;
  return (kStyles[(int) s].odds[roleIndex(r)][step] - '0') / 10.0;
}

double clampSwing(double s) { return std::clamp(s, kSwingMin, kSwingMax); }

} // namespace

const char* roleName(Role r) {
  switch (r) {
    case Role::Kick: return "Kick";
    case Role::Snare: return "Snare";
    case Role::Hat: return "Hat";
    case Role::Perc: return "Perc";
    default: return "Off";
  }
}

const StyleInfo& styleInfo(Style s) { return kStyles[(int) s]; }

int Pattern::hits() const {
  int n = 0;
  for (const auto& lane : vel)
    for (float v : lane) n += v > 0.0f;
  return n;
}

void Taste::learn(const Pattern& p) {
  const double rate = count == 0 ? 1.0 : 0.25;
  for (int l = 0; l < kLanes; ++l)
    for (int s = 0; s < kSteps; ++s) {
      double& o = odds[(size_t) l][(size_t) s];
      o += ((p.on(l, s) ? 1.0 : 0.0) - o) * rate;
    }
  ++count;
}

void Taste::forget(int lane) {
  if (lane < 0) { odds = {}; count = 0; return; }
  if (lane < kLanes) odds[(size_t) lane] = {};
}

double Taste::weight() const { return std::min(0.5, count * 0.1); }

double PatternOps::odds(int lane, int step) const {
  const Role r = roles[(size_t) lane];
  if (r == Role::Off) return 0.0;
  const double flat = kFlat[roleIndex(r)];
  double p = flat + (styleOdds(style, r, step) - flat) * pull;
  const double w = taste.weight();
  if (w > 0) p += (taste.odds[(size_t) lane][(size_t) step] - p) * w;
  return std::clamp(p, 0.0, 0.97);
}

// Louder on the beat than on the e and a; a snare off the backbeat is a ghost note.
float PatternOps::velocityFor(int lane, int step, breed::Rng& rng) const {
  const Role r = roles[(size_t) lane];
  const double metric = step % 4 == 0 ? 1.0 : step % 2 == 0 ? 0.8 : 0.6;
  double v = 0.7;
  switch (r) {
    case Role::Kick: v = 0.7 + 0.25 * metric; break;
    case Role::Snare: v = styleOdds(style, r, step) >= 0.5 ? 0.95 : 0.35; break;
    case Role::Hat: v = 0.35 + 0.55 * (metric - 0.5) * 2.0; break;
    default: v = 0.7; break;
  }
  v *= styleInfo(style).softness;
  v *= 0.88 + 0.24 * rng.uniform();
  return (float) std::clamp(v, 0.08, 1.0);
}

namespace {

std::uint8_t drawRoll(const StyleInfo& st, Role r, breed::Rng& rng) {
  if (r != Role::Hat || !rng.chance(st.rolls)) return 1;
  if (st.rollKind) return (std::uint8_t) st.rollKind;
  return (std::uint8_t) (2 + rng.below(3));
}

float drawNudge(const StyleInfo& st, double pull, breed::Rng& rng) {
  // Without the style's pull, a small free scatter.
  const double late = st.late * pull, loose = 0.02 + (st.loose - 0.02) * pull;
  return (float) std::clamp(late + rng.gauss() * loose, -0.45, 0.45);
}

} // namespace

Pattern PatternOps::fresh(breed::Rng& rng) const {
  const StyleInfo& st = styleInfo(style);
  Pattern p;
  // The swing dial is a soft pull: the bar's swing starts anywhere, then moves towards the dial.
  const double free = kSwingMin + (kSwingMax - kSwingMin) * rng.uniform();
  p.swing = clampSwing(free + (swingDial - free) * (0.4 + 0.6 * pull) + rng.gauss() * 0.01);
  for (int l = 0; l < kLanes; ++l)
    for (int s = 0; s < kSteps; ++s) {
      p.roll[(size_t) l][(size_t) s] = 1;
      if (!rng.chance(odds(l, s))) continue;
      p.vel[(size_t) l][(size_t) s] = velocityFor(l, s, rng);
      p.nudge[(size_t) l][(size_t) s] = drawNudge(st, pull, rng);
      p.roll[(size_t) l][(size_t) s] = drawRoll(st, roles[(size_t) l], rng);
    }
  return p;
}

// Each step is reconsidered with a chance that grows with the spread; a reconsidered step is
// redrawn from the odds (style, pull and taste). Hits that stay get their velocity and nudge
// moved a little, the nudge leaning back towards the style. Locked and Off lanes copy (Off is
// silent).
Pattern PatternOps::mutate(const Pattern& from, double spread, breed::Rng& rng) const {
  const StyleInfo& st = styleInfo(style);
  Pattern p = from;
  double s = from.swing + rng.gauss() * spread * 0.08;
  s += (swingDial - s) * pull * 0.3;
  p.swing = clampSwing(s);
  for (int l = 0; l < kLanes; ++l) {
    if (roles[(size_t) l] == Role::Off) {
      p.vel[(size_t) l] = {};
      continue;
    }
    if (locks[(size_t) l]) continue;
    for (int k = 0; k < kSteps; ++k) {
      float& vel = p.vel[(size_t) l][(size_t) k];
      float& nudge = p.nudge[(size_t) l][(size_t) k];
      std::uint8_t& roll = p.roll[(size_t) l][(size_t) k];
      if (rng.chance(std::min(0.9, spread * 0.8))) {
        const bool on = rng.chance(odds(l, k));
        if (on && vel <= 0.0f) {
          vel = velocityFor(l, k, rng);
          nudge = drawNudge(st, pull, rng);
          roll = drawRoll(st, roles[(size_t) l], rng);
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
      if (rng.chance(spread * 0.3)) roll = drawRoll(st, roles[(size_t) l], rng);
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
      // Swing moves the off sixteenth within its eighth: 50% is straight, 66% is a triplet feel.
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

} // namespace breaks
