#include "row.h"
#include <algorithm>
#include <cmath>

namespace bloodline {

namespace {
constexpr std::uint64_t kModulus = 2147483647; // Park-Miller, as in shared/breeder/engine.js

// Park-Miller's first draws follow small seeds closely (seeds 1 to 2000 all start near 0), so
// scatter the seed first. A 32-bit integer hash, then into 1 to kModulus - 1.
std::uint32_t scatter(std::uint32_t x) {
  x ^= x >> 16; x *= 0x7feb352dU;
  x ^= x >> 15; x *= 0x846ca68bU;
  x ^= x >> 16;
  return static_cast<std::uint32_t>(x % (kModulus - 1)) + 1;
}
}

Row::Row(std::uint32_t seed, Zone zone, Tuning tuning)
    : t_(tuning), seed_(scatter(seed)), zone_(zone), spread_(tuning.startSpread) {
  for (auto& pad : pads_) pad = freshDraw();
}

double Row::rand() {
  seed_ = static_cast<std::uint32_t>((static_cast<std::uint64_t>(seed_) * 16807) % kModulus);
  return (seed_ - 1) / double(kModulus - 1);
}

double Row::gauss() {
  double u1 = std::max(rand(), 1e-9), u2 = rand();
  return std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u2);
}

Values Row::freshDraw() {
  Values v{};
  for (int i = 0; i < kNumSettings; ++i) {
    Range r{ 0.0, 1.0 };
    if (zone_ != Zone::None && rand() < t_.zoneShare) r = zoneRange(zone_, i);
    v[i] = r.lo + (r.hi - r.lo) * rand();
  }
  return v;
}

// Reflect off the ends, so values near an edge are not piled up on it.
static double fold(double v) {
  if (v < 0) v = -v;
  if (v > 1) v = 2 - v;
  return std::clamp(v, 0.0, 1.0);
}

// Locked settings copy the favourite. With a zone on, a value outside it moves part of the way
// back to the nearest edge (a soft wall, #43), so every sound stays reachable.
Values Row::mutate(const Values& from) {
  Values v{};
  for (int i = 0; i < kNumSettings; ++i) {
    if (locks_[i]) { v[i] = from[i]; continue; }
    double x = fold(from[i] + gauss() * spread_);
    if (zone_ != Zone::None) {
      const Range r = zoneRange(zone_, i);
      if (x < r.lo) x += (r.lo - x) * t_.pull;
      else if (x > r.hi) x -= (x - r.hi) * t_.pull;
    }
    v[i] = x;
  }
  return v;
}

const Values& Row::press(int pad) {
  lastPressed_ = pad;
  return pads_[pad];
}

bool Row::choose() {
  if (lastPressed_ < 0) return false;
  const int picked = lastPressed_;
  if (favourite_ >= 0) { // the first choice leaves the spread and count alone
    if (picked == favourite_) {
      spread_ = std::max(t_.minSpread, spread_ * t_.narrow);
      newCount_ = 0;
    } else if (++newCount_ >= 2) {
      spread_ = std::min(t_.maxSpread, spread_ * t_.widen);
      newCount_ = 0;
    }
  }
  favourite_ = picked;
  lastPressed_ = -1;
  const Values fav = pads_[picked];
  for (int pad = 0; pad < kPads; ++pad)
    if (pad != picked) pads_[pad] = mutate(fav);
  return true;
}
void Row::restore(std::uint32_t seed, Zone zone, const std::array<Values, kPads>& pads, int favourite,
                  int lastPressed, double spread, int newCount, const std::array<bool, kNumSettings>& locks) {
  seed_ = (seed == 0 || seed >= kModulus) ? scatter(seed) : seed; // a valid Park-Miller state as is
  zone_ = zone;
  for (int pad = 0; pad < kPads; ++pad)
    for (int i = 0; i < kNumSettings; ++i) pads_[pad][i] = std::clamp(pads[pad][i], 0.0, 1.0);
  favourite_ = (favourite >= 0 && favourite < kPads) ? favourite : -1;
  lastPressed_ = (lastPressed >= 0 && lastPressed < kPads) ? lastPressed : -1;
  spread_ = std::clamp(spread, t_.minSpread, t_.maxSpread);
  newCount_ = std::clamp(newCount, 0, 1);
  locks_ = locks;
}

} // namespace bloodline
