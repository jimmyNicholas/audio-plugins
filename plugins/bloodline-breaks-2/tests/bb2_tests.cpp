// PROTOTYPE (throwaway spike). Just enough checks to trust the seam between the panels: loops pick
// kept pads by label, follow swaps, and everything saves.
#include "../../shared/test/check.h"
#include "studio.h"

using namespace bb2;

// Every candidate but the parent, which stays as it was.
static bool usesOnly(const Studio& s, int lane, std::uint8_t allowed) {
  for (int i = 0; i < Loops::kLitter; ++i) {
    if (i == s.loops.litter().parent()) continue;
    const std::uint8_t m = s.loops.litter().candidate(i).sound[(size_t) lane];
    if (m == 0 || (m & ~allowed)) return false;
  }
  return true;
}

TEST("with nothing kept, every lane plays its built-in sound") {
  Studio s(3);
  for (int i = 0; i < Loops::kLitter; ++i)
    for (int l = 0; l < kLanes; ++l) CHECK_EQ((int) s.loops.litter().candidate(i).sound[(size_t) l], 0);
}

TEST("lanes draw from the pads labelled for them; perc borrows any pad when none is Other") {
  Studio s(5);
  s.drums.setZone(Zone::Kick);
  s.keepDrum(0, 2);
  s.keepDrum(1, 6);
  s.drums.setZone(Zone::Snare);
  s.keepDrum(0, 4);
  s.loops.fresh();
  CHECK(usesOnly(s, KickLane, (1 << 2) | (1 << 6)));
  CHECK(usesOnly(s, SnareLane, 1 << 4));
  CHECK_EQ((int) s.loops.litter().candidate(0).sound[HatLane], 0); // no hats kept: built-in
  CHECK(usesOnly(s, PercLane, (1 << 2) | (1 << 6) | (1 << 4)));
}

TEST("two kicks: the breeder uses both, across a few litters") {
  Studio s(9);
  s.drums.setZone(Zone::Kick);
  s.keepDrum(0, 0);
  s.keepDrum(0, 1);
  bool a = false, b = false;
  for (int k = 0; k < 20; ++k) {
    s.loops.fresh();
    for (int i = 0; i < Loops::kLitter; ++i) {
      const auto m = s.loops.litter().candidate(i).sound[KickLane];
      a |= (m & 1) != 0;
      b |= (m & 2) != 0;
    }
  }
  CHECK(a && b);
}

TEST("breeding picks up a pad kept since; swapping pads moves the loops' sounds with them") {
  Studio s(11);
  s.loops.keep(0, 3);
  s.drums.setZone(Zone::Hats);
  s.keepDrum(2, 5);
  s.loops.breedFrom(1);
  CHECK(usesOnly(s, HatLane, 1 << 5));
  s.swapPads(5, 0);
  CHECK(usesOnly(s, HatLane, 1 << 0));
  CHECK_EQ((int) s.loops.slot(3).loop.sound[HatLane], 0);
}

TEST("cycling a lane's sound runs through its pads, then built-in") {
  Studio s(2);
  s.drums.setZone(Zone::Snare);
  s.keepDrum(0, 1);
  s.keepDrum(0, 3);
  s.loops.fresh();
  s.loops.select(0);
  for (int k = 0; k < 4 && s.loops.playing().sound[SnareLane] != 0; ++k) s.loops.cycleSound(SnareLane);
  CHECK_EQ((int) s.loops.playing().sound[SnareLane], 0);
  s.loops.cycleSound(SnareLane);
  CHECK_EQ((int) s.loops.playing().sound[SnareLane], 1 << 1);
  s.loops.cycleSound(SnareLane);
  CHECK_EQ((int) s.loops.playing().sound[SnareLane], 1 << 3);
  s.loops.cycleSound(SnareLane);
  CHECK_EQ((int) s.loops.playing().sound[SnareLane], 0);
}

TEST("kept loops play, take edits, and survive swapping") {
  Studio s(4);
  s.loops.keep(2, 0);
  s.loops.playSlot(0);
  CHECK(s.loops.playing() == s.loops.slot(0).loop);
  const int before = s.loops.slot(0).loop.hits();
  const bool was = s.loops.slot(0).loop.on(KickLane, 0);
  s.loops.cycleStep(KickLane, 0);
  CHECK(s.loops.slot(0).loop.hits() != before || was);
  s.loops.swapSlots(0, 7);
  CHECK_EQ(s.loops.playingSlot(), 7);
  s.loops.select(1);
  CHECK_EQ(s.loops.playingSlot(), -1);
}

TEST("saving and loading gives back both panels") {
  Studio a(13);
  a.drums.setZone(Zone::Kick);
  a.keepDrum(1, 2);
  a.drums.setZone(Zone::Snare);
  a.keepDrum(3, 5);
  a.loops.setStyle(Style::NeoSoul);
  a.loops.breedFrom(2);
  a.loops.keep(1, 4);
  a.loops.playSlot(4);
  Studio b(1);
  CHECK(b.load(a.save()));
  CHECK(b.save() == a.save());
  CHECK(b.drums.pad(5).name == "Snare 2");
  CHECK(b.drums.pad(5).zone == Zone::Snare);
  CHECK_EQ(b.loops.playingSlot(), 4);
  CHECK(!b.load("bloodline-breaks 1\n"));
}

TEST("the player: a loop hits on time, once a bar") {
  Studio s(6);
  Pattern p;
  p.vel[KickLane][0] = 0.9f;
  p.roll[KickLane][0] = 1;
  Player pl;
  pl.prepare(48000);
  std::vector<Player::Event> ev;
  for (int i = 0; i < 192; ++i) pl.advance(p, 500, 120.0, -1.0, true, ev); // two bars at 120
  CHECK_EQ((int) ev.size(), 2);
}

CHECK_MAIN
