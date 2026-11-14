// The Litter seam (plugins/shared/breed): breeding, crossing, back and the spread, with patches.
#include "../../shared/test/check.h"
#include "patch.h"
#include <algorithm>

using namespace synth;
using Litter = breed::Litter<Patch>;

static double distance(const Patch& a, const Patch& b) {
  double d = 0;
  for (int i = 0; i < kNumSettings; ++i) d = std::max(d, std::fabs(a[i] - b[i]));
  return d;
}

TEST("a new litter: eight different candidates, no parent, the first one heard") {
  PatchOps ops;
  Litter l(8, 3, ops);
  CHECK_EQ(l.size(), 8);
  CHECK_EQ(l.parent(), -1);
  CHECK_EQ(l.selected(), 0);
  for (int i = 1; i < 8; ++i) CHECK(l.candidate(i) != l.candidate(0));
  CHECK(!l.canGoBack());
}

TEST("breeding: the candidate stays as the parent, the others are bred from it, nearer as the spread narrows") {
  PatchOps ops;
  double wide = 0, narrow = 0;
  for (std::uint32_t seed = 1; seed <= 100; ++seed) {
    Litter l(8, seed, ops);
    const Patch p = l.candidate(5);
    l.breed(5, ops);
    CHECK(l.candidate(5) == p);
    CHECK_EQ(l.parent(), 5);
    CHECK_EQ(l.selected(), 5);
    wide += distance(l.candidate(0), p);
    for (int i = 0; i < 5; ++i) l.breed(5, ops);
    narrow += distance(l.candidate(0), p);
  }
  CHECK(narrow < wide * 0.3);
}

TEST("the spread: the parent again narrows by 0.6, a new candidate twice widens by 1.6") {
  PatchOps ops;
  Litter l(8, 9, ops);
  l.breed(0, ops);
  CHECK_NEAR(l.spread(), 0.2, 1e-12);
  l.breed(0, ops);
  CHECK_NEAR(l.spread(), 0.12, 1e-12);
  l.breed(1, ops);
  CHECK_EQ(l.newInARow(), 1);
  l.breed(2, ops);
  CHECK_NEAR(l.spread(), 0.192, 1e-12);
  for (int i = 0; i < 30; ++i) l.breed(0, ops);
  CHECK_NEAR(l.spread(), 0.02, 1e-12);
}

TEST("crossing keeps both parents and fills the rest with children, each group from one side or a blend") {
  PatchOps ops;
  Litter l(8, 4, ops);
  const Patch a = l.candidate(1), b = l.candidate(6);
  l.cross(1, 6, ops);
  CHECK(l.candidate(1) == a);
  CHECK(l.candidate(6) == b);
  CHECK_EQ(l.parent(), 1);
  // A child sits between its parents, give or take a small mutation.
  for (int k : { 0, 2, 3, 4, 5, 7 })
    for (int i = 0; i < kNumSettings; ++i) {
      const double lo = std::min(a[i], b[i]) - 0.2, hi = std::max(a[i], b[i]) + 0.2;
      CHECK(l.candidate(k)[i] >= lo && l.candidate(k)[i] <= hi);
    }
  l.cross(3, 3, ops); // the same twice does nothing
  CHECK(l.candidate(1) == a);
}

TEST("back undoes breeding, crossing and fresh, one step at a time") {
  PatchOps ops;
  Litter l(8, 5, ops);
  const auto s0 = l.state();
  l.breed(2, ops);
  const auto s1 = l.state();
  l.cross(2, 4, ops);
  l.refresh(ops);
  CHECK_EQ(l.parent(), -1);
  CHECK(l.back());
  CHECK(l.back());
  CHECK(l.candidate(0) == s1.candidates[0]);
  CHECK_EQ(l.parent(), 2);
  CHECK(l.back());
  CHECK(l.candidate(0) == s0.candidates[0]);
  CHECK_EQ(l.parent(), -1);
  CHECK(!l.back());
}

TEST("locks copy the parent; Level is never bred; zones centre fresh draws") {
  PatchOps ops;
  ops.locks[Cutoff] = true;
  Litter l(8, 6, ops);
  l.breed(3, ops);
  for (int k = 0; k < 8; ++k) {
    CHECK_EQ(l.candidate(k)[Cutoff], l.candidate(3)[Cutoff]);
    CHECK_EQ(l.candidate(k)[Level], l.candidate(3)[Level]);
  }
  PatchOps bass;
  bass.zone = Zone::Bass;
  int in = 0, n = 0;
  for (std::uint32_t seed = 1; seed <= 300; ++seed) {
    Litter b(8, seed, bass);
    const Range r = zoneRange(Zone::Bass, Cutoff);
    for (int k = 0; k < 8; ++k, ++n) in += b.candidate(k)[Cutoff] >= r.lo && b.candidate(k)[Cutoff] <= r.hi;
  }
  CHECK(in / double(n) > 0.75);
}

TEST("state restores exactly and breeding carries on the same way") {
  PatchOps ops;
  Litter a(8, 7, ops), b(8, 99, ops);
  a.breed(1, ops);
  a.breed(4, ops);
  a.select(6);
  CHECK(b.restore(a.state()));
  a.breed(6, ops);
  b.breed(6, ops);
  for (int k = 0; k < 8; ++k) CHECK(a.candidate(k) == b.candidate(k));
  breed::Litter<Patch>::State wrong = a.state();
  wrong.candidates.pop_back();
  CHECK(!b.restore(wrong));
}

CHECK_MAIN
