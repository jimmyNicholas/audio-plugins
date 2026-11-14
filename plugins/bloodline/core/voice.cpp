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

double gFor(double hz, double sr) { return std::tan(kPi * std::clamp(hz, 10.0, 0.45 * sr) / sr); }

// One step of a state-variable filter (Simper's trapezoidal form). Returns band; sets low.
double svf(double x, double g, double k, double& ic1, double& ic2, double& low) {
  const double a1 = 1.0 / (1.0 + g * (g + k)), a2 = g * a1, a3 = g * a2;
  const double v3 = x - ic2;
  const double v1 = a1 * ic1 + a2 * v3;
  const double v2 = ic2 + a2 * ic1 + a3 * v3;
  ic1 = 2 * v1 - ic1;
  ic2 = 2 * v2 - ic2;
  low = v2;
  return v1;
}

} // namespace

void Voice::prepare(double sampleRate) {
  sr_ = sampleRate;
  declickCoef_ = coefFor(0.002, sr_, 0.001);
  active_ = false;
  hpLow_ = hpBand_ = lpLow_ = lpBand_ = 0;
}

void Voice::trigger(const Values& values, float velocity) {
  trigger(values, velocity, toReal(Pan, values[Pan]), toReal(Level, values[Level]));
}

void Voice::trigger(const Values& values, float velocity, double pan, double levelDb) {
  const Values r = toReal(values);
  const double vel = std::clamp((double) velocity, 0.0, 1.0);
  const double harder = 0.15 * vel * vel;

  declick_ = last_; // cut our own tail without a click
  pitch_ = r[Pitch];
  sweepOct_ = r[Sweep] / 12.0;
  sweepCoef_ = coefFor(r[SweepTime], sr_, 0.001);
  ampCoef_ = coefFor(r[Decay], sr_, 0.001);
  noiseCoef_ = coefFor(r[NoiseDecay], sr_, 0.001);
  noiseMix_ = r[BodyNoise];
  metal_ = r[Metal];
  click_ = std::min(1.0, r[Click] + harder);

  // Drive: tanh that hardens towards a clip over the top fifth of the range.
  const double drive = std::min(1.0, r[Drive] + harder);
  driveGain_ = 1.0 + 30.0 * drive * drive;
  driveNorm_ = 1.0 / std::tanh(driveGain_);
  hard_ = std::clamp((drive - 0.8) / 0.2, 0.0, 1.0);

  steps_ = std::pow(2.0, r[Bits] - 1.0);
  holdInc_ = std::min(1.0, r[Rate] / sr_);
  held_ = 0;
  holdPhase_ = 1.0;

  level_ = 0.5 * vel * std::pow(10.0, std::clamp(levelDb, -60.0, 6.0) / 20.0); // headroom: pads overlap
  pan = std::clamp(pan, -1.0, 1.0); // a balance law: 1 each at centre, never above 1
  gainL_ = std::min(1.0, 1.0 - pan);
  gainR_ = std::min(1.0, 1.0 + pan);

  svfG_ = gFor(r[NoiseTone], sr_);
  clickG_ = gFor(r[ClickTone], sr_);
  hpG_ = gFor(r[HighPass], sr_);
  lpG_ = gFor(r[LowPass], sr_);
  filterK_ = 2.0 - 2.0 * r[Resonance]; // 2 is flat, near 0 rings
  for (int i = 0; i < 6; ++i) {
    sqInc_[i] = k808[i] * pitch_ / 200.0 / sr_;
    sqPhase_[i] = 0.0;
  }

  phase_ = 0.0;
  sweepEnv_ = 1.0;
  amp_ = 1.0;
  noiseAmp_ = 1.0;
  attack_ = 0.0;
  attackStep_ = 1.0 / std::max(1.0, std::max(0.001, r[Attack]) * sr_); // at least 1 ms, so the body does not click
  noiseAttack_ = 0.0;
  noiseAttackStep_ = 1.0 / std::max(1.0, 0.0005 * sr_);
  clickEnv_ = 1.0;
  clickCoef_ = coefFor(0.004, sr_, 0.001);
  active_ = true;
}

double Voice::next() {
  // Body: a sine swept down from Pitch * 2^(Sweep / 12) to Pitch over Sweep time.
  const double freq = std::min(pitch_ * std::exp2(sweepOct_ * sweepEnv_), 0.45 * sr_);
  phase_ += freq / sr_;
  phase_ -= std::floor(phase_);
  const double body = std::sin(2 * kPi * phase_) * amp_ * attack_;
  sweepEnv_ *= sweepCoef_;
  amp_ *= ampCoef_;
  attack_ = std::min(1.0, attack_ + attackStep_);

  // Noise, band-passed around Noise tone.
  noise_ ^= noise_ << 13; noise_ ^= noise_ >> 17; noise_ ^= noise_ << 5;
  const double white = (noise_ / 4294967295.0) * 2.0 - 1.0;
  double low = 0;
  const double noise = svf(white, svfG_, svfK_, nBand_, nLow_, low) * svfK_ * 2.5;

  // Metal: six squares, high-passed by the same filter (input minus its band and low parts).
  double squares = 0;
  for (int i = 0; i < 6; ++i) {
    sqPhase_[i] += sqInc_[i];
    sqPhase_[i] -= std::floor(sqPhase_[i]);
    squares += sqPhase_[i] < 0.5 ? 1.0 : -1.0;
  }
  squares /= 6.0;
  double mLow = 0;
  const double mBand = svf(squares, svfG_, svfK_, mBand_, mLow_, mLow);
  const double metal = (squares - svfK_ * mBand - mLow) * 1.5;

  // The noise part has its own tail (Noise decay).
  const double noisePart = ((1 - metal_) * noise + metal_ * metal) * noiseAmp_ * noiseAttack_;
  noiseAmp_ *= noiseCoef_;
  noiseAttack_ = std::min(1.0, noiseAttack_ + noiseAttackStep_);
  double s = (1 - noiseMix_) * body + noiseMix_ * noisePart;

  // Click: a few milliseconds of noise, band-passed at Click tone.
  double cLow = 0;
  const double click = svf(white, clickG_, 1.0, cBand_, cLow_, cLow) * 2.0;
  s += click_ * clickEnv_ * click;
  clickEnv_ *= clickCoef_;

  // Grit: drive, then bits, then rate (sample and hold).
  const double soft = std::tanh(s * driveGain_) * driveNorm_;
  const double hard = std::clamp(s * driveGain_, -1.0, 1.0);
  s = soft + (hard - soft) * hard_;
  s = std::round(s * steps_) / steps_;
  holdPhase_ += holdInc_;
  if (holdPhase_ >= 1.0) { holdPhase_ -= 1.0; held_ = s; }
  s = held_;

  // Out: high-pass, then low-pass, sharing one resonance.
  double hpLow = 0;
  const double hpBand = svf(s, hpG_, filterK_, hpBand_, hpLow_, hpLow);
  s = s - filterK_ * hpBand - hpLow;
  double lpLow = 0;
  svf(s, lpG_, filterK_, lpBand_, lpLow_, lpLow);
  s = lpLow;

  s = s * level_ + declick_;
  declick_ *= declickCoef_;
  if (!std::isfinite(s)) { s = 0; hpLow_ = hpBand_ = lpLow_ = lpBand_ = 0; }
  // A soft knee above 0.7, so resonance near self-oscillation rounds off instead of clipping.
  const double mag = std::fabs(s);
  if (mag > 0.7) s = std::copysign(0.7 + 0.3 * std::tanh((mag - 0.7) / 0.3), s);

  if (amp_ < 1e-4 && noiseAmp_ < 1e-4 && clickEnv_ < 1e-4 && std::fabs(declick_) < 1e-5) active_ = false;
  last_ = s;
  return s;
}

void Voice::render(float* left, float* right, int n) {
  for (int i = 0; i < n && active_; ++i) {
    const double s = next();
    left[i] += (float) (s * gainL_);
    right[i] += (float) (s * gainR_);
  }
  if (!active_) last_ = 0;
}

} // namespace bloodline
