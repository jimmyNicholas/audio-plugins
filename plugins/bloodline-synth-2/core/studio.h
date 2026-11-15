// PROTOTYPE (throwaway spike). Bloodline Synth 2: the sounds (left) and the loops (right) in one
// plugin, so the loops play the kept sounds without MIDI going between tracks. Keep-pad changes go
// through the studio, so the loops hear about them, and swapping pads moves every loop's sound with
// it. Plain C++17, no JUCE.
#pragma once
#include "loops.h"
#include "sounds.h"
#include <string>

namespace bs2 {

class Studio {
public:
  explicit Studio(std::uint32_t seed = 1);

  Sounds sounds;
  Loops loops;

  void keepSound(int tile, int pad);
  void swapPads(int a, int b);
  void clearPad(int pad);
  // New on both sides resets that side's spread; these are here so the editor has one place to call.
  void setSoundSpread(double s) { sounds.setSpread(s); }
  void setLoopSpread(double s) { loops.setSpread(s); }

  std::string save() const;
  bool load(const std::string& text); // false, changing nothing, if it is not a saved state

private:
  void sync();
};

} // namespace bs2
