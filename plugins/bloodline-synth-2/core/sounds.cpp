#include "sounds.h"
#include <algorithm>
#include <cmath>
#include <istream>
#include <ostream>

namespace bs2 {

using namespace synth;

namespace {
breed::Litter<Patch> makeLitter(std::uint32_t seed, PatchOps& ops) {
  return breed::Litter<Patch>(Sounds::kLitter, seed, ops, breed::Spread{ 0.25, 0.02, 0.5, 0.6, 1.6 });
}
bool validPad(int p) { return p >= 0 && p < Sounds::kPads; }
} // namespace

const char* padLabel(Zone z) { return z == Zone::None ? "Synth" : zoneName(z); }

Sounds::Sounds(std::uint32_t seed) : litter_(makeLitter(seed, ops_)) {}

bool Sounds::anyKept() const {
  for (const Pad& p : pads_) if (p.full) return true;
  return false;
}

void Sounds::setValue(int s, double norm) {
  if (s < 0 || s >= kNumSettings) return;
  Patch p = litter_.heard();
  p[(size_t) s] = std::clamp(norm, 0.0, 1.0);
  litter_.set(litter_.selected(), p);
}

void Sounds::keep(int tile, int p) {
  if (!validPad(p) || tile < 0 || tile >= kLitter) return;
  Pad& pad = pads_[(size_t) p];
  pad.full = true;
  pad.sound = litter_.candidate(tile);
  pad.zone = ops_.zone;
  pad.name = std::string(padLabel(ops_.zone)) + " " + std::to_string(++kept_);
}

void Sounds::swapPads(int a, int b) {
  if (validPad(a) && validPad(b)) std::swap(pads_[(size_t) a], pads_[(size_t) b]);
}

void Sounds::clearPad(int p) { if (validPad(p)) pads_[(size_t) p] = Pad{}; }

void Sounds::breedFromPad(int p) {
  if (!validPad(p) || !pads_[(size_t) p].full) return;
  const int at = litter_.selected();
  litter_.set(at, pads_[(size_t) p].sound);
  litter_.breed(at, ops_);
}

// sounds <zone> <locks as 0s and 1s> <kept so far>
// litter <rng> <selected> <parent> <newInARow> <generation> <spread>
// then four candidates of 24 values, then eight pads: <full> <zone> <24 values> <name, rest of line>
void Sounds::save(std::ostream& out) const {
  const auto st = litter_.state();
  out << "sounds " << (int) ops_.zone << ' ';
  for (bool b : ops_.locks) out << (b ? '1' : '0');
  out << ' ' << kept_ << "\nlitter " << st.rng << ' ' << st.selected << ' ' << st.parent << ' ' << st.newCount << ' ' << st.generation
      << ' ' << st.spread << '\n';
  for (const Patch& v : st.candidates) {
    for (int i = 0; i < kNumSettings; ++i) out << (i ? " " : "") << v[(size_t) i];
    out << '\n';
  }
  for (const Pad& p : pads_) {
    out << (p.full ? 1 : 0) << ' ' << (int) p.zone;
    for (double x : p.sound) out << ' ' << x;
    out << ' ' << (p.name.empty() ? "-" : p.name) << '\n';
  }
}

bool Sounds::load(std::istream& in) {
  std::string word, locks;
  int zone = 0, kept = 0;
  if (!(in >> word >> zone >> locks >> kept) || word != "sounds" || zone < 0 || zone >= kNumZones || locks.size() != (size_t) kNumSettings)
    return false;
  PatchOps ops = ops_;
  ops.zone = (Zone) zone;
  for (int i = 0; i < kNumSettings; ++i) {
    if (locks[(size_t) i] != '0' && locks[(size_t) i] != '1') return false;
    ops.locks[(size_t) i] = locks[(size_t) i] == '1';
  }
  breed::Litter<Patch>::State st;
  if (!(in >> word >> st.rng >> st.selected >> st.parent >> st.newCount >> st.generation >> st.spread) || word != "litter") return false;
  st.candidates.resize(kLitter);
  for (Patch& v : st.candidates)
    for (double& x : v) { if (!(in >> x) || !std::isfinite(x)) return false; x = std::clamp(x, 0.0, 1.0); }
  std::array<Pad, kPads> pads{};
  for (Pad& p : pads) {
    int full = 0, z = 0;
    if (!(in >> full >> z) || z < 0 || z >= kNumZones) return false;
    p.full = full == 1;
    p.zone = (Zone) z;
    for (double& x : p.sound) { if (!(in >> x) || !std::isfinite(x)) return false; x = std::clamp(x, 0.0, 1.0); }
    std::getline(in >> std::ws, p.name);
    if (p.name == "-" || !p.full) p.name.clear();
  }
  breed::Litter<Patch> litter = litter_;
  if (!litter.restore(st)) return false;
  litter_ = litter;
  ops_ = ops;
  pads_ = pads;
  kept_ = std::max(0, kept);
  return true;
}

} // namespace bs2
