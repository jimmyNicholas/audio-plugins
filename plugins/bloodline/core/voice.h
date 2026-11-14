// The shared drum voice (#43): one voice, eight settings. A sine body with a pitch sweep, filtered
// noise, Metal (the 808's six squares, tuned from Pitch), a click, and drive. Kick, Snare and Hats
// are only different settings. Plain C++17, no JUCE.
#pragma once
#include "settings.h"
#include <cstdint>

namespace bloodline {

class Voice {
public:
  void prepare(double sampleRate);

  // Starts the sound, cutting this voice's own tail. Values are normalised; velocity is 0 to 1
  // and sets the level, with a little more click and drive when played harder (#37).
  void trigger(const Values& values, float velocity);

  // Adds n samples to out.
  void render(float* out, int n);

  bool active() const { return active_; }

private:
  float next();

  double sr_ = 44100.0;
  bool active_ = false;

  // Fixed at trigger.
  double pitch_ = 60, sweepOct_ = 0, sweepCoef_ = 0, ampCoef_ = 0;
  double noiseMix_ = 0, metal_ = 0, click_ = 0, driveGain_ = 1, driveNorm_ = 1, level_ = 0;
  double svfG_ = 0, svfK_ = 1.4;

  // Running.
  double phase_ = 0, sweepEnv_ = 0, amp_ = 0, attack_ = 0, attackStep_ = 0;
  double clickEnv_ = 0, clickCoef_ = 0;
  double sqPhase_[6] = {}, sqInc_[6] = {};
  double nLow_ = 0, nBand_ = 0, mLow_ = 0, mBand_ = 0, clickHp_ = 0, clickPrev_ = 0;
  double declick_ = 0, declickCoef_ = 0, last_ = 0;
  std::uint32_t noise_ = 0x9e3779b9U;
};

} // namespace bloodline
