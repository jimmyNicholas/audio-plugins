// A looping two-bar riff, so the synth can be heard hands-free while you click through candidates.
// Three patterns: a boom bap bass line, minor seventh chord stabs, and an arpeggio. It follows the
// host's tempo when there is one. Plain C++17, no JUCE.
#pragma once
#include <vector>

namespace synth {

class Riff {
public:
  static constexpr int kPatterns = 3;
  static const char* patternName(int p);

  struct Event { int offset, note; float velocity; bool on; };

  void prepare(double sampleRate) { sr_ = sampleRate; pos_ = 0; }
  void setPattern(int p);
  int pattern() const { return pattern_; }
  // Turning it off ends its notes (see advance).
  void setOn(bool on);
  bool on() const { return on_; }

  // Advances by n samples at `bpm`, appending note-ons and offs (with their offset in the block)
  // to `events`. When turned off, the notes still sounding end at offset 0.
  void advance(int n, double bpm, std::vector<Event>& events);

private:
  struct Step { int step, length; std::vector<int> notes; float velocity; };
  const std::vector<Step>& steps() const;

  double sr_ = 48000, pos_ = 0; // pos_ in sixteenths
  int pattern_ = 0;
  bool on_ = false, stopping_ = false;
  std::vector<int> sounding_;
};

} // namespace synth
