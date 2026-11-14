// One breeding row (#37, #44, #45): three candidate pads, A, B and C, each row with its own zone,
// spread, favourite and locks, plus the row's Pan, Level and Mono. Whether a row is open or placed
// is the Instrument's business; a Row only breeds and keeps sounds. A value type: copy it to keep a
// snapshot.
#pragma once
#include "settings.h"
#include <array>
#include <cstddef>
#include <cstdint>

namespace bloodline {

// Tunable by ear (#37, #43).
struct Tuning {
  double startSpread = 0.25, minSpread = 0.02, maxSpread = 0.5;
  double narrow = 0.6;        // choosing the favourite again
  double widen = 1.6;         // choosing a new candidate twice in a row
  double zoneShare = 0.8;     // share of fresh draws taken from the zone
  double pull = 0.5;          // how far a mutation outside the zone moves back to its edge
};

// The row's own Pan and Level, added on top of each pad's (#45). Never bred, kept through a reset.
constexpr double kRowLevelMin = -24.0, kRowLevelMax = 6.0;

class Row {
public:
  static constexpr int kPads = 3;

  // Everything that is saved with a set, for restore().
  struct State {
    std::uint32_t seed = 1;
    Zone zone = Zone::None;
    std::array<Values, kPads> pads{};
    int favourite = -1, lastPressed = -1, lastPlayed = -1;
    double spread = 0.25;
    int newCount = 0;
    std::array<bool, kNumSettings> locks{};
    std::array<bool, kPads> padMono{};
    double rowPan = 0, rowLevel = 0;
    bool rowMono = false;
  };

  Row(std::uint32_t seed, Zone zone, Tuning tuning = {});

  // Pressing A, B or C (0 to 2) plays it and marks it as last pressed. Returns its values.
  const Values& press(int pad);

  // Commits the last pressed candidate. Returns false, changing nothing, if no candidate was
  // pressed since the last choice.
  bool choose();

  // Random on a placed row (#44): an equal pick on every hit, never the same pad twice in a row.
  // `r` is a draw from 0 to 1 that the caller makes. Returns the pad, and marks it as last played.
  int random(double r);

  // Three fresh candidates, no favourite, the starting spread. Keeps zone, locks, and the row's
  // Pan, Level and Mono (#44, #45).
  void reset();

  // Choosing Kick switches the row's Mono on (#45); it can be switched off again.
  void setZone(Zone zone);
  void setLock(int setting, bool on) { locks_[static_cast<size_t>(setting)] = on; }

  // Turning a setting by hand on one pad (#45). Does not lock, and does not change the spread.
  void setValue(int pad, int setting, double norm);
  void setPadMono(int pad, bool on) { padMono_[static_cast<size_t>(pad)] = on; }
  void setRowPan(double pan);
  void setRowLevel(double db);
  void setRowMono(bool on) { rowMono_ = on; }

  Zone zone() const { return zone_; }
  bool locked(int setting) const { return locks_[static_cast<size_t>(setting)]; }
  const Values& candidate(int pad) const { return pads_[static_cast<size_t>(pad)]; }
  int favourite() const { return favourite_; }      // -1 before the first choice
  int lastPressed() const { return lastPressed_; }  // -1 if none since the last choice
  int lastPlayed() const { return lastPlayed_; }    // the pad last heard, by any route; -1 if none
  bool hasChosen() const { return favourite_ >= 0; }
  const Values& chosen() const { return pads_[static_cast<size_t>(favourite_ < 0 ? 0 : favourite_)]; }
  double spread() const { return spread_; }
  int newInARow() const { return newCount_; }       // new candidates chosen in a row, 0 or 1
  std::uint32_t seed() const { return seed_; }
  const Tuning& tuning() const { return t_; }
  bool padMono(int pad) const { return padMono_[static_cast<size_t>(pad)]; }
  double rowPan() const { return rowPan_; }
  double rowLevel() const { return rowLevel_; }
  bool rowMono() const { return rowMono_; }

  // Where a pad sits and how loud, with the row's offsets and Mono applied: pan -1 to 1, level in dB.
  double padPan(int pad) const;
  double padLevel(int pad) const;

  State state() const;
  // For restoring saved state. Values are clamped and indexes checked.
  void restore(const State& s);

private:
  double rand();
  double gauss();
  Values freshDraw();
  Values mutate(const Values& from, double ownLevel);

  Tuning t_;
  std::uint32_t seed_;
  Zone zone_;
  std::array<Values, kPads> pads_{};
  std::array<bool, kNumSettings> locks_{};
  std::array<bool, kPads> padMono_{};
  int favourite_ = -1;
  int lastPressed_ = -1;
  int lastPlayed_ = -1;
  int lastRandom_ = -1;  // not saved (#44)
  double spread_;
  int newCount_ = 0;
  double rowPan_ = 0, rowLevel_ = 0;
  bool rowMono_ = false;
};

} // namespace bloodline
