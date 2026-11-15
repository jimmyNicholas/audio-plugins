// PROTOTYPE (throwaway spike). Bloodline Breaks 2: the drums (left) and the loops (right) in one
// plugin, so the loops play the kept drums without MIDI going between tracks. Whatever changes the
// keep pads also tells the loops, and swapping pads moves every loop's sounds with them. Also the
// loop player and the built-in sounds. Plain C++17, no JUCE.
#pragma once
#include "drums.h"
#include "loops.h"
#include <string>
#include <vector>

namespace bb2 {

// MIDI in, and in the dragged-out MIDI file.
constexpr int kFirstPadNote = 36;     // 36 to 43: the eight keep pads
constexpr int kFirstBuiltInNote = 44; // 44 to 47: a lane's built-in sound, when it has no pad
constexpr int kFirstSlotNote = 48;    // 48 to 55: switch to a kept loop

class Studio {
public:
  explicit Studio(std::uint32_t seed = 1);

  Drums drums;
  Loops loops;

  // Keep-pad changes go through here, so the loops hear about them.
  void keepDrum(int tile, int pad);
  void swapPads(int a, int b);
  void clearPad(int pad);

  std::string save() const;
  bool load(const std::string& text); // false, changing nothing, if it is not a saved state

private:
  void sync();
};

// The lane a drum candidate stands in for when the audition plays In break.
int laneFor(Zone z);

// The built-in sound for a lane, as Bloodline Drums settings (normalised).
Values builtIn(int lane);

// Plays a pattern on a loop: following the host's bar when the host is playing, or free-running
// at the host's tempo from the top of the bar. The pattern is read afresh each block.
class Player {
public:
  struct Event { int offset, lane; float velocity; };

  void prepare(double sampleRate) { sr_ = sampleRate; running_ = wasFree_ = false; freeBeat_ = beat_ = 0.0; }
  void advance(const Pattern& p, int n, double bpm, double hostBeat, bool freeRun, std::vector<Event>& out);
  bool running() const { return running_; }
  double beat() const { return beat_; } // 0 to 4

private:
  double sr_ = 48000.0, freeBeat_ = 0.0, beat_ = 0.0;
  bool running_ = false, wasFree_ = false;
  std::array<Hit, kMaxHits> hits_{};
};

} // namespace bb2
