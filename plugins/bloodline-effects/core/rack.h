// A Rack is one chain, running: its effects, built and prepared up front (that is where every
// buffer is allocated). The Player plays one rack, fades from the last one when a new one comes in,
// and ends in a DC blocker and a limiter that are never bred, so a wild chain cannot get painful.
// Plain C++17, no JUCE.
#pragma once
#include "chain.h"
#include "effects.h"
#include <cmath>
#include <memory>
#include <vector>

namespace fx {

class Rack {
public:
  Rack(const Chain& chain, double sampleRate, int maxBlock); // allocates; build it off the audio thread

  const Chain& chain() const { return chain_; }
  // Takes new settings and mix when the effects and their order are the same; returns false otherwise.
  bool setSettings(const Chain& chain);
  // The chain's output, dry and wet mixed. n must be at most maxBlock.
  void process(const float* inL, const float* inR, float* outL, float* outR, int n);

private:
  Chain chain_;
  std::vector<std::unique_ptr<Effect>> effects_;
  std::vector<float> dryL_, dryR_;
};

class Player {
public:
  void prepare(double sampleRate, int maxBlock);
  double sampleRate() const { return sr_; }
  int maxBlock() const { return maxBlock_; }

  // The new rack plays from the next block, fading in over 40 ms as the current one fades out.
  // Returns the rack it no longer needs, if any, to be deleted off the audio thread.
  std::unique_ptr<Rack> use(std::unique_ptr<Rack> rack);
  // Plays this rack at once, with no fade (after prepare, when racks built for another block size
  // must not run). Returns the racks it drops, to be deleted off the audio thread.
  std::array<std::unique_ptr<Rack>, 2> replace(std::unique_ptr<Rack> rack);
  const Rack* current() const { return current_.get(); }
  bool setSettings(const Chain& chain) { return current_ && current_->setSettings(chain); }

  void setBypass(bool on) { bypass_ = on; }
  void setOutput(double db) { outGain_ = std::pow(10.0, db / 20.0); }

  // Processes left and right in place.
  void process(float* left, float* right, int n);

  double reduction() const { return limitGain_; } // the limiter's gain now, 1 when it is doing nothing

private:
  void block(float* left, float* right, int n);

  double sr_ = 48000;
  int maxBlock_ = 512;
  std::unique_ptr<Rack> current_, fading_;
  double fade_ = 1.0, fadeStep_ = 0.0005;
  bool bypass_ = false;
  double wet_ = 1.0, outGain_ = 1.0;
  double limitGain_ = 1.0, limitRelease_ = 0.9999;
  std::array<double, 2> dcX_{}, dcY_{};
  std::vector<float> inL_, inR_, aL_, aR_, bL_, bR_;
};

} // namespace fx
