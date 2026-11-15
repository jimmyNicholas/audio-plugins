// PROTOTYPE (throwaway spike). Bloodline Breaks' pattern: one bar of up to four drums on pads 36
// to 39, as sixteen steps a lane, with velocity, a nudge off the grid and rolls, plus the bar's
// own swing. How a pattern is drawn, mutated and crossed for the shared litter, with the soft
// pulls: the style (a hip hop preset), the swing dial, and the taste learned from what you breed.
// Plain C++17, no JUCE.
#pragma once
#include "../../shared/breed/litter.h"
#include <array>
#include <cstdint>

namespace breaks {

constexpr int kLanes = 4, kSteps = 16, kFirstNote = 36;
constexpr double kSwingMin = 0.5, kSwingMax = 0.75; // MPC style: where the off sixteenth lands in its eighth

// What a lane plays. It steers breeding (a kick likes the one, a snare the backbeat) and picks the
// built-in sound. Off silences the lane, for fewer than four drums.
enum class Role { Kick, Snare, Hat, Perc, Off };
constexpr int kNumRoles = 5;
const char* roleName(Role r);

// Hip hop presets. None of them is a rule: each is a soft pull on breeding.
enum class Style { Free, BoomBap, LoFi, Dilla, Trap, Drill, WestCoast };
constexpr int kNumStyles = 7;

struct StyleInfo {
  const char* name;
  const char* odds[4];   // per role (Kick, Snare, Hat, Perc): sixteen digits, the chance of a hit in tenths
  double swing;          // where the swing dial goes when the style is chosen
  double late, loose;    // the nudge's lean (late is positive) and scatter, in steps
  double rolls;          // the chance a hat step is a roll
  int rollKind;          // 0: 32nds, triplets or 64ths at random; otherwise always that many hits
  double softness;       // velocity scale
  int bpm;               // typical tempo, shown only
};
const StyleInfo& styleInfo(Style s);

struct Pattern {
  std::array<std::array<float, kSteps>, kLanes> vel{};   // 0 is a rest
  std::array<std::array<float, kSteps>, kLanes> nudge{}; // -0.45 to 0.45 of a step
  std::array<std::array<std::uint8_t, kSteps>, kLanes> roll{}; // hits in the step, 1 to 4
  double swing = 0.5;

  bool on(int lane, int step) const { return vel[(size_t) lane][(size_t) step] > 0.0f; }
  int hits() const;
  bool operator==(const Pattern& o) const { return vel == o.vel && nudge == o.nudge && roll == o.roll && swing == o.swing; }
  bool operator!=(const Pattern& o) const { return !(*this == o); }
};

// What you have bred from, as the chance of a hit on each step. Learned on every breed and cross,
// and a soft pull on fresh litters, so fresh draws drift towards what you keep choosing.
struct Taste {
  std::array<std::array<double, kSteps>, kLanes> odds{};
  int count = 0;
  void learn(const Pattern& p);
  void forget(int lane = -1); // -1 forgets every lane
  double weight() const;      // 0 before anything is learned, up to 0.5
};

// What the litter calls to breed patterns. Every pull is soft: `pull` (0 to 1) says how hard the
// style and the swing dial pull, and nothing is ever forced.
struct PatternOps {
  Style style = Style::BoomBap;
  double swingDial = 0.58, pull = 0.7;
  std::array<Role, kLanes> roles{ Role::Kick, Role::Snare, Role::Hat, Role::Perc };
  std::array<bool, kLanes> locks{};
  Taste taste;

  Pattern fresh(breed::Rng& rng) const;
  Pattern mutate(const Pattern& from, double spread, breed::Rng& rng) const;
  Pattern cross(const Pattern& a, const Pattern& b, breed::Rng& rng) const; // each lane from one parent

  // The chance of a hit on a step, with the style pulled in by `pull`.
  double odds(int lane, int step) const;
  float velocityFor(int lane, int step, breed::Rng& rng) const;
};

// One hit in the bar, for playing and for the MIDI file.
struct Hit { double beat; int lane; float velocity; };
// Every hit of the bar, in beats from 0 to 4, with swing, nudges and rolls applied. Hits nudged
// before the bar wrap round to its end.
int hitsOf(const Pattern& p, Hit* out, int max);
constexpr int kMaxHits = kLanes * kSteps * 4;

} // namespace breaks
