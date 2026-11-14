#include "breeder.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>

namespace synth {

namespace {
breed::Litter<Patch> makeLitter(std::uint32_t seed, PatchOps& ops) { return breed::Litter<Patch>(Breeder::kLitter, seed, ops); }
} // namespace

Breeder::Breeder(std::uint32_t seed) : litter_(makeLitter(seed, ops_)) {}

void Breeder::setValue(int setting, double norm) {
  if (setting < 0 || setting >= kNumSettings) return;
  Patch p = litter_.heard();
  p[(size_t) setting] = std::clamp(norm, 0.0, 1.0);
  litter_.set(litter_.selected(), p);
}

// bloodline-synth 1
// litter <rng> <selected> <parent> <newInARow> <generation> <spread> <zone> <locks as 0s and 1s>
// then one line of 24 values per candidate.
std::string Breeder::save() const {
  const auto st = litter_.state();
  std::ostringstream out;
  out.imbue(std::locale::classic());
  out << std::setprecision(17) << "bloodline-synth 1\nlitter " << st.rng << ' ' << st.selected << ' ' << st.parent << ' '
      << st.newCount << ' ' << st.generation << ' ' << st.spread << ' ' << static_cast<int>(ops_.zone) << ' ';
  for (bool b : ops_.locks) out << (b ? '1' : '0');
  out << '\n';
  for (const Patch& p : st.candidates) {
    for (int i = 0; i < kNumSettings; ++i) out << (i ? " " : "") << p[(size_t) i];
    out << '\n';
  }
  return out.str();
}

bool Breeder::load(const std::string& text) {
  std::istringstream in(text);
  in.imbue(std::locale::classic());
  std::string word, locks;
  int version = 0, zone = 0;
  breed::Litter<Patch>::State st;
  if (!(in >> word >> version) || word != "bloodline-synth" || version != 1) return false;
  if (!(in >> word >> st.rng >> st.selected >> st.parent >> st.newCount >> st.generation >> st.spread >> zone >> locks)) return false;
  if (word != "litter" || zone < 0 || zone >= kNumZones || locks.size() != (size_t) kNumSettings) return false;
  PatchOps ops = ops_;
  for (int i = 0; i < kNumSettings; ++i) {
    if (locks[(size_t) i] != '0' && locks[(size_t) i] != '1') return false;
    ops.locks[(size_t) i] = locks[(size_t) i] == '1';
  }
  ops.zone = static_cast<Zone>(zone);
  st.candidates.resize(kLitter);
  for (Patch& p : st.candidates)
    for (double& v : p) {
      if (!(in >> v) || !std::isfinite(v)) return false;
      v = std::clamp(v, 0.0, 1.0);
    }
  breed::Litter<Patch> litter = litter_;
  if (!litter.restore(st)) return false;
  litter_ = litter;
  ops_ = ops;
  return true;
}

} // namespace synth
