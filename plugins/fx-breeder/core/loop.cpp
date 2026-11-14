#include "loop.h"
#include <algorithm>
#include <cmath>

namespace fx {

namespace {
constexpr double kPi = 3.14159265358979323846;
const char* kKick  = "x.....x...x.....x......xx.x.....";
const char* kSnare = "....x.......x.......x.......x..x";
const char* kHat   = "x.x.x.x.x.x.x.xox.x.x.x.x.x.x.xo"; // o is open
} // namespace

void Loop::render(float* left, float* right, int n, double bpm) {
  const double perSample = std::clamp(bpm, 40.0, 300.0) / 60.0 * 4.0 / sr_;
  const double kickDecay = std::exp(std::log(0.001) / (0.35 * sr_)), snareDecay = std::exp(std::log(0.001) / (0.18 * sr_));
  const double hatClosed = std::exp(std::log(0.001) / (0.05 * sr_)), hatOpen = std::exp(std::log(0.001) / (0.3 * sr_));
  for (int i = 0; i < n; ++i) {
    const double before = pos_;
    pos_ += perSample;
    if (std::floor(pos_) != std::floor(before) || before == 0) {
      const int step = static_cast<int>(std::floor(pos_)) % 32;
      if (kKick[step] == 'x') { kick_ = 1.0; kickPhase_ = 0; }
      if (kSnare[step] == 'x') snare_ = 1.0;
      if (kHat[step] != '.') { hat_ = kHat[step] == 'o' ? 0.8 : 0.6; open_ = kHat[step] == 'o'; }
    }
    if (pos_ >= 32.0 * 64) pos_ -= 32.0 * 64;

    noise_ ^= noise_ << 13; noise_ ^= noise_ >> 17; noise_ ^= noise_ << 5;
    const double white = (noise_ / 4294967295.0) * 2.0 - 1.0;
    const double kickHz = 50.0 + 150.0 * kick_ * kick_ * kick_;
    kickPhase_ += kickHz / sr_;
    kickPhase_ -= std::floor(kickPhase_);
    const double kick = std::sin(2 * kPi * kickPhase_) * kick_;
    const double snare = (0.6 * white + 0.4 * std::sin(2 * kPi * kickPhase_ * 3.7)) * snare_;
    const double hp = white - hatPrev_ + 0.5 * hatHp_;
    hatPrev_ = white;
    hatHp_ = hp;
    const double hat = hp * hat_ * 0.35;
    kick_ *= kickDecay;
    snare_ *= snareDecay;
    hat_ *= open_ ? hatOpen : hatClosed;

    const double s = 0.55 * kick + 0.3 * snare + hat;
    left[i] = (float) s;
    right[i] = (float) s;
  }
}

} // namespace fx
