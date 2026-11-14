#include "voice.h"
#include <algorithm>
#include <cmath>

namespace bloodline {

namespace {

constexpr double kPi = 3.14159265358979323846;

// The 808 hat's six square oscillators, in Hz. Metal plays them at Pitch / 200 times these, so
// Hats' 100 to 400 Hz pitch zone covers 0.5 to 2 times the 808 set (#43).
constexpr double k808[6] = { 205.3, 304.4, 369.6, 522.7, 540.0, 800.0 };

double coefFor(double seconds, double sr, double target) { // per-sample multiplier to fall to target
  return std::exp(std::log(target) / std::max(1.0, seconds * sr));
}

// One step of a state-variable filter (Simper's trapezoidal form). Returns band; sets low.
struct Svf {
  static double step(double x, double g, double k, double& ic1, double& ic2, double& low) {
    const double a1 = 1.0 / (1.0 + g * (g + k)), a2 = g * a1, a3 = g * a2;
    const double v3 = x - ic2;
    const double v1 = a1 * ic1 + a2 * v3;
    const double v2 = ic2 + a2 * ic1 + a3 * v3;
    ic1 = 2 * v1 - ic1;
    ic2 = 2 * v2 - ic2;
    low = v2;
    return v1;
  }
};

} // namespace

void Voice::prepare(double sampleRate) {
  sr_ = sampleRate;
  declickCoef_ = coefFor(0.002, sr_, 0.001);
  active_ = false;
}

void Voice::trigger(const Values& values, float velocity) {
  const Values r = toReal(values);
  const double vel = std::clamp((double) velocity, 0.0, 1.0);
  const double harder = 0.15 * vel * vel;

  declick_ = last_; // cut our own tail without a click
  pitch_ = r[Pitch];
  sweepOct_ = r[Sweep] / 12.0;
  const double decay = r[Decay];
  sweepCoef_ = coefFor(std::clamp(decay * 0.15, 0.008, 0.06), sr_, 0.001);
  ampCoef_ = coefFor(decay, sr_, 0.001);
  noiseMix_ = r[BodyNoise];
  metal_ = r[Metal];
  click_ = std::min(1.0, r[Click] + harder);
  const double drive = std::min(1.0, r[Drive] + harder);
  driveGain_ = 1.0 + 12.0 * drive * drive;
  driveNorm_ = 1.0 / std::tanh(driveGain_);
  level_ = 0.5 * vel; // headroom: three rows can hit at once

  svfG_ = std::tan(kPi * std::min(r[NoiseTone], 0.45 * sr_) / sr_);
  for (int i = 0; i < 6; ++i) {
    sqInc_[i] = k808[i] * pitch_ / 200.0 / sr_;
    sqPhase_[i] = 0.0;
  }

  phase_ = 0.0;
  sweepEnv_ = 1.0;
  amp_ = 1.0;
  attack_ = 0.0;
  attackStep_ = 1.0 / std::max(1.0, 0.001 * sr_); // 1 ms, so the body does not click
  clickEnv_ = 1.0;
  clickCoef_ = coefFor(0.004, sr_, 0.001);
  active_ = true;
}

float Voice::next() {
  // Body: a sine swept down from Pitch * 2^(Sweep / 12) to Pitch.
  const double freq = pitch_ * std::exp2(sweepOct_ * sweepEnv_);
  phase_ += freq / sr_;
  phase_ -= std::floor(phase_);
  const double body = std::sin(2 * kPi * phase_);
  sweepEnv_ *= sweepCoef_;

  // Noise, band-passed around Noise tone.
  noise_ ^= noise_ << 13; noise_ ^= noise_ >> 17; noise_ ^= noise_ << 5;
  const double white = (noise_ / 4294967295.0) * 2.0 - 1.0;
  double low = 0;
  const double noise = Svf::step(white, svfG_, svfK_, nBand_, nLow_, low) * svfK_ * 2.5;

  // Metal: six squares, high-passed by the same filter (input minus its band and low parts).
  double squares = 0;
  for (int i = 0; i < 6; ++i) {
    sqPhase_[i] += sqInc_[i];
    sqPhase_[i] -= std::floor(sqPhase_[i]);
    squares += sqPhase_[i] < 0.5 ? 1.0 : -1.0;
  }
  squares /= 6.0;
  double mLow = 0;
  const double mBand = Svf::step(squares, svfG_, svfK_, mBand_, mLow_, mLow);
  const double metal = (squares - svfK_ * mBand - mLow) * 1.5;

  const double noisePart = (1 - metal_) * noise + metal_ * metal;
  double s = ((1 - noiseMix_) * body + noiseMix_ * noisePart) * amp_ * attack_;
  amp_ *= ampCoef_;
  attack_ = std::min(1.0, attack_ + attackStep_);

  // Click: a few milliseconds of high-passed noise.
  const double hp = white - clickPrev_ + 0.95 * clickHp_;
  clickPrev_ = white;
  clickHp_ = hp;
  s += click_ * clickEnv_ * hp * 0.8;
  clickEnv_ *= clickCoef_;

  s = std::tanh(s * driveGain_) * driveNorm_ * level_;
  s += declick_;
  declick_ *= declickCoef_;
  s = std::clamp(s, -1.0, 1.0);

  if (amp_ < 1e-4 && clickEnv_ < 1e-4 && std::fabs(declick_) < 1e-5) active_ = false;
  last_ = s;
  return (float) s;
}

void Voice::render(float* out, int n) {
  for (int i = 0; i < n && active_; ++i) out[i] += next();
  if (!active_) last_ = 0;
}

} // namespace bloodline
