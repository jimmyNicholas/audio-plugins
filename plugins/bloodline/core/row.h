// One breeding row (#37): three candidate pads, A, B and C, and a Choose pad. Each row has its
// own zone, spread, favourite and locks. A value type: copy it to keep a snapshot.
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

class Row {
public:
  static constexpr int kPads = 3;

  Row(std::uint32_t seed, Zone zone, Tuning tuning = {});

  // Pressing A, B or C (0 to 2) plays it and marks it as last pressed. Returns its values.
  const Values& press(int pad);

  // Commits the last pressed candidate. Returns false, changing nothing, if no candidate was
  // pressed since the last choice.
  bool choose();

  void setZone(Zone zone) { zone_ = zone; }
  void setLock(int setting, bool on) { locks_[static_cast<size_t>(setting)] = on; }

  Zone zone() const { return zone_; }
  bool locked(int setting) const { return locks_[static_cast<size_t>(setting)]; }
  const Values& candidate(int pad) const { return pads_[static_cast<size_t>(pad)]; }
  int favourite() const { return favourite_; }      // -1 before the first choice
  int lastPressed() const { return lastPressed_; }  // -1 if none since the last choice
  bool hasChosen() const { return favourite_ >= 0; }
  const Values& chosen() const { return pads_[static_cast<size_t>(favourite_ < 0 ? 0 : favourite_)]; } // the bottom pad
  double spread() const { return spread_; }
  int newInARow() const { return newCount_; }       // new candidates chosen in a row, 0 or 1
  std::uint32_t seed() const { return seed_; }
  const Tuning& tuning() const { return t_; }

  // For restoring saved state. Values are clamped and indexes checked.
  void restore(std::uint32_t seed, Zone zone, const std::array<Values, kPads>& pads, int favourite,
               int lastPressed, double spread, int newCount, const std::array<bool, kNumSettings>& locks);

private:
  double rand();
  double gauss();
  Values freshDraw();
  Values mutate(const Values& from);

  Tuning t_;
  std::uint32_t seed_;
  Zone zone_;
  std::array<Values, kPads> pads_{};
  std::array<bool, kNumSettings> locks_{};
  int favourite_ = -1;
  int lastPressed_ = -1;
  double spread_;
  int newCount_ = 0;
};

} // namespace bloodline
