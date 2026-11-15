// The shared drum voice (#43, #45): one voice, 19 settings. A sine body with a pitch sweep and an
// attack, filtered noise with its own tail, Metal (the 808's six squares, tuned from Pitch), a
// tuned click, then drive, a bit and rate crusher, high-pass and low-pass filters, and pan. Kick,
// Snare and Hats are only different settings. Plain C++17, no JUCE.
#pragma once
#include "settings.h"
#include <cstdint>

namespace bloodline {

class Voice {
public:
  void prepare(double sampleRate);

  // Starts the sound, cutting this voice's own tail. Values are normalised; velocity is 0 to 1
  // and sets the level, with a little more click and drive when played harder (#37). Pan (-1 to
  // 1) and level (dB) are given separately, since the row adds its own and Mono centres (#45).
  void trigger(const Values& values, float velocity, double pan, double levelDb);
  // The same, with the values' own Pan and Level.
  void trigger(const Values& values, float velocity);

  // Adds n samples to left and right.
  void render(float* left, float* right, int n);

  bool active() const { return active_; }

private:
  double next(); // one mono sample, before pan

  double sr_ = 44100.0;
  bool active_ = false;

  // Fixed at trigger.
  double pitch_ = 60, sweepOct_ = 0, sweepCoef_ = 0, ampCoef_ = 0, noiseCoef_ = 0;
  double noiseMix_ = 0, metal_ = 0, click_ = 0, driveGain_ = 1, driveNorm_ = 1, hard_ = 0;
  double steps_ = 32768, holdInc_ = 1, level_ = 0, gainL_ = 1, gainR_ = 1;
  double svfG_ = 0, svfK_ = 1.4, clickG_ = 0, hpG_ = 0, lpG_ = 0, filterK_ = 2;

  // Running.
  double phase_ = 0, sweepEnv_ = 0, amp_ = 0, noiseAmp_ = 0, attack_ = 0, attackStep_ = 0;
  double noiseAttack_ = 0, noiseAttackStep_ = 0;
  double clickEnv_ = 0, clickCoef_ = 0;
  double sqPhase_[6] = {}, sqInc_[6] = {};
  double nLow_ = 0, nBand_ = 0, mLow_ = 0, mBand_ = 0, cLow_ = 0, cBand_ = 0;
  double hpLow_ = 0, hpBand_ = 0, lpLow_ = 0, lpBand_ = 0;
  double held_ = 0, holdPhase_ = 1;
  double declick_ = 0, declickCoef_ = 0, last_ = 0;
  std::uint32_t noise_ = 0x9e3779b9U;
};

} // namespace bloodline
