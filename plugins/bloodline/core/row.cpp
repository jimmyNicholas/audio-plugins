#include "row.h"
#include <algorithm>
#include <cmath>

namespace bloodline {

namespace {
constexpr std::uint64_t kModulus = 2147483647; // Park-Miller, as in shared/breeder/engine.js
constexpr double kPi = 3.14159265358979323846;

// Park-Miller's first draws follow small seeds closely (seeds 1 to 2000 all start near 0), so
// scatter the seed first. A 32-bit integer hash, then into 1 to kModulus - 1.
std::uint32_t scatter(std::uint32_t x) {
  x ^= x >> 16; x *= 0x7feb352dU;
  x ^= x >> 15; x *= 0x846ca68bU;
  x ^= x >> 16;
  return static_cast<std::uint32_t>(x % (kModulus - 1)) + 1;
}

// Reflect off the ends, so values near an edge are not piled up on it.
double fold(double v) {
  if (v < 0) v = -v;
  if (v > 1) v = 2 - v;
  return std::clamp(v, 0.0, 1.0);
}
} // namespace

Row::Row(std::uint32_t seed, Zone zone, Tuning tuning)
    : t_(tuning), seed_(scatter(seed)), zone_(zone), spread_(tuning.startSpread) {
  for (auto& pad : pads_) pad = freshDraw();
  if (zone == Zone::Kick) rowMono_ = true;
}

double Row::rand() {
  seed_ = static_cast<std::uint32_t>((static_cast<std::uint64_t>(seed_) * 16807) % kModulus);
  return (seed_ - 1) / double(kModulus - 1);
}

double Row::gauss() {
  double u1 = std::max(rand(), 1e-9), u2 = rand();
  return std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * kPi * u2);
}

// Level starts at 0 dB, so a quiet candidate never sounds like a bad one (#45).
Values Row::freshDraw() {
  Values v{};
  for (int i = 0; i < kNumSettings; ++i) {
    Range r{ 0.0, 1.0 };
    if (zone_ != Zone::None && rand() < t_.zoneShare) r = zoneRange(zone_, i);
    v[i] = r.lo + (r.hi - r.lo) * rand();
  }
  v[Level] = 1.0;
  return v;
}

// Locked settings copy the favourite, and each pad keeps its own Level. With a zone on, a value
// outside it moves part of the way back to the nearest edge (a soft wall, #43), so every sound
// stays reachable. Settings outside the zoned groups span 0 to 1, so they are never pulled.
Values Row::mutate(const Values& from, double ownLevel) {
  Values v{};
  for (int i = 0; i < kNumSettings; ++i) {
    if (!info(i).bred) { v[i] = ownLevel; continue; }
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
  lastPlayed_ = pad;
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
  lastPlayed_ = picked;
  const Values fav = pads_[picked];
  for (int pad = 0; pad < kPads; ++pad)
    if (pad != picked) pads_[pad] = mutate(fav, pads_[pad][Level]);
  return true;
}

int Row::random(double r) {
  r = std::clamp(r, 0.0, 0.999999);
  int pad;
  if (lastRandom_ < 0) pad = static_cast<int>(r * kPads);
  else {
    pad = static_cast<int>(r * (kPads - 1)); // one of the other two
    if (pad >= lastRandom_) pad += 1;
  }
  lastRandom_ = pad;
  lastPlayed_ = pad;
  return pad;
}

void Row::reset() {
  for (auto& pad : pads_) pad = freshDraw();
  padMono_ = {};
  favourite_ = lastPressed_ = lastPlayed_ = lastRandom_ = -1;
  spread_ = t_.startSpread;
  newCount_ = 0;
}

void Row::setZone(Zone zone) {
  if (zone == Zone::Kick && zone_ != Zone::Kick) rowMono_ = true;
  zone_ = zone;
}

void Row::setValue(int pad, int setting, double norm) {
  if (pad < 0 || pad >= kPads || setting < 0 || setting >= kNumSettings) return;
  pads_[static_cast<size_t>(pad)][static_cast<size_t>(setting)] = std::clamp(norm, 0.0, 1.0);
}

void Row::setRowPan(double pan) { rowPan_ = std::clamp(pan, -1.0, 1.0); }
void Row::setRowLevel(double db) { rowLevel_ = std::clamp(db, kRowLevelMin, kRowLevelMax); }

double Row::padPan(int pad) const {
  if (rowMono_ || padMono_[static_cast<size_t>(pad)]) return 0.0;
  return std::clamp(toReal(Pan, pads_[static_cast<size_t>(pad)][Pan]) + rowPan_, -1.0, 1.0);
}

double Row::padLevel(int pad) const { return toReal(Level, pads_[static_cast<size_t>(pad)][Level]) + rowLevel_; }

Row::State Row::state() const {
  State s;
  s.seed = seed_;
  s.zone = zone_;
  s.pads = pads_;
  s.favourite = favourite_;
  s.lastPressed = lastPressed_;
  s.lastPlayed = lastPlayed_;
  s.spread = spread_;
  s.newCount = newCount_;
  s.locks = locks_;
  s.padMono = padMono_;
  s.rowPan = rowPan_;
  s.rowLevel = rowLevel_;
  s.rowMono = rowMono_;
  return s;
}

void Row::restore(const State& s) {
  auto pad = [](int i) { return (i >= 0 && i < kPads) ? i : -1; };
  seed_ = (s.seed == 0 || s.seed >= kModulus) ? scatter(s.seed) : s.seed; // a valid Park-Miller state as is
  zone_ = s.zone;
  for (int p = 0; p < kPads; ++p)
    for (int i = 0; i < kNumSettings; ++i)
      pads_[p][i] = std::isfinite(s.pads[p][i]) ? std::clamp(s.pads[p][i], 0.0, 1.0) : 0.5;
  favourite_ = pad(s.favourite);
  lastPressed_ = pad(s.lastPressed);
  lastPlayed_ = pad(s.lastPlayed);
  lastRandom_ = -1;
  spread_ = std::clamp(std::isfinite(s.spread) ? s.spread : t_.startSpread, t_.minSpread, t_.maxSpread);
  newCount_ = std::clamp(s.newCount, 0, 1);
  locks_ = s.locks;
  padMono_ = s.padMono;
  setRowPan(std::isfinite(s.rowPan) ? s.rowPan : 0.0);
  setRowLevel(std::isfinite(s.rowLevel) ? s.rowLevel : 0.0);
  rowMono_ = s.rowMono;
}

} // namespace bloodline
