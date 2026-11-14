#include "breeder.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>

namespace breaks {

namespace {
breed::Litter<Pattern> makeLitter(std::uint32_t seed, PatternOps& ops) { return breed::Litter<Pattern>(Breeder::kLitter, seed, ops); }
} // namespace

Breeder::Breeder(std::uint32_t seed) : litter_(makeLitter(seed, ops_)) {}

void Breeder::breedFrom(int i) {
  if (i < 0 || i >= kLitter) return;
  ops_.taste.learn(litter_.candidate(i));
  litter_.breed(i, ops_);
}

void Breeder::cross(int i, int j) {
  if (i < 0 || j < 0 || i >= kLitter || j >= kLitter || i == j) return;
  ops_.taste.learn(litter_.candidate(i));
  ops_.taste.learn(litter_.candidate(j));
  litter_.cross(i, j, ops_);
}

void Breeder::setStyle(Style s) {
  ops_.style = s;
  ops_.swingDial = styleInfo(s).swing;
}

void Breeder::setRole(int lane, Role r) {
  if (lane < 0 || lane >= kLanes || ops_.roles[(size_t) lane] == r) return;
  ops_.roles[(size_t) lane] = r;
  ops_.taste.forget(lane);
  if (r != Role::Off) return;
  for (int i = 0; i < kLitter; ++i) {
    Pattern p = litter_.candidate(i);
    p.vel[(size_t) lane] = {};
    litter_.set(i, p);
  }
}

void Breeder::cycleStep(int lane, int step) {
  if (lane < 0 || lane >= kLanes || step < 0 || step >= kSteps) return;
  Pattern p = litter_.heard();
  float& v = p.vel[(size_t) lane][(size_t) step];
  if (v <= 0.0f) { v = 0.9f; p.nudge[(size_t) lane][(size_t) step] = 0.0f; p.roll[(size_t) lane][(size_t) step] = 1; }
  else if (v > 0.5f) v = 0.35f;
  else v = 0.0f;
  litter_.set(litter_.selected(), p);
}

void Breeder::setSwing(double s) {
  Pattern p = litter_.heard();
  p.swing = std::clamp(s, kSwingMin, kSwingMax);
  litter_.set(litter_.selected(), p);
}

// bloodline-breaks 1
// ops <style> <swing dial> <pull> <4 roles> <4 locks as 0s and 1s> <taste count>
// taste <64 odds>
// litter <rng> <selected> <parent> <newInARow> <generation> <spread>
// then per candidate: <swing> <64 velocities> <64 nudges> <64 rolls>
std::string Breeder::save() const {
  const auto st = litter_.state();
  std::ostringstream out;
  out.imbue(std::locale::classic());
  out << std::setprecision(9) << "bloodline-breaks 1\nops " << (int) ops_.style << ' ' << ops_.swingDial << ' ' << ops_.pull;
  for (Role r : ops_.roles) out << ' ' << (int) r;
  out << ' ';
  for (bool b : ops_.locks) out << (b ? '1' : '0');
  out << ' ' << ops_.taste.count << "\ntaste";
  for (const auto& lane : ops_.taste.odds)
    for (double o : lane) out << ' ' << o;
  out << "\nlitter " << st.rng << ' ' << st.selected << ' ' << st.parent << ' ' << st.newCount << ' ' << st.generation << ' '
      << st.spread << '\n';
  for (const Pattern& p : st.candidates) {
    out << p.swing;
    for (const auto& lane : p.vel) for (float v : lane) out << ' ' << v;
    for (const auto& lane : p.nudge) for (float v : lane) out << ' ' << v;
    for (const auto& lane : p.roll) for (auto v : lane) out << ' ' << (int) v;
    out << '\n';
  }
  return out.str();
}

bool Breeder::load(const std::string& text) {
  std::istringstream in(text);
  in.imbue(std::locale::classic());
  std::string word, locks;
  int version = 0, styleIndex = 0;
  PatternOps ops = ops_;
  if (!(in >> word >> version) || word != "bloodline-breaks" || version != 1) return false;
  if (!(in >> word >> styleIndex >> ops.swingDial >> ops.pull) || word != "ops") return false;
  if (styleIndex < 0 || styleIndex >= kNumStyles || !std::isfinite(ops.swingDial) || !std::isfinite(ops.pull)) return false;
  ops.style = (Style) styleIndex;
  ops.swingDial = std::clamp(ops.swingDial, kSwingMin, kSwingMax);
  ops.pull = std::clamp(ops.pull, 0.0, 1.0);
  for (Role& r : ops.roles) {
    int k = 0;
    if (!(in >> k) || k < 0 || k >= kNumRoles) return false;
    r = (Role) k;
  }
  if (!(in >> locks >> ops.taste.count) || locks.size() != (size_t) kLanes) return false;
  for (int l = 0; l < kLanes; ++l) ops.locks[(size_t) l] = locks[(size_t) l] == '1';
  ops.taste.count = std::max(0, ops.taste.count);
  if (!(in >> word) || word != "taste") return false;
  for (auto& lane : ops.taste.odds)
    for (double& o : lane) {
      if (!(in >> o) || !std::isfinite(o)) return false;
      o = std::clamp(o, 0.0, 1.0);
    }
  breed::Litter<Pattern>::State st;
  if (!(in >> word >> st.rng >> st.selected >> st.parent >> st.newCount >> st.generation >> st.spread) || word != "litter") return false;
  st.candidates.resize(kLitter);
  for (Pattern& p : st.candidates) {
    if (!(in >> p.swing) || !std::isfinite(p.swing)) return false;
    p.swing = std::clamp(p.swing, kSwingMin, kSwingMax);
    for (auto& lane : p.vel) for (float& v : lane) { if (!(in >> v) || !std::isfinite(v)) return false; v = std::clamp(v, 0.0f, 1.0f); }
    for (auto& lane : p.nudge) for (float& v : lane) { if (!(in >> v) || !std::isfinite(v)) return false; v = std::clamp(v, -0.45f, 0.45f); }
    for (auto& lane : p.roll) for (auto& v : lane) { int k = 0; if (!(in >> k)) return false; v = (std::uint8_t) std::clamp(k, 1, 4); }
  }
  breed::Litter<Pattern> litter = litter_;
  if (!litter.restore(st)) return false;
  litter_ = litter;
  ops_ = ops;
  return true;
}

} // namespace breaks
