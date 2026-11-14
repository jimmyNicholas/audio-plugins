#include "engine.h"
#include <algorithm>
#include <cmath>

namespace synth {

namespace {

constexpr double kPi = 3.14159265358979323846;

double coefFor(double seconds, double sr) { return std::exp(std::log(0.001) / std::max(1.0, seconds * sr)); }

// PolyBLEP: rounds off a naive saw or square's jumps, so most of the aliasing goes.
double blep(double t, double dt) {
  if (t < dt) { t /= dt; return t + t - t * t - 1.0; }
  if (t > 1.0 - dt) { t = (t - 1.0) / dt; return t * t + t + t + 1.0; }
  return 0.0;
}

// Sine, then triangle, then saw, then square, morphing between neighbours.
double shaped(double phase, double dt, double shape) {
  dt = std::min(dt, 0.5);
  const double seg = shape * 3.0;
  const int k = std::min(2, static_cast<int>(seg));
  const double f = seg - k;
  auto sine = [&] { return std::sin(2 * kPi * phase); };
  auto tri = [&] { return 1.0 - 4.0 * std::fabs(phase - 0.5); };
  auto saw = [&] { return 2.0 * phase - 1.0 - blep(phase, dt); };
  auto square = [&] {
    double s = phase < 0.5 ? 1.0 : -1.0;
    s += blep(phase, dt);
    double t2 = phase + 0.5;
    t2 -= std::floor(t2);
    return s - blep(t2, dt);
  };
  switch (k) {
    case 0: return sine() + (tri() - sine()) * f;
    case 1: return tri() + (saw() - tri()) * f;
    default: return saw() + (square() - saw()) * f;
  }
}

double noteHz(int note) { return 440.0 * std::exp2((note - 69) / 12.0); }

} // namespace

void Voice::prepare(double sampleRate) {
  sr_ = sampleRate;
  stage_ = Stage::Off;
}

void Voice::start(int note, float velocity, const Patch& p, double glideFromHz, std::uint64_t age) {
  const bool wasActive = active();
  note_ = note;
  age_ = age;
  vel_ = std::clamp((double) velocity, 0.0, 1.0);
  targetHz_ = noteHz(note);
  hz_ = glideFromHz > 0 ? glideFromHz : targetHz_;
  stage_ = Stage::Attack;
  if (!wasActive) { env_ = 0; side_ = {}; lfoPhase_ = 0; }
  fenv_ = 1.0;
  noise_ = 22222u + static_cast<std::uint32_t>(note) * 7919u;
  tick_ = 0;
  control(p);
}

void Voice::release() { if (active()) stage_ = Stage::Release; }

void Voice::control(const Patch& p) {
  auto r = [&](int s) { return toReal(s, p[s]); };
  atkStep_ = 1.0 / std::max(1.0, r(Attack) * sr_);
  decCoef_ = coefFor(r(Decay), sr_);
  sus_ = r(Sustain);
  relCoef_ = coefFor(r(Release), sr_);
  fenvCoef_ = coefFor(r(FilterDecay), sr_);
  shapeA_ = r(ShapeA);
  shapeB_ = r(ShapeB);
  ratioB_ = std::exp2(r(TuneB) / 12.0);
  detuneCt_ = r(Detune);
  mixB_ = r(MixB);
  sub_ = r(Sub);
  noiseAmt_ = r(Noise);
  fm_ = r(FM) * 4.0;
  const double d = r(Drive);
  drive_ = 1.0 + 12.0 * d * d;
  driveNorm_ = 1.0 / std::tanh(drive_);
  k_ = 2.0 - 2.0 * r(Resonance);
  gain_ = 0.4 * vel_ * std::pow(10.0, r(Level) / 20.0); // headroom for eight voices
  width_ = r(Width);
  lfoInc_ = r(LfoRate) / sr_;
  vibrato_ = r(Vibrato);
  wobble_ = r(Wobble);
  envSt_ = r(EnvAmount);
  cutoff_ = r(Cutoff);
  glideCoef_ = 1.0 - std::exp(-1.0 / std::max(1.0, r(Glide) * sr_));

  // The filter's frequency for this block, from its envelope and the LFO.
  const double lfo = std::sin(2 * kPi * lfoPhase_);
  const double hz = cutoff_ * std::exp2(envSt_ * fenv_ / 12.0 + wobble_ * lfo);
  const double g = std::tan(kPi * std::clamp(hz, 20.0, 0.45 * sr_) / sr_);
  g_ = { g, g };
}

void Voice::render(const Patch& p, float* left, float* right, int n) {
  float* out[2] = { left, right };
  for (int i = 0; i < n && active(); ++i) {
    if (tick_++ % 16 == 0) control(p);

    // Envelopes.
    switch (stage_) {
      case Stage::Attack: env_ += atkStep_; if (env_ >= 1.0) { env_ = 1.0; stage_ = Stage::Decay; } break;
      case Stage::Decay: env_ = sus_ + (env_ - sus_) * decCoef_; if (env_ - sus_ < 1e-4) stage_ = Stage::Sustain; break;
      case Stage::Sustain: env_ = sus_; break;
      case Stage::Release: env_ *= relCoef_; if (env_ < 1e-4) { stage_ = Stage::Off; env_ = 0; } break;
      case Stage::Off: break;
    }
    if (stage_ == Stage::Sustain && sus_ < 1e-4) stage_ = Stage::Off;
    fenv_ *= fenvCoef_;
    hz_ += (targetHz_ - hz_) * glideCoef_;
    const double lfo = std::sin(2 * kPi * lfoPhase_);
    lfoPhase_ += lfoInc_;
    lfoPhase_ -= std::floor(lfoPhase_);
    const double base = hz_ * std::exp2(vibrato_ * lfo / 12.0);

    noise_ ^= noise_ << 13; noise_ ^= noise_ >> 17; noise_ ^= noise_ << 5;
    const double white = (noise_ / 4294967295.0) * 2.0 - 1.0;

    for (int s = 0; s < 2; ++s) {
      Side& sd = side_[(size_t) s];
      const double spread = (s == 0 ? -1.0 : 1.0) * width_;
      const double hzA = base * std::exp2(spread * 7.0 / 1200.0);
      const double hzB = base * ratioB_ * std::exp2((detuneCt_ + spread * 20.0) / 1200.0);
      const double dtA = std::min(0.49, hzA / sr_), dtB = std::min(0.49, hzB / sr_);

      const double b = shaped(sd.b, dtB, shapeB_);
      double pa = sd.a + fm_ * b * 0.25;
      pa -= std::floor(pa);
      const double a = shaped(pa, dtA, shapeA_);
      const double sub = sd.sub < 0.5 ? 1.0 : -1.0;
      sd.a += dtA; sd.a -= std::floor(sd.a);
      sd.b += dtB; sd.b -= std::floor(sd.b);
      sd.sub += dtA * 0.5; sd.sub -= std::floor(sd.sub);

      double x = (1.0 - mixB_) * a + mixB_ * b + 0.6 * sub_ * sub + 0.5 * noiseAmt_ * white;
      x = std::tanh(x * drive_) * driveNorm_;

      // Low-pass (Simper's state-variable filter).
      const double g = g_[(size_t) s];
      const double a1 = 1.0 / (1.0 + g * (g + k_)), a2 = g * a1, a3 = g * a2;
      const double v3 = x - sd.ic2;
      const double v1 = a1 * sd.ic1 + a2 * v3;
      const double v2 = sd.ic2 + a2 * sd.ic1 + a3 * v3;
      sd.ic1 = 2 * v1 - sd.ic1;
      sd.ic2 = 2 * v2 - sd.ic2;
      double y = v2 * env_ * gain_;
      if (!std::isfinite(y)) { y = 0; sd.ic1 = sd.ic2 = 0; }
      out[s][i] += (float) y;
    }
  }
}

void Engine::prepare(double sampleRate) {
  for (auto& v : voices_) v.prepare(sampleRate);
  lastHz_ = 0;
}

void Engine::noteOn(int note, float velocity, const Patch& p) {
  if (velocity <= 0.0f) { noteOff(note); return; }
  Voice* pick = nullptr;
  for (auto& v : voices_) if (v.active() && v.note() == note) { pick = &v; break; }  // the same note again
  if (!pick) for (auto& v : voices_) if (!v.active()) { pick = &v; break; }         // a free voice
  if (!pick) {                                                                         // the oldest
    pick = &voices_[0];
    for (auto& v : voices_) if (v.age() < pick->age()) pick = &v;
  }
  // Glide from the last note played, when there is one and Glide is long enough to hear.
  const double from = toReal(Glide, p[Glide]) > 0.002 ? lastHz_ : 0.0;
  pick->start(note, velocity, p, from, ++age_);
  lastHz_ = 440.0 * std::exp2((note - 69) / 12.0);
}

void Engine::noteOff(int note) {
  for (auto& v : voices_) if (v.held() && v.note() == note) v.release();
}

void Engine::allNotesOff() { for (auto& v : voices_) v.release(); }

void Engine::render(const Patch& p, float* left, float* right, int n) {
  for (int i = 0; i < n; ++i) left[i] = right[i] = 0.0f;
  for (auto& v : voices_) if (v.active()) v.render(p, left, right, n);

  // Out: bits on the mix, then a soft knee so stacked voices round off instead of clipping.
  const double steps = std::pow(2.0, toReal(Bits, p[Bits]) - 1.0);
  for (float* ch : { left, right })
    for (int i = 0; i < n; ++i) {
      double s = std::round(ch[i] * steps) / steps;
      const double mag = std::fabs(s);
      if (mag > 0.7) s = std::copysign(0.7 + 0.3 * std::tanh((mag - 0.7) / 0.3), s);
      ch[i] = (float) s;
    }
}

int Engine::sounding() const {
  int n = 0;
  for (const auto& v : voices_) n += v.active();
  return n;
}

} // namespace synth
