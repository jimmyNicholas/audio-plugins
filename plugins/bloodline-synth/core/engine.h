// Bloodline Synth's sound: eight voices playing one patch. Two oscillators that morph from sine to
// triangle, saw and square, B phase-modulating A (FM), a sub an octave down and noise, then drive
// and a resonant low-pass with its own envelope, an LFO for vibrato and wobble, glide, and stereo
// width (each side detuned a little apart). Held notes follow the patch as it changes, so clicking
// through candidates while a chord or the riff plays changes the sound under your hands. Plain
// C++17, no JUCE.
#pragma once
#include "patch.h"
#include <array>
#include <cstdint>

namespace synth {

class Voice {
public:
  void prepare(double sampleRate);
  void start(int note, float velocity, const Patch& p, double glideFromHz, std::uint64_t age);
  void release();
  // Adds n samples of this voice, reading the patch as it is now.
  void render(const Patch& p, float* left, float* right, int n);

  bool active() const { return stage_ != Stage::Off; }
  bool held() const { return stage_ != Stage::Off && stage_ != Stage::Release; }
  int note() const { return note_; }
  std::uint64_t age() const { return age_; }
  double hz() const { return hz_; }

private:
  enum class Stage { Off, Attack, Decay, Sustain, Release };
  void control(const Patch& p); // every 16 samples

  double sr_ = 48000;
  Stage stage_ = Stage::Off;
  int note_ = 60;
  std::uint64_t age_ = 0;
  double vel_ = 1, hz_ = 261.6, targetHz_ = 261.6, glideCoef_ = 0;
  double env_ = 0, fenv_ = 0, lfoPhase_ = 0;
  int tick_ = 0;
  std::uint32_t noise_ = 22222;

  // From the patch, refreshed at control rate.
  double atkStep_ = 0, decCoef_ = 0, sus_ = 0, relCoef_ = 0, fenvCoef_ = 0;
  double shapeA_ = 0, shapeB_ = 0, ratioB_ = 1, detuneCt_ = 0, mixB_ = 0, sub_ = 0, noiseAmt_ = 0, fm_ = 0;
  double drive_ = 1, driveNorm_ = 1, k_ = 2, gain_ = 0, width_ = 0;
  double lfoInc_ = 0, vibrato_ = 0, wobble_ = 0, envSt_ = 0, cutoff_ = 1000;
  std::array<double, 2> g_{};

  struct Side { double a = 0, b = 0, sub = 0, ic1 = 0, ic2 = 0; };
  std::array<Side, 2> side_{};
};

class Engine {
public:
  static constexpr int kVoices = 8;

  void prepare(double sampleRate);
  void noteOn(int note, float velocity, const Patch& p);
  void noteOff(int note);
  void allNotesOff();
  // Overwrites left and right with n samples, every voice playing `p`.
  void render(const Patch& p, float* left, float* right, int n);
  int sounding() const;

private:
  std::array<Voice, kVoices> voices_;
  std::uint64_t age_ = 0;
  double lastHz_ = 0;
};

} // namespace synth
