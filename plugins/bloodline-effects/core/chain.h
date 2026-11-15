// Bloodline Effects's candidate: a chain of up to six effects, each with four settings, and a dry/wet mix.
// Breeding changes the settings, and also which effects there are and their order: an effect can
// turn into another, swap with its neighbour, appear or disappear. Wild makes those structural
// changes much more likely. A Character centres which effects appear, as zones do in Bloodline.
// Plain C++17, no JUCE.
#pragma once
#include "../../shared/breed/litter.h"
#include <array>

namespace fx {

enum class Type { Drive, Crush, Filter, Comb, Delay, Reverb, Gate, Ring, Tape, Comp, Width, Glitch };
constexpr int kNumTypes = 12;
constexpr int kParams = 4, kMaxUnits = 6;

struct TypeInfo {
  const char* name;
  const char* params[kParams];
  unsigned colour; // ARGB, for the UI
};
const TypeInfo& info(Type t);

struct Unit {
  Type type = Type::Drive;
  std::array<double, kParams> p{}; // normalised, 0 to 1
  bool locked = false;             // never changed, swapped or removed by breeding
  bool operator==(const Unit& o) const { return type == o.type && p == o.p && locked == o.locked; }
  bool operator!=(const Unit& o) const { return !(*this == o); }
};

struct Chain {
  int count = 0;
  std::array<Unit, kMaxUnits> units{};
  double mix = 1.0;      // dry/wet, bred
  bool operator==(const Chain& o) const;
  bool operator!=(const Chain& o) const { return !(*this == o); }
  // Two chains with the same effects in the same order can share a running rack: only settings differ.
  bool sameShape(const Chain& o) const;
};

enum class Character { None, Space, Dirt, Motion, Glitch };
constexpr int kNumCharacters = 5;
const char* characterName(Character c);

struct ChainOps {
  Character character = Character::None;
  bool wild = false;
  double characterShare = 0.8;

  Type drawType(breed::Rng& rng) const;
  Unit drawUnit(breed::Rng& rng) const;
  Chain fresh(breed::Rng& rng) const;
  Chain mutate(const Chain& from, double spread, breed::Rng& rng) const;
  Chain cross(const Chain& a, const Chain& b, breed::Rng& rng) const; // the front of one, the back of the other
};

} // namespace fx
