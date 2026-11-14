// PROTOTYPE (throwaway spike). Plays the heard bar on a loop: following the host's bar when the
// host is playing, or free-running at the host's tempo when Play is on. The pattern is read
// afresh on every block, so clicking through the litter changes the loop on its next hit. Each hit
// is a MIDI note (36 to 39); the JUCE layer also plays it on the built-in kit. Plain C++17, no JUCE.
#pragma once
#include "pattern.h"
#include "settings.h"
#include <array>
#include <vector>

namespace breaks {

struct NoteEvent { int offset, lane; float velocity; bool on; };

class Player {
public:
  void prepare(double sampleRate);

  // Advances one block of n samples. `hostBeat` is the host's position in quarter notes when it
  // is playing, or negative when it is not; then the loop runs on its own while `freeRun` is set,
  // from the top of the bar. Appends note-ons and offs to `events`, in order.
  void advance(const Pattern& p, int n, double bpm, double hostBeat, bool freeRun, std::vector<NoteEvent>& events);

  bool running() const { return running_; }
  double beat() const { return beat_; } // where the loop is in the bar, 0 to 4

private:
  double sr_ = 48000.0, freeBeat_ = 0.0, beat_ = 0.0;
  bool running_ = false, wasFree_ = false;
  std::array<int, kLanes> offAt_{ -1, -1, -1, -1 }; // samples from the block start; -1 is none pending
  std::array<Hit, kMaxHits> hits_{};
  std::vector<NoteEvent> ons_;
};

// The built-in sound for a role, as Bloodline Drums settings (normalised).
bloodline::Values soundFor(Role r);

} // namespace breaks
