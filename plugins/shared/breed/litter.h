// A litter: the mouse-led breeding loop shared by Bloodline Synth and Bloodline Effects. Plain C++17,
// header only, no JUCE.
//
// A litter is a handful of candidates. Click one to hear it (select). Breed from it, and it stays
// where it is as the parent while every other candidate is bred from it. Cross two, and the others
// become their children. Back undoes the last breed, cross or fresh. The spread follows Bloodline's
// rules (#37): breeding from the parent again narrows it, breeding from a new candidate twice in a
// row widens it.
//
// What a candidate is, and how it is drawn, mutated and crossed, belongs to the plugin. Every
// breeding call takes an `ops` object with:
//   G fresh(Rng&);
//   G mutate(const G& from, double spread, Rng&);
//   G cross(const G& a, const G& b, Rng&);
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <vector>

namespace breed {

// Park-Miller, with the seed scattered first (small seeds give near-zero first draws).
class Rng {
public:
  static constexpr std::uint64_t kModulus = 2147483647;

  explicit Rng(std::uint32_t seed = 1) { reseed(seed); }

  void reseed(std::uint32_t x) {
    x ^= x >> 16; x *= 0x7feb352dU;
    x ^= x >> 15; x *= 0x846ca68bU;
    x ^= x >> 16;
    state_ = static_cast<std::uint32_t>(x % (kModulus - 1)) + 1;
  }
  // For restoring: a valid Park-Miller state is taken as is.
  void setState(std::uint32_t s) { if (s == 0 || s >= kModulus) reseed(s); else state_ = s; }
  std::uint32_t state() const { return state_; }

  double uniform() { // 0 to 1
    state_ = static_cast<std::uint32_t>((static_cast<std::uint64_t>(state_) * 16807) % kModulus);
    return (state_ - 1) / double(kModulus - 1);
  }
  double gauss() {
    const double u1 = std::max(uniform(), 1e-9), u2 = uniform();
    return std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * 3.14159265358979323846 * u2);
  }
  int below(int n) { return std::min(n - 1, static_cast<int>(uniform() * n)); }
  bool chance(double p) { return uniform() < p; }

private:
  std::uint32_t state_ = 1;
};

// Reflect off the ends, so values near an edge are not piled up on it.
inline double fold(double v) {
  if (v < 0) v = -v;
  if (v > 1) v = 2 - v;
  return std::clamp(v, 0.0, 1.0);
}

struct Spread {
  double start = 0.2, min = 0.02, max = 0.5;
  double narrow = 0.6, widen = 1.6;
};

template <class G>
class Litter {
public:
  // Everything that is saved: the candidates, which is heard, the parent, the spread, and the
  // random state. Back's history is not saved.
  struct State {
    std::vector<G> candidates;
    int selected = 0, parent = -1, newCount = 0, generation = 0;
    double spread = 0.2;
    std::uint32_t rng = 1;
  };

  template <class Ops>
  Litter(int size, std::uint32_t seed, Ops& ops, Spread spread = {}) : s_(spread), rng_(seed), spread_(spread.start) {
    cands_.reserve(static_cast<size_t>(size));
    for (int i = 0; i < size; ++i) cands_.push_back(ops.fresh(rng_));
  }

  int size() const { return static_cast<int>(cands_.size()); }
  const G& candidate(int i) const { return cands_[static_cast<size_t>(i)]; }
  const G& heard() const { return cands_[static_cast<size_t>(selected_)]; }
  int selected() const { return selected_; }
  int parent() const { return parent_; }       // -1 before the first breed
  double spread() const { return spread_; }
  int newInARow() const { return newCount_; }
  int generation() const { return generation_; }
  bool canGoBack() const { return !history_.empty(); }
  const Spread& rules() const { return s_; }
  Rng& rng() { return rng_; }

  void select(int i) { if (valid(i)) selected_ = i; }
  // Changing a candidate by hand. Does not change the spread.
  void set(int i, const G& g) { if (valid(i)) cands_[static_cast<size_t>(i)] = g; }
  void setSpread(double s) { spread_ = std::clamp(s, s_.min, s_.max); }

  // Candidate i becomes the parent and stays put; every other candidate is bred from it.
  template <class Ops>
  void breed(int i, Ops& ops) {
    if (!valid(i)) return;
    remember();
    if (parent_ >= 0) {
      if (i == parent_) { spread_ = std::max(s_.min, spread_ * s_.narrow); newCount_ = 0; }
      else if (++newCount_ >= 2) { spread_ = std::min(s_.max, spread_ * s_.widen); newCount_ = 0; }
    }
    parent_ = i;
    selected_ = i;
    const G from = cands_[static_cast<size_t>(i)];
    for (int k = 0; k < size(); ++k)
      if (k != i) cands_[static_cast<size_t>(k)] = ops.mutate(from, spread_, rng_);
    ++generation_;
  }

  // Candidates i and j stay; every other candidate is a child of the two, bred a little. i becomes
  // the parent. The spread is left alone.
  template <class Ops>
  void cross(int i, int j, Ops& ops) {
    if (!valid(i) || !valid(j) || i == j) return;
    remember();
    const G a = cands_[static_cast<size_t>(i)], b = cands_[static_cast<size_t>(j)];
    for (int k = 0; k < size(); ++k)
      if (k != i && k != j) cands_[static_cast<size_t>(k)] = ops.mutate(ops.cross(a, b, rng_), spread_ * 0.25, rng_);
    parent_ = i;
    selected_ = i;
    newCount_ = 0;
    ++generation_;
  }

  // A whole fresh litter: no parent, the starting spread.
  template <class Ops>
  void refresh(Ops& ops) {
    remember();
    for (auto& c : cands_) c = ops.fresh(rng_);
    parent_ = -1;
    newCount_ = 0;
    spread_ = s_.start;
    ++generation_;
  }

  // Undoes the last breed, cross or fresh. Returns false if there is nothing to go back to.
  bool back() {
    if (history_.empty()) return false;
    const Snapshot& snap = history_.back();
    cands_ = snap.candidates;
    parent_ = snap.parent;
    selected_ = snap.selected;
    spread_ = snap.spread;
    newCount_ = snap.newCount;
    generation_ = snap.generation;
    history_.pop_back();
    return true;
  }

  State state() const {
    return { cands_, selected_, parent_, newCount_, generation_, spread_, rng_.state() };
  }

  // Returns false, changing nothing, if the state does not fit this litter.
  bool restore(const State& st) {
    if ((int) st.candidates.size() != size()) return false;
    cands_ = st.candidates;
    selected_ = valid(st.selected) ? st.selected : 0;
    parent_ = valid(st.parent) ? st.parent : -1;
    newCount_ = std::clamp(st.newCount, 0, 1);
    generation_ = std::max(0, st.generation);
    spread_ = std::clamp(std::isfinite(st.spread) ? st.spread : s_.start, s_.min, s_.max);
    rng_.setState(st.rng);
    history_.clear();
    return true;
  }

private:
  static constexpr size_t kHistory = 32;
  struct Snapshot { std::vector<G> candidates; int parent, selected, newCount, generation; double spread; };

  bool valid(int i) const { return i >= 0 && i < size(); }
  void remember() {
    history_.push_back({ cands_, parent_, selected_, newCount_, generation_, spread_ });
    if (history_.size() > kHistory) history_.pop_front();
  }

  Spread s_;
  Rng rng_;
  std::vector<G> cands_;
  int selected_ = 0, parent_ = -1, newCount_ = 0, generation_ = 0;
  double spread_;
  std::deque<Snapshot> history_;
};

} // namespace breed
