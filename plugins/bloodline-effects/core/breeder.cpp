#include "breeder.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>

namespace fx {

namespace {
breed::Litter<Chain> makeLitter(std::uint32_t seed, ChainOps& ops) { return breed::Litter<Chain>(Breeder::kLitter, seed, ops); }
} // namespace

Breeder::Breeder(std::uint32_t seed) : litter_(makeLitter(seed, ops_)) {}

void Breeder::setSetting(int unit, int param, double norm) {
  Chain c = litter_.heard();
  if (unit < 0 || unit >= c.count || param < 0 || param >= kParams) return;
  c.units[(size_t) unit].p[(size_t) param] = std::clamp(norm, 0.0, 1.0);
  litter_.set(litter_.selected(), c);
}

void Breeder::setMix(double mix) {
  Chain c = litter_.heard();
  c.mix = std::clamp(mix, 0.0, 1.0);
  litter_.set(litter_.selected(), c);
}

void Breeder::setLock(int unit, bool on) {
  Chain c = litter_.heard();
  if (unit < 0 || unit >= c.count) return;
  c.units[(size_t) unit].locked = on;
  litter_.set(litter_.selected(), c);
}

void Breeder::remove(int unit) {
  Chain c = litter_.heard();
  if (unit < 0 || unit >= c.count || c.count <= 1) return;
  for (int i = unit; i < c.count - 1; ++i) c.units[(size_t) i] = c.units[(size_t) i + 1];
  c.count -= 1;
  c.units[(size_t) c.count] = Unit{};
  litter_.set(litter_.selected(), c);
}

// bloodline-effects 1
// litter <rng> <selected> <parent> <newInARow> <generation> <spread> <character> <wild>
// then one line per chain: <count> <mix>, and per unit: <type> <locked> <4 settings>
std::string Breeder::save() const {
  const auto st = litter_.state();
  std::ostringstream out;
  out.imbue(std::locale::classic());
  out << std::setprecision(17) << "bloodline-effects 1\nlitter " << st.rng << ' ' << st.selected << ' ' << st.parent << ' '
      << st.newCount << ' ' << st.generation << ' ' << st.spread << ' ' << static_cast<int>(ops_.character) << ' '
      << (ops_.wild ? 1 : 0) << '\n';
  for (const Chain& c : st.candidates) {
    out << c.count << ' ' << c.mix;
    for (int i = 0; i < c.count; ++i) {
      const Unit& u = c.units[(size_t) i];
      out << ' ' << static_cast<int>(u.type) << ' ' << (u.locked ? 1 : 0);
      for (double v : u.p) out << ' ' << v;
    }
    out << '\n';
  }
  return out.str();
}

bool Breeder::load(const std::string& text) {
  std::istringstream in(text);
  in.imbue(std::locale::classic());
  std::string word;
  int version = 0, character = 0, wild = 0;
  breed::Litter<Chain>::State st;
  if (!(in >> word >> version) || word != "bloodline-effects" || version != 1) return false;
  if (!(in >> word >> st.rng >> st.selected >> st.parent >> st.newCount >> st.generation >> st.spread >> character >> wild)) return false;
  if (word != "litter" || character < 0 || character >= kNumCharacters) return false;
  st.candidates.resize(kLitter);
  for (Chain& c : st.candidates) {
    if (!(in >> c.count >> c.mix) || c.count < 1 || c.count > kMaxUnits || !std::isfinite(c.mix)) return false;
    c.mix = std::clamp(c.mix, 0.0, 1.0);
    for (int i = 0; i < c.count; ++i) {
      Unit& u = c.units[(size_t) i];
      int type = 0, locked = 0;
      if (!(in >> type >> locked) || type < 0 || type >= kNumTypes) return false;
      u.type = static_cast<Type>(type);
      u.locked = locked != 0;
      for (double& v : u.p) {
        if (!(in >> v) || !std::isfinite(v)) return false;
        v = std::clamp(v, 0.0, 1.0);
      }
    }
  }
  breed::Litter<Chain> litter = litter_;
  if (!litter.restore(st)) return false;
  litter_ = litter;
  ops_.character = static_cast<Character>(character);
  ops_.wild = wild != 0;
  return true;
}

} // namespace fx
