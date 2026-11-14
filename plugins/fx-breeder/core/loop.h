// A built-in drum loop to feed the chain, so you can audition candidates without a clip playing
// (and in the Standalone app, which has no input). Two bars of boom bap from three tiny synthesised
// drums, at the host's tempo. Plain C++17, no JUCE.
#pragma once
#include <cstdint>

namespace fx {

class Loop {
public:
  void prepare(double sampleRate) { sr_ = sampleRate; pos_ = 0; kick_ = snare_ = hat_ = 0; }
  // Writes n samples of the loop into left and right.
  void render(float* left, float* right, int n, double bpm);

private:
  double sr_ = 48000, pos_ = 0; // pos_ in sixteenths
  double kick_ = 0, snare_ = 0, hat_ = 0, kickPhase_ = 0, hatHp_ = 0, hatPrev_ = 0, open_ = 0;
  std::uint32_t noise_ = 0x2468aceU;
};

} // namespace fx
