#include "studio.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>

namespace bb2 {

Studio::Studio(std::uint32_t seed) : drums(seed), loops(seed * 2654435761U + 7) { sync(); }

void Studio::sync() {
  std::array<int, kKitPads> zones{};
  for (int p = 0; p < kKitPads; ++p) zones[(size_t) p] = drums.pad(p).full ? (int) drums.pad(p).zone : -1;
  loops.setPads(zones);
}

void Studio::keepDrum(int tile, int pad) { drums.keep(tile, pad); sync(); }

void Studio::swapPads(int a, int b) {
  if (a == b) return;
  drums.swapPads(a, b);
  loops.swapPadBits(a, b);
  sync();
}

void Studio::clearPad(int pad) { drums.clearPad(pad); sync(); }

// bloodline-breaks-2 1, then the drums, then the loops.
std::string Studio::save() const {
  std::ostringstream out;
  out.imbue(std::locale::classic());
  out << std::setprecision(17) << "bloodline-breaks-2 1\n";
  drums.save(out);
  loops.save(out);
  return out.str();
}

bool Studio::load(const std::string& text) {
  std::istringstream in(text);
  in.imbue(std::locale::classic());
  std::string word;
  int version = 0;
  if (!(in >> word >> version) || word != "bloodline-breaks-2" || version != 1) return false;
  Drums d = drums;
  Loops l = loops;
  if (!d.load(in) || !l.load(in)) return false;
  drums = d;
  loops = l;
  sync();
  return true;
}

int laneFor(Zone z) {
  switch (z) {
    case Zone::Kick: return KickLane;
    case Zone::Snare: return SnareLane;
    case Zone::Hats: return HatLane;
    default: return PercLane;
  }
}

Values builtIn(int lane) {
  using namespace bloodline;
  // Real values in setting order: Pitch, Sweep, Sweep time, Attack, Decay, Body/Noise, Noise tone,
  // Noise decay, Metal, Click, Click tone, Drive, Bits, Rate, High-pass, Low-pass, Resonance, Pan, Level.
  static const double kick[kNumSettings]  = { 50, 18, 0.04, 0, 0.45, 0.03, 1000, 0.02, 0, 0.4, 2500, 0.25, 16, 48000, 20, 12000, 0, 0, -2 };
  static const double snare[kNumSettings] = { 190, 4, 0.02, 0, 0.15, 0.6, 4000, 0.2, 0, 0.3, 4000, 0.15, 16, 48000, 80, 14000, 0, 0, -3 };
  static const double hat[kNumSettings]   = { 300, 0, 0.005, 0, 0.05, 0.95, 10000, 0.06, 0.7, 0.1, 8000, 0, 16, 48000, 6000, 20000, 0, 0, -8 };
  static const double perc[kNumSettings]  = { 420, 2, 0.01, 0, 0.04, 0.7, 2500, 0.12, 0.1, 0.6, 3000, 0.1, 16, 48000, 300, 12000, 0, 0, -5 };
  const double* real = lane == KickLane ? kick : lane == SnareLane ? snare : lane == HatLane ? hat : perc;
  Values v{};
  for (int i = 0; i < kNumSettings; ++i) v[(size_t) i] = toNorm(i, real[i]);
  return v;
}

void Player::advance(const Pattern& p, int n, double bpm, double hostBeat, bool freeRun, std::vector<Event>& out) {
  const bool host = hostBeat >= 0.0;
  if (freeRun && !host && !wasFree_) freeBeat_ = 0.0; // Play starts from the top of the bar
  wasFree_ = freeRun && !host;
  if (!host && !freeRun) { running_ = false; return; }
  running_ = true;

  const double perSample = std::clamp(bpm, 20.0, 400.0) / 60.0 / sr_;
  const double start = host ? hostBeat : freeBeat_;
  const double end = start + n * perSample;
  if (!host) freeBeat_ = std::fmod(end, 4.0);
  beat_ = std::fmod(std::fmod(end, 4.0) + 4.0, 4.0);

  const int count = hitsOf(p, hits_.data(), kMaxHits);
  for (int i = 0; i < count; ++i) {
    const Hit& h = hits_[(size_t) i];
    for (double t = h.beat + 4.0 * std::ceil((start - h.beat) / 4.0); t < end; t += 4.0)
      out.push_back({ std::clamp((int) std::lround((t - start) / perSample), 0, n - 1), h.lane, h.velocity });
  }
}

} // namespace bb2
