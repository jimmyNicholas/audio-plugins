// Everything FX Breeder keeps, without JUCE: a litter of six chains, the Character and Wild that
// steer breeding, and the text that is saved with a set. The JUCE layer builds a Rack for the heard
// chain and hands it to the Player.
#pragma once
#include "chain.h"
#include <string>

namespace fx {

class Breeder {
public:
  static constexpr int kLitter = 6;

  explicit Breeder(std::uint32_t seed = 1);

  const breed::Litter<Chain>& litter() const { return litter_; }
  const Chain& heard() const { return litter_.heard(); }

  void select(int i) { litter_.select(i); }
  void breedFrom(int i) { litter_.breed(i, ops_); }
  void cross(int i, int j) { litter_.cross(i, j, ops_); }
  void fresh() { litter_.refresh(ops_); }
  bool back() { return litter_.back(); }

  void setCharacter(Character c) { ops_.character = c; }
  void setWild(bool on) { ops_.wild = on; }
  Character character() const { return ops_.character; }
  bool wild() const { return ops_.wild; }

  // By hand, on the heard chain: a setting, the mix, a unit's lock, or removing a unit.
  void setSetting(int unit, int param, double norm);
  void setMix(double mix);
  void setLock(int unit, bool on);
  void remove(int unit);

  std::string save() const;
  bool load(const std::string& text); // false, changing nothing, if it is not a saved state

private:
  ChainOps ops_;
  breed::Litter<Chain> litter_;
};

} // namespace fx
