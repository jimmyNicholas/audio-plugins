// The whole second spike without JUCE (#44, #45): a workbench and a kit of three slots on a 4x4
// pad grid, notes 36 to 51 (MPD226 Bank A), one voice per pad, and the state that is saved with a
// Live set. The JUCE processor only forwards notes and on-screen actions, renders, and saves and
// loads the string from save().
//
//  kit slot 3 (48-51)   [A] [B] [C] [random | choose]   blank: [ ] [ ] [ ] [place here]   line 3
//  kit slot 2 (44-47)   [A] [B] [C] [random | choose]                                     line 2
//  kit slot 1 (40-43)   [A] [B] [C] [random | choose]                                     line 1
//  workbench  (36-39)   [A] [B] [C] [choose]                                              line 0
//
// A "line" is a row of the grid: 0 is the workbench, 1 to 3 are the kit's slots. The sound follows
// the position: a slot's notes play whatever row is in that slot, like a Drum Rack pad.
#pragma once
#include "row.h"
#include "voice.h"
#include <array>
#include <optional>
#include <string>

namespace bloodline {

constexpr int kNumLines = 4, kBench = 0, kNumSlots = 3;
constexpr int kFirstNote = 36, kNumPads = 16;

struct Pad {
  enum Kind { None, Candidate, Choose, Random, Place, Blank } kind = None;
  int line = -1;  // 0 is the workbench, 1 to 3 the kit's slots
  int pad = -1;   // 0 to 2 for a candidate
};

// The note of a line's A, B or C (0 to 2), or its fourth pad (3).
int noteFor(int line, int pad);

class Instrument {
public:
  explicit Instrument(std::uint32_t seed = 1);

  void prepare(double sampleRate);

  // What a note does now. Notes outside 36 to 51 are None.
  Pad padFor(int note) const;

  void noteOn(int note, float velocity);
  void render(float* left, float* right, int n); // overwrites both

  // The row on a line, or nullptr for a blank slot.
  const Row* row(int line) const;
  // The workbench is always open; a placed row can be opened in place, and closed again (#44).
  bool isOpen(int line) const;

  // Each returns false, changing nothing, when it does not apply.
  bool place(int line);              // moves the workbench row into this blank slot, placed
  bool setOpen(int line, bool open); // a placed row only
  bool move(int line, int by);       // swaps a slot with its neighbour (by is +1 up or -1 down), blank or not
  bool reset(int line);              // open rows only: fresh candidates, keeps zone and locks
  bool clear(int line);              // empties a slot
  void startAgain();                 // an empty kit and a fresh workbench

  // Open rows only (#44, #45).
  bool setZone(int line, Zone zone);
  bool setLock(int line, int setting, bool on);
  bool setValue(int line, int setting, double norm); // on the row's last played pad
  bool setPadMono(int line, int pad, bool on);
  // Any row: the row's Pan, Level and Mono are like a mixer's, so they work placed too.
  bool setRowPan(int line, double pan);
  bool setRowLevel(int line, double db);
  bool setRowMono(int line, bool on);

  // A text form of the workbench and every slot. Random's last pick is not saved (#44).
  std::string save() const;
  // Returns false, changing nothing, if the text is not a saved state.
  bool load(const std::string& text);

private:
  Row* editable(int line);       // open rows only
  Row* any(int line);
  Row freshRow();
  double draw();                 // for Random, apart from the rows' own breeding
  void trigger(int note, int line, int pad, float velocity);

  Row bench_;
  std::array<std::optional<Row>, kNumSlots> slots_;
  std::array<bool, kNumSlots> open_{};
  std::uint32_t nextSeed_;
  std::uint32_t pick_ = 0x2545f491U;
  std::array<Voice, kNumPads> voices_;
};

} // namespace bloodline
