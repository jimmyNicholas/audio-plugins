// The whole spike without JUCE (#37): three rows on a 4x4 pad grid, notes 36 to 51, one voice per
// pad, and the state that is saved with a Live set. The JUCE processor only forwards notes,
// renders, and saves and loads the string from save().
//
//  row 1 (48-51)   [A] [B] [C] [choose]     Hats by default     rows index 0
//  row 2 (44-47)   [A] [B] [C] [choose]     Snare by default    rows index 1
//  row 3 (40-43)   [A] [B] [C] [choose]     Kick by default     rows index 2
//  bottom (36-39)  [row 3] [row 2] [row 1] [empty]
#pragma once
#include "row.h"
#include "voice.h"
#include <array>
#include <string>

namespace bloodline {

constexpr int kNumRows = 3;
constexpr int kFirstNote = 36, kNumPads = 16;

struct Pad {
  enum Kind { None, Candidate, Choose, Bottom, Empty } kind = None;
  int row = -1;   // rows index, 0 is row 1 (the top)
  int pad = -1;   // 0 to 2 for a candidate
};

// What a note does. Notes outside 36 to 51 are None.
Pad padFor(int note);
// The note of a row's candidate (0 to 2), Choose (3), or bottom pad.
int candidateNote(int row, int pad);
int chooseNote(int row);
int bottomNote(int row);

class Instrument {
public:
  explicit Instrument(std::uint32_t seed = 1);

  void prepare(double sampleRate);
  void noteOn(int note, float velocity);
  void render(float* out, int n); // overwrites out

  const Row& row(int i) const { return rows_[static_cast<size_t>(i)]; }
  void setZone(int row, Zone zone) { rows_[static_cast<size_t>(row)].setZone(zone); }
  void setLock(int row, int setting, bool on) { rows_[static_cast<size_t>(row)].setLock(setting, on); }

  // A text form of every row: seeds, zones, candidates, favourite, last pressed, spread, locks.
  std::string save() const;
  // Returns false, changing nothing, if the text is not a saved state.
  bool load(const std::string& text);

private:
  std::array<Row, kNumRows> rows_;
  std::array<Voice, kNumPads> voices_;
};

} // namespace bloodline
