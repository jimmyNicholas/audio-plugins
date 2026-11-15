#include "drums.h"
#include <algorithm>
#include <cmath>
#include <istream>
#include <ostream>

namespace bb2 {

using namespace bloodline;

Values DrumOps::fresh(breed::Rng& rng) const {
  Values v{};
  for (int i = 0; i < kNumSettings; ++i) {
    Range r{ 0.0, 1.0 };
    if (zone != Zone::None && rng.uniform() < zoneShare) r = zoneRange(zone, i);
    v[(size_t) i] = r.lo + (r.hi - r.lo) * rng.uniform();
  }
  v[Level] = 1.0; // 0 dB, so a quiet candidate never sounds like a bad one
  return v;
}

Values DrumOps::mutate(const Values& from, double spread, breed::Rng& rng) const {
  Values v{};
  for (int i = 0; i < kNumSettings; ++i) {
    if (!info(i).bred || locks[(size_t) i]) { v[(size_t) i] = from[(size_t) i]; continue; }
    double x = breed::fold(from[(size_t) i] + rng.gauss() * spread);
    if (zone != Zone::None) {
      const Range r = zoneRange(zone, i);
      if (x < r.lo) x += (r.lo - x) * pull;
      else if (x > r.hi) x -= (x - r.hi) * pull;
    }
    v[(size_t) i] = x;
  }
  return v;
}

Values DrumOps::cross(const Values& a, const Values& b, breed::Rng& rng) const {
  Values v = a;
  for (int g = 0; g < kNumGroups; ++g) {
    const double r = rng.uniform();
    const double mix = r < 0.4 ? 0.0 : r < 0.8 ? 1.0 : rng.uniform();
    for (int i = group(g).first; i < group(g).first + group(g).count; ++i)
      if (!locks[(size_t) i] && info(i).bred) v[(size_t) i] = a[(size_t) i] + (b[(size_t) i] - a[(size_t) i]) * mix;
  }
  return v;
}

namespace {
breed::Litter<Values> makeLitter(std::uint32_t seed, DrumOps& ops) {
  return breed::Litter<Values>(Drums::kLitter, seed, ops, breed::Spread{ 0.25, 0.02, 0.5, 0.6, 1.6 });
}
bool validPad(int p) { return p >= 0 && p < Drums::kPads; }
const char* label(Zone z) { return z == Zone::None ? "Other" : zoneName(z); }
} // namespace

Drums::Drums(std::uint32_t seed) : litter_(makeLitter(seed, ops_)) {}

void Drums::setValue(int s, double norm) {
  if (s < 0 || s >= kNumSettings) return;
  Values v = litter_.heard();
  v[(size_t) s] = std::clamp(norm, 0.0, 1.0);
  litter_.set(litter_.selected(), v);
}

void Drums::keep(int tile, int p) {
  if (!validPad(p) || tile < 0 || tile >= kLitter) return;
  Pad& pad = pads_[(size_t) p];
  pad.full = true;
  pad.sound = litter_.candidate(tile);
  pad.zone = ops_.zone;
  pad.name = std::string(label(ops_.zone)) + " " + std::to_string(++kept_);
  litter_.refresh(ops_);
}

void Drums::swapPads(int a, int b) {
  if (validPad(a) && validPad(b)) std::swap(pads_[(size_t) a], pads_[(size_t) b]);
}

void Drums::clearPad(int p) { if (validPad(p)) pads_[(size_t) p] = Pad{}; }

void Drums::breedFromPad(int p) {
  if (!validPad(p) || !pads_[(size_t) p].full) return;
  const int at = litter_.selected();
  litter_.set(at, pads_[(size_t) p].sound);
  litter_.breed(at, ops_);
}

// drums <zone> <locks as 0s and 1s> <kept so far>
// litter <rng> <selected> <parent> <newInARow> <generation> <spread>
// then four candidates of 19 values, then eight pads: <full> <zone> <19 values> <name, rest of line>
void Drums::save(std::ostream& out) const {
  const auto st = litter_.state();
  out << "drums " << (int) ops_.zone << ' ';
  for (bool b : ops_.locks) out << (b ? '1' : '0');
  out << ' ' << kept_ << "\nlitter " << st.rng << ' ' << st.selected << ' ' << st.parent << ' ' << st.newCount << ' ' << st.generation
      << ' ' << st.spread << '\n';
  for (const Values& v : st.candidates) {
    for (int i = 0; i < kNumSettings; ++i) out << (i ? " " : "") << v[(size_t) i];
    out << '\n';
  }
  for (const Pad& p : pads_) {
    out << (p.full ? 1 : 0) << ' ' << (int) p.zone;
    for (double x : p.sound) out << ' ' << x;
    out << ' ' << (p.name.empty() ? "-" : p.name) << '\n';
  }
}

bool Drums::load(std::istream& in) {
  std::string word, locks;
  int zone = 0, kept = 0;
  if (!(in >> word >> zone >> locks >> kept) || word != "drums" || zone < 0 || zone >= kNumZones || locks.size() != (size_t) kNumSettings)
    return false;
  DrumOps ops = ops_;
  ops.zone = (Zone) zone;
  for (int i = 0; i < kNumSettings; ++i) ops.locks[(size_t) i] = locks[(size_t) i] == '1';
  breed::Litter<Values>::State st;
  if (!(in >> word >> st.rng >> st.selected >> st.parent >> st.newCount >> st.generation >> st.spread) || word != "litter") return false;
  st.candidates.resize(kLitter);
  for (Values& v : st.candidates)
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
  breed::Litter<Values> litter = litter_;
  if (!litter.restore(st)) return false;
  litter_ = litter;
  ops_ = ops;
  pads_ = pads;
  kept_ = std::max(0, kept);
  return true;
}

const char* Audition::patternName(int p) {
  switch (p) {
    case Quarters: return "Quarters";
    case BoomBap: return "Boom bap";
    case Sixteenths: return "16ths";
    default: return "In break";
  }
}

void Audition::advance(int n, double bpm, std::vector<Hit>& out) {
  if (!on_ || pattern_ == InBreak) return;
  static const char* kSteps[3] = { "9000900090009000", "9002009000609000", "9575957595759575" };
  const char* steps = kSteps[pattern_];
  const double perSample = std::clamp(bpm, 20.0, 400.0) / 60.0 * 4.0 / sr_; // sixteenths a sample
  const double start = pos_, end = pos_ + n * perSample;
  for (double t = std::ceil(start); t < end; t += 1.0) {
    const int v = steps[(int) std::fmod(t, 16.0)] - '0';
    if (v > 0) out.push_back({ std::clamp((int) std::lround((t - start) / perSample), 0, n - 1), v / 9.0f });
  }
  pos_ = std::fmod(end, 16.0);
}

} // namespace bb2
