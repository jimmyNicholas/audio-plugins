// PROTOTYPE (throwaway spike). Just enough checks to trust the loop: breeding, the soft pulls,
// timing and saving.
#include "../../shared/test/check.h"
#include "breeder.h"
#include "player.h"

using namespace breaks;

static int laneHits(const Pattern& p, int lane) {
  int n = 0;
  for (int s = 0; s < kSteps; ++s) n += p.on(lane, s);
  return n;
}

TEST("an Off lane stays silent through fresh litters, breeding and crossing") {
  Breeder b(4);
  b.setRole(3, Role::Off);
  for (int i = 0; i < Breeder::kLitter; ++i) CHECK_EQ(laneHits(b.litter().candidate(i), 3), 0);
  b.fresh();
  b.breedFrom(2);
  b.cross(2, 5);
  for (int i = 0; i < Breeder::kLitter; ++i) CHECK_EQ(laneHits(b.litter().candidate(i), 3), 0);
}

TEST("styles pull softly: Boom bap snares land on the backbeat far more often than elsewhere") {
  int back = 0, other = 0;
  for (std::uint32_t seed = 1; seed <= 200; ++seed) {
    Breeder b(seed);
    const Pattern& p = b.litter().candidate(0);
    back += p.on(1, 4) + p.on(1, 12);
    for (int s : { 0, 2, 6, 8, 10, 14 }) other += p.on(1, s);
  }
  CHECK(back > 200);   // of 400
  CHECK(other < 120);  // of 1200, but not never
  CHECK(other > 0);
}

TEST("pull 0 frees the style; pull 1 holds it hard") {
  auto backbeat = [](double pull) {
    int n = 0;
    for (std::uint32_t seed = 1; seed <= 200; ++seed) {
      Breeder b(seed);
      b.setPull(pull);
      b.fresh();
      n += b.litter().candidate(0).on(1, 4);
    }
    return n;
  };
  CHECK(backbeat(1.0) > backbeat(0.0) + 80);
}

TEST("the swing dial is a soft pull: fresh swing gathers round it but is not fixed") {
  Breeder b(7);
  b.setSwingDial(0.66);
  b.setPull(0.6);
  b.fresh();
  double sum = 0;
  bool allSame = true;
  for (int i = 0; i < Breeder::kLitter; ++i) {
    sum += b.litter().candidate(i).swing;
    allSame = allSame && b.litter().candidate(i).swing == b.litter().candidate(0).swing;
  }
  CHECK_NEAR(sum / Breeder::kLitter, 0.66, 0.05);
  CHECK(!allSame);
}

TEST("swing moves only the off sixteenths") {
  Pattern p;
  p.vel[0][0] = p.vel[0][1] = 1.0f;
  for (auto& l : p.roll) l.fill(1);
  p.swing = 0.66;
  Hit h[4];
  CHECK_EQ(hitsOf(p, h, 4), 2);
  CHECK_NEAR(h[0].beat, 0.0, 1e-9);
  CHECK_NEAR(h[1].beat, 0.33, 1e-9);
}

TEST("taste: after breeding from patterns with a kick on step 6, fresh litters have more of them") {
  int before = 0, after = 0;
  for (std::uint32_t seed = 1; seed <= 100; ++seed) {
    Breeder b(seed);
    b.fresh();
    for (int i = 0; i < Breeder::kLitter; ++i) before += b.litter().candidate(i).on(0, 6);
    for (int r = 0; r < 5; ++r) {
      b.cycleStep(0, 6);
      if (!b.heard().on(0, 6)) b.cycleStep(0, 6);
      if (!b.heard().on(0, 6)) b.cycleStep(0, 6);
      b.breedFrom(b.litter().selected());
    }
    b.fresh();
    for (int i = 0; i < Breeder::kLitter; ++i) after += b.litter().candidate(i).on(0, 6);
  }
  CHECK(after > before + 100);
}

TEST("the player: one bar of a four-on-the-floor kick gives four notes, each with its off") {
  Pattern p;
  for (int s : { 0, 4, 8, 12 }) { p.vel[0][(size_t) s] = 0.8f; p.roll[0][(size_t) s] = 1; }
  Player pl;
  pl.prepare(48000);
  std::vector<NoteEvent> ev;
  // 120 bpm: a bar is 96000 samples.
  for (int block = 0; block < 190; ++block) pl.advance(p, 500, 120.0, -1.0, true, ev);
  int ons = 0, offs = 0;
  for (auto& e : ev) (e.on ? ons : offs) += 1;
  CHECK_EQ(ons, 4);
  CHECK_EQ(offs, 4);
  CHECK_EQ(ev[0].offset, 0);
}

TEST("the player follows the host: the same notes wherever the host starts the block") {
  Pattern p;
  p.vel[2][3] = 0.5f; p.roll[2][3] = 1;
  Player pl;
  pl.prepare(48000);
  std::vector<NoteEvent> ev;
  pl.advance(p, 4800, 120.0, 4.7, false, ev); // beat 0.7 of bar 2: step 3 is at beat 0.75
  CHECK(!ev.empty());
  CHECK_EQ(ev[0].offset, 1200);
}

TEST("saving and loading gives back the same litter, pulls, roles and taste") {
  Breeder a(11);
  a.setStyle(Style::Trap);
  a.setPull(0.3);
  a.setRole(3, Role::Off);
  a.setLock(1, true);
  a.breedFrom(3);
  a.cycleStep(2, 5);
  Breeder b(99);
  CHECK(b.load(a.save()));
  CHECK(b.save() == a.save());
  CHECK(!b.load("bloodline-synth 1\n"));
}

CHECK_MAIN
