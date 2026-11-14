// The Row seam: one breeding row, seeded, driven only through press, choose, setZone and setLock.
#include "check.h"
#include "row.h"
#include <algorithm>

using namespace bloodline;

TEST("a new row has three candidates, no favourite, and choose does nothing until a pad is pressed") {
  Row row(42, Zone::Kick);
  CHECK_EQ(row.favourite(), -1);
  CHECK(!row.hasChosen());
  Values a = row.candidate(0);
  CHECK(!row.choose());
  CHECK_EQ(row.favourite(), -1);
  CHECK(row.candidate(0) == a);
  CHECK(row.candidate(0) != row.candidate(1));
  CHECK(row.candidate(1) != row.candidate(2));
}

static bool inZone(Zone z, int setting, double v) {
  Range r = zoneRange(z, setting);
  return v >= r.lo && v <= r.hi;
}

// Kick's pitch zone (40 to 90 Hz) is about 23% of the full 30 Hz to 1 kHz log range, so an 80%
// zone draw puts about 85% of pitches in it, and a draw with no zone about 23%.
TEST("fresh draws come from the zone about 80% of the time, and from anywhere with no zone") {
  int inKick = 0, inNone = 0, n = 0;
  for (std::uint32_t seed = 1; seed <= 2000; ++seed) {
    Row kick(seed, Zone::Kick), none(seed, Zone::None);
    for (int pad = 0; pad < Row::kPads; ++pad) {
      inKick += inZone(Zone::Kick, Pitch, kick.candidate(pad)[Pitch]);
      inNone += inZone(Zone::Kick, Pitch, none.candidate(pad)[Pitch]);
      n += 1;
    }
  }
  CHECK_NEAR(inKick / double(n), 0.85, 0.03);
  CHECK_NEAR(inNone / double(n), 0.23, 0.03);
}

TEST("every fresh value is between 0 and 1") {
  for (std::uint32_t seed = 1; seed <= 500; ++seed) {
    Row row(seed, Zone::Hats);
    for (int pad = 0; pad < Row::kPads; ++pad)
      for (double v : row.candidate(pad)) CHECK(v >= 0.0 && v <= 1.0);
  }
}

static double distance(const Values& a, const Values& b) {
  double d = 0;
  for (int i = 0; i < kNumSettings; ++i) d = std::max(d, std::fabs(a[i] - b[i]));
  return d;
}

TEST("choosing commits the last pressed candidate: it stays on its pad and the others are bred from it") {
  Row row(7, Zone::None);
  Values b = row.candidate(1);
  row.press(0);
  row.press(1);
  CHECK_EQ(row.lastPressed(), 1);
  CHECK(row.choose());
  CHECK_EQ(row.favourite(), 1);
  CHECK(row.hasChosen());
  CHECK(row.candidate(1) == b);
  CHECK(row.chosen() == b);
  CHECK_EQ(row.lastPressed(), -1);
  // Bred, not fresh: every setting within a few spreads (0.25) of the favourite.
  CHECK(row.candidate(0) != b && row.candidate(2) != b);
  CHECK(distance(row.candidate(0), b) < 1.0);
  CHECK(!row.choose()); // nothing pressed since
}

TEST("the first choice sets the favourite and leaves the spread alone") {
  Row row(7, Zone::Snare);
  row.press(2);
  row.choose();
  CHECK_NEAR(row.step(), 0.25, 1e-12);
  CHECK_EQ(row.newInARow(), 0);
}

TEST("bred candidates sit closer to the favourite as the spread narrows") {
  double wide = 0, narrow = 0;
  for (std::uint32_t seed = 1; seed <= 300; ++seed) {
    Row row(seed, Zone::None);
    row.press(0); row.choose();
    wide += distance(row.candidate(1), row.candidate(0));
    for (int i = 0; i < 5; ++i) { row.press(0); row.choose(); } // favourite again: narrows
    narrow += distance(row.candidate(1), row.candidate(0));
  }
  CHECK(narrow < wide * 0.3);
}

// Choose pad `pad` (pressing it first).
static void pick(Row& row, int pad) { row.press(pad); row.choose(); }
static int other(const Row& row) { return row.favourite() == 0 ? 1 : 0; }

TEST("spread: the favourite again narrows by 0.6, down to 0.02") {
  Row row(3, Zone::None);
  pick(row, 0);
  pick(row, 0);
  CHECK_NEAR(row.step(), 0.15, 1e-12);
  for (int i = 0; i < 20; ++i) pick(row, 0);
  CHECK_NEAR(row.step(), 0.02, 1e-12);
}

TEST("spread: a new candidate twice in a row widens by 1.6, up to 0.5, and the count restarts") {
  Row row(3, Zone::None);
  pick(row, 0);
  pick(row, other(row));
  CHECK_EQ(row.newInARow(), 1);
  CHECK_NEAR(row.step(), 0.25, 1e-12);
  pick(row, other(row));
  CHECK_NEAR(row.step(), 0.4, 1e-12);
  CHECK_EQ(row.newInARow(), 0);
  pick(row, other(row));
  pick(row, other(row));
  CHECK_NEAR(row.step(), 0.5, 1e-12);
}

TEST("spread: choosing the favourite in between restarts the new-candidate count") {
  Row row(3, Zone::None);
  pick(row, 0);
  pick(row, other(row));       // count 1
  pick(row, row.favourite());  // narrows to 0.15, count 0
  pick(row, other(row));       // count 1, no widening
  CHECK_NEAR(row.step(), 0.15, 1e-12);
  CHECK_EQ(row.newInARow(), 1);
}

// A row whose pad 0 is the favourite with every setting at `v`, at the narrowest spread.
static Row rowWithFavourite(std::uint32_t seed, double v, Zone zone) {
  Row row(seed, zone);
  Values fav; fav.fill(v);
  std::array<bool, kNumSettings> noLocks{};
  row.restore(row.seed(), zone, { fav, fav, fav }, 0, -1, 0.02, 0, noLocks);
  return row;
}

TEST("changing the zone keeps the candidates") {
  Row row(11, Zone::Hats);
  std::array<Values, 3> before{ row.candidate(0), row.candidate(1), row.candidate(2) };
  row.setZone(Zone::Kick);
  CHECK(row.zone() == Zone::Kick);
  for (int pad = 0; pad < 3; ++pad) CHECK(row.candidate(pad) == before[pad]);
}

// Kick's pitch zone tops out near 0.31 (90 Hz). A favourite at 0.8, bred at the narrowest spread,
// gives mutations near 0.8; with Kick on they are pulled halfway back, to about 0.56.
TEST("with a zone on, mutations outside it are pulled halfway back to its edge; the favourite is not") {
  const double hi = zoneRange(Zone::Kick, Pitch).hi;
  for (std::uint32_t seed = 1; seed <= 50; ++seed) {
    Row row = rowWithFavourite(seed, 0.8, Zone::Kick);
    pick(row, 0);
    CHECK_NEAR(row.candidate(0)[Pitch], 0.8, 1e-12);
    CHECK_NEAR(row.candidate(1)[Pitch], hi + (0.8 - hi) / 2, 0.06);
    CHECK_NEAR(row.candidate(2)[Pitch], hi + (0.8 - hi) / 2, 0.06);

    Row free = rowWithFavourite(seed, 0.8, Zone::None);
    pick(free, 0);
    CHECK_NEAR(free.candidate(1)[Pitch], 0.8, 0.08);
  }
}

TEST("mutations inside the zone are not pulled") {
  const Range r = zoneRange(Zone::Kick, Pitch);
  const double mid = (r.lo + r.hi) / 2;
  double total = 0;
  for (std::uint32_t seed = 1; seed <= 200; ++seed) {
    Row row = rowWithFavourite(seed, mid, Zone::Kick);
    pick(row, 0);
    total += row.candidate(1)[Pitch];
  }
  CHECK_NEAR(total / 200, mid, 0.005);
}

TEST("locked settings copy the favourite; unlocked ones still vary") {
  Row row(5, Zone::None);
  row.setLock(Decay, true);
  CHECK(row.locked(Decay));
  pick(row, 2);
  for (int i = 0; i < 4; ++i) pick(row, other(row));
  const Values& fav = row.candidate(row.favourite());
  for (int pad = 0; pad < 3; ++pad) CHECK_EQ(row.candidate(pad)[Decay], fav[Decay]);
  CHECK(row.candidate(other(row))[Pitch] != fav[Pitch]);
}

TEST("restore clamps values and rejects bad indexes") {
  Row row(1, Zone::None);
  Values wild; wild.fill(3.0);
  std::array<bool, kNumSettings> locks{};
  row.restore(9, Zone::Snare, { wild, wild, wild }, 7, 9, 99.0, 5, locks);
  CHECK_EQ(row.candidate(0)[Pitch], 1.0);
  CHECK_EQ(row.favourite(), -1);
  CHECK_EQ(row.lastPressed(), -1);
  CHECK_NEAR(row.step(), 0.5, 1e-12);
  CHECK(row.newInARow() <= 1);
  CHECK(row.zone() == Zone::Snare);
}

TEST("the same seed and the same presses give the same row; a restored row breeds on the same way") {
  Row a(77, Zone::Snare), b(77, Zone::Snare);
  for (int pad : { 0, 2, 2, 1, 0 }) { pick(a, pad); pick(b, pad); }
  for (int pad = 0; pad < 3; ++pad) CHECK(a.candidate(pad) == b.candidate(pad));

  std::array<bool, kNumSettings> locks{};
  Row c(1, Zone::None);
  c.restore(a.seed(), a.zone(), { a.candidate(0), a.candidate(1), a.candidate(2) }, a.favourite(),
            a.lastPressed(), a.step(), a.newInARow(), locks);
  pick(a, 1); pick(c, 1);
  for (int pad = 0; pad < 3; ++pad) CHECK(a.candidate(pad) == c.candidate(pad));
}

CHECK_MAIN
