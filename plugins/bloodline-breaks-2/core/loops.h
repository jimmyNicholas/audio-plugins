// PROTOTYPE (throwaway spike). Bloodline Breaks 2's right panel: Bloodline Breaks' one-bar loops,
// cut to a litter of four, with eight keep slots. The four lanes are fixed: kick, snare, hat and
// perc. A lane is a list of hits placed anywhere in the bar (the sixteenth grid is only a guide),
// and a sound: one or two of the left panel's kept pads, stacked or flammed. The breeder chooses
// the sounds, guided by the pads' labels. There are no built-in sounds: with nothing kept, a loop
// is silent. Plain C++17, no JUCE.
#pragma once
#include "../../shared/breed/litter.h"
#include <array>
#include <cstdint>
#include <iosfwd>

namespace bb2 {

constexpr int kLanes = 4, kSteps = 16, kKitPads = 8, kMaxLaneHits = 32;
constexpr double kSwingMin = 0.5, kSwingMax = 0.75; // MPC style: where the off sixteenth lands in its eighth
constexpr double kMergeGap = 0.25;                  // 1/64 of a bar, in sixteenths: closer hits are one
constexpr double kFlamMin = 8.0, kFlamMax = 40.0, kFlamLevelMin = 0.4, kFlamLevelMax = 0.8;

// The lanes, in the order they are kept; the editor draws Kick at the bottom.
enum Lane { KickLane, SnareLane, HatLane, PercLane };

// Soft pulls, never rules. None is flat odds: no lean, no scatter, no rolls.
enum class Style { None, BoomBap, LoFi, Strung, NeoSoul };
constexpr int kNumStyles = 5;

struct StyleInfo {
  const char* name;
  const char* odds[kLanes]; // per lane: sixteen digits, the chance of a hit in tenths (None: unused)
  double swing;             // where the swing dial goes when the style is chosen (None: left alone)
  double late, loose;       // the nudge's lean (late is positive) and scatter, in sixteenths
  double rolls;             // the chance a hat hit is a roll
  double softness;          // velocity scale
  int bpm;                  // typical tempo, shown only
};
const StyleInfo& styleInfo(Style s);

// Where a hit lands once swung: within each eighth, the first sixteenth's span is stretched to
// 2 * swing and the second squeezed into what is left. On the grid this is classic swing; off it,
// it bends smoothly. Both in sixteenths, 0 to 16.
double warp(double pos, double swing);
double unwarp(double landed, double swing);
double wrapBar(double pos); // into 0 to just under 16

struct Stroke {
  double pos = 0.0;     // sixteenths from the top of the bar, before swing
  float vel = 0.9f;     // 0.08 to 1
  std::uint8_t roll = 1; // hits in the sixteenth, 1 to 4
  bool operator==(const Stroke& o) const { return pos == o.pos && vel == o.vel && roll == o.roll; }
};

enum Layer { One, Stack, Flam };

struct LaneData {
  std::array<Stroke, kMaxLaneHits> hits{};
  int count = 0;
  std::int8_t first = -1, second = -1; // kept pads, -1 for none; a flam may use one pad twice
  std::uint8_t layer = One;            // matters only with two sounds
  float flam = 20.0f;                  // ms after the hit, kFlamMin to kFlamMax
  float flamLevel = 0.6f;              // of the hit's velocity

  bool add(double pos, float vel, std::uint8_t roll = 1); // false if full
  void remove(int i);
  void tidy();                  // sorts by position and merges hits closer than kMergeGap
  int find(double pos) const;   // a hit within kMergeGap, or -1
  bool layered() const { return second >= 0 && layer != One; }
  bool operator==(const LaneData& o) const;
  bool operator!=(const LaneData& o) const { return !(*this == o); }
};

struct Pattern {
  std::array<LaneData, kLanes> lanes{};
  double swing = 0.5;

  int hits() const;
  bool operator==(const Pattern& o) const { return lanes == o.lanes && swing == o.swing; }
  bool operator!=(const Pattern& o) const { return !(*this == o); }
};

// What the litter calls to breed patterns. `pull` (0 to 1) says how hard the style and the swing
// dial pull. `pads` is the left panel's keep row: each pad's zone (as an int), or -1 if empty.
struct PatternOps {
  Style style = Style::BoomBap;
  double swingDial = 0.58, pull = 0.7;
  std::array<bool, kLanes> locks{};
  std::array<int, kKitPads> pads{ -1, -1, -1, -1, -1, -1, -1, -1 };

  Pattern fresh(breed::Rng& rng) const;
  Pattern mutate(const Pattern& from, double spread, breed::Rng& rng) const;
  Pattern cross(const Pattern& a, const Pattern& b, breed::Rng& rng) const; // each lane whole from one parent

  double odds(int lane, int step) const;
  float velocityFor(int lane, int step, breed::Rng& rng) const;
  // The breeder's choice of sound for a lane: from pads labelled for it, else Drum (wildcard)
  // pads, and sometimes (rarely, more often for perc) any other kept pad. Sometimes two, stacked
  // or flammed. Leaves the lane with no sound if nothing is kept.
  void drawSound(LaneData& lane, int laneIndex, breed::Rng& rng) const;
  // The pads a lane might use, best first: labelled for it, then Drum pads, then the rest.
  int choices(int lane, int* out) const;
  bool kept(int pad) const { return pad >= 0 && pad < kKitPads && pads[(size_t) pad] >= 0; }
  bool anyKept() const;
};

// One sound to play: the hit's beat, which lane and pad, how hard, and how long after the beat
// (a flam's second note). `primary` marks the first sound of each hit.
struct Hit { double beat; int lane, pad; float velocity; double delayMs; bool primary; };
// Every sound of the bar, in beats from 0 to 4, with swing, rolls and layers applied. Lanes with
// no kept pad give nothing.
int hitsOf(const Pattern& p, const PatternOps& ops, Hit* out, int max);
constexpr int kMaxHits = kLanes * kMaxLaneHits * 4 * 2;

struct Slot { bool full = false; Pattern loop; };

class Loops {
public:
  static constexpr int kLitter = 4, kSlots = 8;

  explicit Loops(std::uint32_t seed = 1);

  const breed::Litter<Pattern>& litter() const { return litter_; }
  const PatternOps& ops() const { return ops_; }
  const Slot& slot(int s) const { return slots_[(size_t) s]; }
  int playingSlot() const { return playingSlot_; } // -1 when the heard tile plays
  const Pattern& playing() const;                  // what the loop plays and the inspector shows

  void select(int i) { litter_.select(i); playingSlot_ = -1; }
  void breedFrom(int i) { litter_.breed(i, ops_); playingSlot_ = -1; }
  void cross(int i, int j) { litter_.cross(i, j, ops_); playingSlot_ = -1; }
  void fresh() { litter_.refresh(ops_); playingSlot_ = -1; }
  bool back() { playingSlot_ = -1; return litter_.back(); }
  void setSpread(double s) { litter_.setSpread(s); }

  void setStyle(Style s);
  void setSwingDial(double s);
  void setPull(double p);
  void setLock(int lane, bool on) { if (lane >= 0 && lane < kLanes) ops_.locks[(size_t) lane] = on; }
  void setPads(const std::array<int, kKitPads>& zones) { ops_.pads = zones; }
  // Kit pads a and b swapped places: every loop follows its sounds to their new pads.
  void swapPads(int a, int b);

  // By hand, on what is playing. Positions are sixteenths before swing.
  int addHit(int lane, double pos, float vel); // the new hit's index, or -1
  void moveHit(int lane, int i, double pos, float vel);
  void removeHit(int lane, int i);
  void tidy(int lane); // after a drag: sort and merge
  void setSwing(double s);
  void cycleSound(int lane);       // the first sound, through the lane's choices, then none
  void cycleSecond(int lane);      // the second sound, the same way
  void setSound(int lane, int which, int pad); // which: 0 first, 1 second; pad -1 clears
  void cycleLayer(int lane);       // One, Stack, Flam
  void setFlam(int lane, double ms);

  // Keeping a loop leaves the litter as it is, for variations on it.
  void keep(int tile, int s);
  void swapSlots(int a, int b);
  void clearSlot(int s);
  void playSlot(int s);
  void breedFromSlot(int s); // the slot's loop becomes the parent of a new litter

  void save(std::ostream&) const;
  bool load(std::istream&);

private:
  template <class F> void change(F f);

  PatternOps ops_;
  breed::Litter<Pattern> litter_;
  std::array<Slot, kSlots> slots_{};
  int playingSlot_ = -1;
};

} // namespace bb2
