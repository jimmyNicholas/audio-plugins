#include "player.h"
#include <algorithm>
#include <cmath>

namespace breaks {

void Player::prepare(double sampleRate) {
  sr_ = sampleRate;
  offAt_.fill(-1);
  running_ = wasFree_ = false;
  freeBeat_ = beat_ = 0.0;
  ons_.reserve(kMaxHits * 2);
}

void Player::advance(const Pattern& p, int n, double bpm, double hostBeat, bool freeRun, std::vector<NoteEvent>& events) {
  const size_t first = events.size();
  const bool host = hostBeat >= 0.0;
  if (freeRun && !host && !wasFree_) freeBeat_ = 0.0; // Play starts from the top of the bar
  wasFree_ = freeRun && !host;

  if (!host && !freeRun) {
    for (int l = 0; l < kLanes; ++l)
      if (offAt_[(size_t) l] >= 0) { events.push_back({ 0, l, 0.0f, false }); offAt_[(size_t) l] = -1; }
    running_ = false;
    return;
  }
  running_ = true;

  bpm = std::clamp(bpm, 20.0, 400.0);
  const double perSample = bpm / 60.0 / sr_;
  const double start = host ? hostBeat : freeBeat_;
  const double end = start + n * perSample;
  if (!host) freeBeat_ = std::fmod(end, 4.0);
  beat_ = std::fmod(std::fmod(end, 4.0) + 4.0, 4.0);

  // Every hit of the bar that falls in this block, in any repeat of the bar.
  ons_.clear();
  const int count = hitsOf(p, hits_.data(), kMaxHits);
  for (int i = 0; i < count; ++i) {
    const Hit& h = hits_[(size_t) i];
    for (double t = h.beat + 4.0 * std::ceil((start - h.beat) / 4.0); t < end; t += 4.0) {
      const int at = std::clamp((int) std::lround((t - start) / perSample), 0, n - 1);
      ons_.push_back({ at, h.lane, h.velocity, true });
    }
  }
  std::stable_sort(ons_.begin(), ons_.end(), [](const NoteEvent& a, const NoteEvent& b) { return a.offset < b.offset; });

  // Short notes, 30 ms, cut early by the lane's next hit.
  const int length = std::max(1, (int) (0.03 * sr_));
  for (const NoteEvent& on : ons_) {
    int& off = offAt_[(size_t) on.lane];
    if (off >= 0) events.push_back({ std::min(off, on.offset), on.lane, 0.0f, false });
    events.push_back(on);
    off = on.offset + length;
  }
  for (int l = 0; l < kLanes; ++l) {
    int& off = offAt_[(size_t) l];
    if (off < 0) continue;
    if (off < n) { events.push_back({ off, l, 0.0f, false }); off = -1; }
    else off -= n;
  }
  std::stable_sort(events.begin() + (long) first, events.end(), [](const NoteEvent& a, const NoteEvent& b) { return a.offset < b.offset; });
}

bloodline::Values soundFor(Role r) {
  using namespace bloodline;
  // Real values in setting order: Pitch, Sweep, Sweep time, Attack, Decay, Body/Noise, Noise tone,
  // Noise decay, Metal, Click, Click tone, Drive, Bits, Rate, High-pass, Low-pass, Resonance, Pan, Level.
  static const double kick[kNumSettings]  = { 50, 18, 0.04, 0, 0.45, 0.03, 1000, 0.02, 0, 0.4, 2500, 0.25, 16, 48000, 20, 12000, 0, 0, -2 };
  static const double snare[kNumSettings] = { 190, 4, 0.02, 0, 0.15, 0.6, 4000, 0.2, 0, 0.3, 4000, 0.15, 16, 48000, 80, 14000, 0, 0, -3 };
  static const double hat[kNumSettings]   = { 300, 0, 0.005, 0, 0.05, 0.95, 10000, 0.06, 0.7, 0.1, 8000, 0, 16, 48000, 6000, 20000, 0, 0, -8 };
  static const double perc[kNumSettings]  = { 420, 2, 0.01, 0, 0.04, 0.7, 2500, 0.12, 0.1, 0.6, 3000, 0.1, 16, 48000, 300, 12000, 0, 0, -5 };
  const double* real = r == Role::Kick ? kick : r == Role::Snare ? snare : r == Role::Hat ? hat : perc;
  Values v{};
  for (int i = 0; i < kNumSettings; ++i) v[(size_t) i] = toNorm(i, real[i]);
  return v;
}

} // namespace breaks
