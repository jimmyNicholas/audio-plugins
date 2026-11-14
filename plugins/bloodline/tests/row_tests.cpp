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

// Kick's pitch zone (40 to 90 Hz) is about 18% of the full 20 Hz to 2 kHz log range, so an 80%
// zone draw puts about 84% of pitches in it, and a draw with no zone about 18%.
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
  CHECK_NEAR(inKick / double(n), 0.835, 0.03);
  CHECK_NEAR(inNone / double(n), 0.176, 0.03);
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
  CHECK_NEAR(row.spread(), 0.25, 1e-12);
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
  CHECK_NEAR(row.spread(), 0.15, 1e-12);
  for (int i = 0; i < 20; ++i) pick(row, 0);
  CHECK_NEAR(row.spread(), 0.02, 1e-12);
}

TEST("spread: a new candidate twice in a row widens by 1.6, up to 0.5, and the count restarts") {
  Row row(3, Zone::None);
  pick(row, 0);
  pick(row, other(row));
  CHECK_EQ(row.newInARow(), 1);
  CHECK_NEAR(row.spread(), 0.25, 1e-12);
  pick(row, other(row));
  CHECK_NEAR(row.spread(), 0.4, 1e-12);
  CHECK_EQ(row.newInARow(), 0);
  pick(row, other(row));
  pick(row, other(row));
  CHECK_NEAR(row.spread(), 0.5, 1e-12);
}

TEST("spread: choosing the favourite in between restarts the new-candidate count") {
  Row row(3, Zone::None);
  pick(row, 0);
  pick(row, other(row));       // count 1
  pick(row, row.favourite());  // narrows to 0.15, count 0
  pick(row, other(row));       // count 1, no widening
  CHECK_NEAR(row.spread(), 0.15, 1e-12);
  CHECK_EQ(row.newInARow(), 1);
}

// A row whose pad 0 is the favourite with every setting at `v`, at the narrowest spread.
static Row rowWithFavourite(std::uint32_t seed, double v, Zone zone) {
  Row row(seed, zone);
  Values fav; fav.fill(v);
  Row::State s = row.state();
  s.pads = { fav, fav, fav };
  s.favourite = 0;
  s.lastPressed = -1;
  s.spread = 0.02;
  row.restore(s);
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
  Row::State s;
  s.seed = 9; s.zone = Zone::Snare; s.pads = { wild, wild, wild };
  s.favourite = 7; s.lastPressed = 9; s.lastPlayed = -4; s.spread = 99.0; s.newCount = 5;
  s.rowPan = 4; s.rowLevel = -100;
  row.restore(s);
  CHECK_EQ(row.candidate(0)[Pitch], 1.0);
  CHECK_EQ(row.favourite(), -1);
  CHECK_EQ(row.lastPressed(), -1);
  CHECK_NEAR(row.spread(), 0.5, 1e-12);
  CHECK(row.newInARow() <= 1);
  CHECK(row.zone() == Zone::Snare);
  CHECK_EQ(row.lastPlayed(), -1);
  CHECK_EQ(row.rowPan(), 1.0);
  CHECK_EQ(row.rowLevel(), kRowLevelMin);
}

TEST("the same seed and the same presses give the same row; a restored row breeds on the same way") {
  Row a(77, Zone::Snare), b(77, Zone::Snare);
  for (int pad : { 0, 2, 2, 1, 0 }) { pick(a, pad); pick(b, pad); }
  for (int pad = 0; pad < 3; ++pad) CHECK(a.candidate(pad) == b.candidate(pad));

  Row c(1, Zone::None);
  c.restore(a.state());
  pick(a, 1); pick(c, 1);
  for (int pad = 0; pad < 3; ++pad) CHECK(a.candidate(pad) == c.candidate(pad));
}

TEST("Random: an equal pick, never the same pad twice in a row, and it breeds nothing") {
  Row row(21, Zone::None);
  const Row::State before = row.state();
  std::uint32_t r = 99;
  auto next = [&r] { r = r * 1664525u + 1013904223u; return (r >> 8) / 16777216.0; };
  int counts[3] = {}, last = -1;
  for (int i = 0; i < 3000; ++i) {
    const int pad = row.random(next());
    CHECK(pad != last);
    CHECK_EQ(row.lastPlayed(), pad);
    counts[pad] += 1;
    last = pad;
  }
  for (int c : counts) CHECK_NEAR(c / 3000.0, 1.0 / 3.0, 0.03);
  for (int pad = 0; pad < 3; ++pad) CHECK(row.candidate(pad) == before.pads[pad]);
  CHECK_EQ(row.favourite(), -1);
}

TEST("reset: three fresh candidates, no favourite, the starting spread; zone, locks, Pan, Level and Mono kept") {
  Row row(4, Zone::Snare);
  row.setLock(Metal, true);
  row.setRowPan(-0.5);
  row.setRowLevel(-6);
  row.setRowMono(true);
  pick(row, 0);
  pick(row, 0);
  const Values old = row.candidate(1);
  row.reset();
  CHECK(row.candidate(1) != old);
  CHECK_EQ(row.favourite(), -1);
  CHECK_EQ(row.lastPressed(), -1);
  CHECK_EQ(row.lastPlayed(), -1);
  CHECK_NEAR(row.spread(), 0.25, 1e-12);
  CHECK(row.zone() == Zone::Snare);
  CHECK(row.locked(Metal));
  CHECK_EQ(row.rowPan(), -0.5);
  CHECK_EQ(row.rowLevel(), -6.0);
  CHECK(row.rowMono());
}

TEST("Level starts at 0 dB, is never bred, and each pad keeps its own") {
  Row row(8, Zone::None);
  for (int pad = 0; pad < 3; ++pad) CHECK_EQ(row.candidate(pad)[Level], 1.0);
  row.setValue(2, Level, 0.25);
  pick(row, 0);
  for (int i = 0; i < 5; ++i) pick(row, other(row));
  int quiet = 0;
  for (int pad = 0; pad < 3; ++pad) quiet += row.candidate(pad)[Level] == 0.25;
  CHECK_EQ(quiet, 1); // pad C keeps the level it was turned to, round after round
  CHECK_EQ(row.candidate(2)[Level], 0.25);
  for (int pad = 0; pad < 3; ++pad) CHECK(row.candidate(pad)[Level] == 1.0 || row.candidate(pad)[Level] == 0.25);
}

TEST("every unlocked setting except Level changes every round") {
  Row row(12, Zone::None);
  pick(row, 1);
  const Values& fav = row.candidate(1);
  for (int pad : { 0, 2 })
    for (int i = 0; i < kNumSettings; ++i)
      CHECK((i == Level) == (row.candidate(pad)[i] == fav[i]));
}

TEST("turning a setting by hand changes that pad; on the favourite, the next round breeds from it") {
  Row row = rowWithFavourite(30, 0.5, Zone::None);
  row.setValue(0, Pitch, 0.9);
  CHECK_EQ(row.candidate(0)[Pitch], 0.9);
  CHECK_NEAR(row.spread(), 0.02, 1e-12); // no change to the spread
  CHECK(!row.locked(Pitch));
  pick(row, 0);
  CHECK_NEAR(row.candidate(1)[Pitch], 0.9, 0.08);
  row.setValue(0, Pitch, 7.0);
  CHECK_EQ(row.candidate(0)[Pitch], 1.0);
}

TEST("Mono: the Kick zone switches the row's Mono on; pad or row Mono centres a pad and keeps its pan") {
  Row row(5, Zone::None);
  CHECK(!row.rowMono());
  row.setValue(0, Pan, 0.0); // hard left
  CHECK_EQ(row.padPan(0), -1.0);
  row.setRowPan(0.25);
  CHECK_EQ(row.padPan(0), -0.75);
  row.setPadMono(0, true);
  CHECK_EQ(row.padPan(0), 0.0);
  row.setPadMono(0, false);
  row.setZone(Zone::Kick);
  CHECK(row.rowMono());
  CHECK_EQ(row.padPan(0), 0.0);
  row.setRowMono(false);
  row.setZone(Zone::Kick); // already Kick: stays off
  CHECK(!row.rowMono());
  CHECK_EQ(row.padPan(0), -0.75);
  CHECK(Row(5, Zone::Kick).rowMono());
}

TEST("a pad's level is its own plus the row's, in dB") {
  Row row(5, Zone::None);
  row.setValue(1, Level, toNorm(Level, -6));
  row.setRowLevel(-3);
  CHECK_NEAR(row.padLevel(1), -9.0, 1e-9);
  CHECK_NEAR(row.padLevel(0), -3.0, 1e-9);
  row.setRowLevel(50);
  CHECK_EQ(row.rowLevel(), kRowLevelMax);
}

CHECK_MAIN
