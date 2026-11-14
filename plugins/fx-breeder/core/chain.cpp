#include "chain.h"
#include <algorithm>

namespace fx {

namespace {

const TypeInfo kInfo[kNumTypes] = {
  { "Drive",  { "Amount", "Fold", "Tone", "Bias" },        0xffe4572e },
  { "Crush",  { "Bits", "Rate", "Jitter", "Mix" },          0xfff2c14e },
  { "Filter", { "Cutoff", "Resonance", "Mode", "Sweep" },   0xff6cc3d5 },
  { "Comb",   { "Time", "Feedback", "Depth", "Rate" },      0xff7bdcb5 },
  { "Delay",  { "Time", "Feedback", "Tone", "Ping-pong" },  0xff8e9aff },
  { "Reverb", { "Size", "Decay", "Damping", "Mix" },        0xffb48cff },
  { "Gate",   { "Rate", "Duty", "Depth", "Smooth" },        0xffff8fab },
  { "Ring",   { "Freq", "Shape", "Drift", "Mix" },          0xffffb86b },
  { "Tape",   { "Wow", "Wow rate", "Flutter", "Saturate" }, 0xffc9a66b },
  { "Comp",   { "Threshold", "Ratio", "Attack", "Release" },0xff9be564 },
  { "Width",  { "Width", "Haas", "Spin", "Spin depth" },    0xff5ec2ff },
  { "Glitch", { "Chance", "Length", "Speed", "Mix" },       0xffff5d8f },
};

// Which effects each character draws from, 80% of the time.
const std::array<std::array<Type, 4>, kNumCharacters - 1> kCharacters = { {
  { Type::Reverb, Type::Delay, Type::Width, Type::Comb },     // Space
  { Type::Drive, Type::Crush, Type::Tape, Type::Comp },       // Dirt
  { Type::Filter, Type::Comb, Type::Gate, Type::Width },      // Motion
  { Type::Glitch, Type::Ring, Type::Gate, Type::Crush },      // Glitch
} };

} // namespace

const TypeInfo& info(Type t) { return kInfo[static_cast<int>(t)]; }

const char* characterName(Character c) {
  switch (c) {
    case Character::Space: return "Space";
    case Character::Dirt: return "Dirt";
    case Character::Motion: return "Motion";
    case Character::Glitch: return "Glitch";
    default: return "None";
  }
}

bool Chain::operator==(const Chain& o) const {
  if (count != o.count || mix != o.mix) return false;
  for (int i = 0; i < count; ++i) if (units[i] != o.units[i]) return false;
  return true;
}

bool Chain::sameShape(const Chain& o) const {
  if (count != o.count) return false;
  for (int i = 0; i < count; ++i) if (units[i].type != o.units[i].type) return false;
  return true;
}

Type ChainOps::drawType(breed::Rng& rng) const {
  if (character != Character::None && rng.uniform() < characterShare)
    return kCharacters[static_cast<size_t>(character) - 1][static_cast<size_t>(rng.below(4))];
  return static_cast<Type>(rng.below(kNumTypes));
}

Unit ChainOps::drawUnit(breed::Rng& rng) const {
  Unit u;
  u.type = drawType(rng);
  for (double& v : u.p) v = rng.uniform();
  return u;
}

Chain ChainOps::fresh(breed::Rng& rng) const {
  Chain c;
  c.count = 2 + rng.below(4); // 2 to 5
  for (int i = 0; i < c.count; ++i) c.units[i] = drawUnit(rng);
  c.mix = 0.4 + 0.6 * rng.uniform();
  return c;
}

// The chance of each structural change grows with the spread; Wild triples it.
Chain ChainOps::mutate(const Chain& from, double spread, breed::Rng& rng) const {
  Chain c = from;
  const double s = std::clamp(spread / 0.2, 0.1, 2.5) * (wild ? 3.0 : 1.0);
  for (int i = 0; i < c.count; ++i) {
    Unit& u = c.units[i];
    if (u.locked) continue;
    if (rng.chance(0.12 * s)) { u = drawUnit(rng); continue; }     // turns into another effect
    for (double& v : u.p) v = breed::fold(v + rng.gauss() * spread);
  }
  c.mix = breed::fold(c.mix + rng.gauss() * spread * 0.5);

  if (c.count > 1 && rng.chance(0.15 * s)) {                       // two neighbours swap
    const int i = rng.below(c.count - 1);
    if (!c.units[i].locked && !c.units[i + 1].locked) std::swap(c.units[i], c.units[i + 1]);
  }
  if (c.count < kMaxUnits && rng.chance(0.10 * s)) {               // a new effect appears
    const int at = rng.below(c.count + 1);
    for (int i = c.count; i > at; --i) c.units[i] = c.units[i - 1];
    c.units[at] = drawUnit(rng);
    c.count += 1;
  }
  if (c.count > 1 && rng.chance(0.10 * s)) {                       // one disappears
    const int at = rng.below(c.count);
    if (!c.units[at].locked) {
      for (int i = at; i < c.count - 1; ++i) c.units[i] = c.units[i + 1];
      c.count -= 1;
      c.units[c.count] = Unit{};
    }
  }
  if (wild && rng.chance(0.08)) {                                  // Wild only: the order is shuffled
    for (int i = c.count - 1; i > 0; --i) {
      const int j = rng.below(i + 1);
      if (!c.units[i].locked && !c.units[j].locked) std::swap(c.units[i], c.units[j]);
    }
  }
  return c;
}

Chain ChainOps::cross(const Chain& a, const Chain& b, breed::Rng& rng) const {
  Chain c;
  const int cutA = 1 + rng.below(a.count), cutB = rng.below(b.count);
  for (int i = 0; i < cutA && c.count < kMaxUnits; ++i) c.units[c.count++] = a.units[i];
  for (int i = cutB; i < b.count && c.count < kMaxUnits; ++i) c.units[c.count++] = b.units[i];
  c.mix = (a.mix + b.mix) / 2;
  return c;
}

} // namespace fx
