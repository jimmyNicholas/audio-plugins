// PROTOTYPE (throwaway spike). Just enough checks to trust the seam between the panels: loops pick
// kept pads by label, follow swaps, breed visibly, place hits anywhere, flam, and everything saves.
#include "../../shared/test/check.h"
#include "studio.h"
#include <algorithm>
#include <cmath>

using namespace bb2;

// Every candidate but the parent, which stays as it was.
static bool usesOnly(const Studio& s, int lane, std::uint8_t allowed) {
  for (int i = 0; i < Loops::kLitter; ++i) {
    if (i == s.loops.litter().parent()) continue;
    const LaneData& l = s.loops.litter().candidate(i).lanes[(size_t) lane];
    if (l.first < 0 || !((allowed >> l.first) & 1)) return false;
    if (l.second >= 0 && !((allowed >> l.second) & 1)) return false;
  }
  return true;
}

// A change you can see: a hit added or removed, or one moved by more than half a sixteenth.
static bool visiblyDiffers(const LaneData& a, const LaneData& b) {
  if (a.count != b.count) return true;
  for (int i = 0; i < a.count; ++i) {
    double best = 99;
    for (int j = 0; j < b.count; ++j) {
      const double d = std::fabs(a.hits[(size_t) i].pos - b.hits[(size_t) j].pos);
      best = std::min(best, std::min(d, 16.0 - d));
    }
    if (best > 0.5) return true;
  }
  return false;
}

static void keepAs(Studio& s, Zone z, int tile, int pad) {
  s.drums.setZone(z);
  s.keepDrum(tile, pad);
}

TEST("lanes draw from the pads labelled for them, and nothing when nothing fits") {
  Studio s(5);
  keepAs(s, Zone::Kick, 0, 2);
  keepAs(s, Zone::Kick, 1, 6);
  keepAs(s, Zone::Snare, 0, 4);
  int borrowed = 0, silent = 0;
  for (int k = 0; k < 40; ++k) {
    s.loops.fresh();
    for (int i = 0; i < Loops::kLitter; ++i) {
      const auto& p = s.loops.litter().candidate(i);
      const int kick = p.lanes[KickLane].first;
      CHECK(kick == 2 || kick == 6 || kick == 4); // its own, or rarely another lane's
      borrowed += kick == 4;
      silent += p.lanes[HatLane].first < 0;
    }
  }
  CHECK(borrowed > 0 && borrowed < 60);
  CHECK(silent > 100); // no hats or Drum pads kept: mostly silent, sometimes borrowing
}

TEST("two kicks: the breeder uses both, across a few litters") {
  Studio s(9);
  keepAs(s, Zone::Kick, 0, 0);
  keepAs(s, Zone::Kick, 0, 1);
  bool a = false, b = false;
  for (int k = 0; k < 20; ++k) {
    s.loops.fresh();
    for (int i = 0; i < Loops::kLitter; ++i) {
      const int f = s.loops.litter().candidate(i).lanes[KickLane].first;
      a |= f == 0;
      b |= f == 1;
    }
  }
  CHECK(a && b);
}

TEST("breeding picks up a pad kept since; swapping pads moves the loops' sounds with them") {
  Studio s(11);
  s.loops.keep(0, 3);
  keepAs(s, Zone::Hats, 2, 5);
  s.loops.breedFrom(1);
  CHECK(usesOnly(s, HatLane, 1 << 5));
  s.swapPads(5, 0);
  CHECK(usesOnly(s, HatLane, 1 << 0));
  CHECK_EQ((int) s.loops.slot(3).loop.lanes[HatLane].first, -1);
}

TEST("cycling a lane's sound runs through its pads, then none") {
  Studio s(2);
  keepAs(s, Zone::Snare, 0, 1);
  keepAs(s, Zone::Snare, 0, 3);
  s.loops.select(0);
  for (int k = 0; k < 4 && s.loops.playing().lanes[SnareLane].first != -1; ++k) s.loops.cycleSound(SnareLane);
  CHECK_EQ((int) s.loops.playing().lanes[SnareLane].first, -1);
  s.loops.cycleSound(SnareLane);
  CHECK_EQ((int) s.loops.playing().lanes[SnareLane].first, 1);
  s.loops.cycleSound(SnareLane);
  CHECK_EQ((int) s.loops.playing().lanes[SnareLane].first, 3);
  s.loops.cycleSound(SnareLane);
  CHECK_EQ((int) s.loops.playing().lanes[SnareLane].first, -1);
}

TEST("kept loops play, take edits, and survive swapping") {
  Studio s(4);
  s.loops.keep(2, 0);
  s.loops.playSlot(0);
  CHECK(s.loops.playing() == s.loops.slot(0).loop);
  const int before = s.loops.slot(0).loop.hits();
  const int at = s.loops.addHit(KickLane, 7.3, 0.9f);
  if (at >= 0) CHECK_EQ(s.loops.slot(0).loop.hits(), before + 1);
  s.loops.swapSlots(0, 7);
  CHECK_EQ(s.loops.playingSlot(), 7);
  s.loops.select(1);
  CHECK_EQ(s.loops.playingSlot(), -1);
}

TEST("the player: a loop hits on time, once a bar, and only while Playing is on") {
  Studio s(6);
  keepAs(s, Zone::Kick, 0, 0);
  Pattern p;
  p.lanes[KickLane].first = 0;
  p.lanes[KickLane].add(0, 0.9f);
  Player pl;
  pl.prepare(48000);
  std::vector<Player::Event> ev;
  for (int i = 0; i < 192; ++i) pl.advance(p, s.loops.ops(), 500, 120.0, -1.0, true, ev); // two bars at 120
  CHECK_EQ((int) ev.size(), 2);
  ev.clear();
  for (int i = 0; i < 192; ++i) pl.advance(p, s.loops.ops(), 500, 120.0, i * 500 / 24000.0, false, ev); // Live plays, Playing off
  CHECK_EQ((int) ev.size(), 0);
}

// 1. None zone and None style.
TEST("None zone keeps Drum pads; Perc zone keeps Perc pads inside its ranges") {
  Studio s(21);
  keepAs(s, Zone::None, 0, 0);
  CHECK(s.drums.pad(0).name == "Drum 1");
  CHECK(s.drums.pad(0).zone == Zone::None);
  keepAs(s, Zone::Perc, 0, 1);
  CHECK(s.drums.pad(1).name == "Perc 2");
  s.drums.fresh();
  // With zone share 0.8, most Perc pitches land in 200 to 900 Hz.
  int inside = 0;
  for (int k = 0; k < 50; ++k) {
    s.drums.fresh();
    for (int i = 0; i < Drums::kLitter; ++i) {
      const double hz = bloodline::toReal(bloodline::Pitch, s.drums.litter().candidate(i)[bloodline::Pitch]);
      inside += hz >= 200 && hz <= 900;
    }
  }
  CHECK(inside > 140);
}

TEST("a None style litter has no lean: backbeat snares are no likelier than other steps") {
  Studio s(23);
  s.loops.setSwingDial(0.62);
  s.loops.setStyle(Style::None);
  CHECK(std::fabs(s.loops.ops().swingDial - 0.62) < 1e-9); // choosing None leaves the swing dial
  int backbeat = 0, other = 0;
  for (int k = 0; k < 400; ++k) {
    s.loops.fresh();
    for (int i = 0; i < Loops::kLitter; ++i) {
      const auto& l = s.loops.litter().candidate(i).lanes[SnareLane];
      for (int h = 0; h < l.count; ++h) {
        const double pos = l.hits[(size_t) h].pos;
        CHECK(std::fabs(pos - std::round(pos)) < 1e-9); // no scatter
        const int step = (int) std::lround(pos) % 16;
        (step == 4 || step == 12 ? backbeat : other) += 1;
      }
    }
  }
  const double perBackbeat = backbeat / 2.0, perOther = other / 14.0;
  CHECK(perBackbeat < perOther * 1.3);
  CHECK(perBackbeat > perOther * 0.7);
}

// 2. Spread.
TEST("spread: setSpread holds within limits, and New resets both panels") {
  Studio s(25);
  s.drums.setSpread(9.0);
  CHECK(std::fabs(s.drums.litter().spread() - 0.5) < 1e-9);
  s.loops.setSpread(-1.0);
  CHECK(std::fabs(s.loops.litter().spread() - 0.02) < 1e-9);
  s.drums.setSpread(0.41);
  s.loops.setSpread(0.37);
  s.drums.fresh();
  s.loops.fresh();
  CHECK(std::fabs(s.drums.litter().spread() - 0.25) < 1e-9);
  CHECK(std::fabs(s.loops.litter().spread() - 0.2) < 1e-9);
}

// 3. Free hits.
TEST("free hits: the swing warp, wrapping at the bar's end, and duplicates merging") {
  // 5.3 is 1.3 into the eighth at 4: past the off sixteenth, which swing 0.66 put at 1.32.
  const double sw = 0.66, landed = 4.0 + 2 * sw + (2 - 2 * sw) * 0.3;
  CHECK(std::fabs(warp(5.3, sw) - landed) < 1e-9);
  CHECK(std::fabs(unwarp(warp(5.3, sw), sw) - 5.3) < 1e-9);
  CHECK(std::fabs(warp(5.0, sw) - (4.0 + 2 * sw)) < 1e-9); // on the grid: classic swing
  CHECK(std::fabs(warp(0.4, sw) - 2 * sw * 0.4) < 1e-9);
  CHECK(std::fabs(wrapBar(16.25) - 0.25) < 1e-9);
  CHECK(std::fabs(wrapBar(-0.5) - 15.5) < 1e-9);
  LaneData l;
  l.add(3.0, 0.5f);
  l.add(3.1, 0.9f);  // within 1/64 of a bar: one hit, the louder
  l.add(15.95, 0.4f);
  l.add(0.05, 0.6f); // across the bar's end
  l.add(8.0, 0.7f);
  l.tidy();
  CHECK_EQ(l.count, 3);
  CHECK(std::fabs(l.hits[1].pos - 3.1) < 1e-9);
  // Breeding keeps every hit inside the bar.
  Studio s(27);
  for (int k = 0; k < 30; ++k) s.loops.breedFrom(k % 4);
  for (int i = 0; i < Loops::kLitter; ++i)
    for (const auto& lane : s.loops.litter().candidate(i).lanes)
      for (int h = 0; h < lane.count; ++h) CHECK(lane.hits[(size_t) h].pos >= 0.0 && lane.hits[(size_t) h].pos < 16.0);
}

// 4. Flam.
TEST("flam: the second hit comes flam ms later at its level; one pad flams with itself") {
  Studio s(29);
  keepAs(s, Zone::Snare, 0, 3);
  Pattern p;
  LaneData& l = p.lanes[SnareLane];
  l.first = 3;
  l.second = 3;
  l.layer = Flam;
  l.flam = 20.0f;
  l.flamLevel = 0.5f;
  l.add(4.0, 0.8f); // beat 1 (0-based), at 120 BPM 0.5 s in
  Player pl;
  pl.prepare(48000);
  std::vector<Player::Event> ev;
  int at = 0;
  std::vector<int> when;
  for (int i = 0; i < 100; ++i) {
    ev.clear();
    pl.advance(p, s.loops.ops(), 512, 120.0, -1.0, true, ev);
    for (const auto& e : ev) { when.push_back(at + e.offset); CHECK_EQ(e.pad, 3); }
    if (ev.size() == 2) CHECK(std::fabs(ev[1].velocity - 0.4f) < 1e-6);
    at += 512;
  }
  CHECK_EQ((int) when.size(), 2);
  if (when.size() == 2) {
    CHECK(std::abs(when[0] - 24000) <= 1);
    CHECK(std::abs(when[1] - when[0] - 960) <= 1); // 20 ms at 48 kHz
  }
  // Stack: both at once.
  l.second = 3;
  l.layer = Stack;
  Hit hits[8];
  const int n = hitsOf(p, s.loops.ops(), hits, 8);
  CHECK_EQ(n, 2);
  CHECK(hits[0].beat == hits[1].beat && hits[1].delayMs == 0.0);
}

TEST("flam drawn by the breeder stays in range") {
  Studio s(31);
  keepAs(s, Zone::Snare, 0, 0);
  keepAs(s, Zone::Snare, 1, 1);
  int flams = 0;
  for (int k = 0; k < 60; ++k) {
    s.loops.fresh();
    for (int i = 0; i < Loops::kLitter; ++i) {
      const auto& l = s.loops.litter().candidate(i).lanes[SnareLane];
      CHECK(l.flam >= 8.0f && l.flam <= 40.0f && l.flamLevel >= 0.4f && l.flamLevel <= 0.8f);
      flams += l.layer == Flam && l.second >= 0;
    }
  }
  CHECK(flams > 20);
}

// 5. No built-ins.
TEST("no built-ins: with nothing kept every lane chooses nothing and plays nothing") {
  Studio s(3);
  for (int i = 0; i < Loops::kLitter; ++i)
    for (const auto& l : s.loops.litter().candidate(i).lanes) CHECK_EQ((int) l.first, -1);
  Hit hits[kMaxHits];
  const int n = hitsOf(s.loops.litter().candidate(0), s.loops.ops(), hits, kMaxHits);
  for (int i = 0; i < n; ++i) CHECK_EQ(hits[i].pad, -1);
}

TEST("no built-ins: with only a Drum pad, every lane may use it") {
  Studio s(33);
  keepAs(s, Zone::None, 0, 5);
  s.loops.fresh();
  for (int l = 0; l < kLanes; ++l) CHECK(usesOnly(s, l, 1 << 5));
}

// 6. Keep.
TEST("keeping leaves the litter as it is; one tile kept twice gives two pads") {
  Studio s(35);
  s.drums.setZone(Zone::Hats);
  const auto before = s.drums.litter().state();
  s.keepDrum(2, 0);
  s.keepDrum(2, 1);
  const auto after = s.drums.litter().state();
  CHECK(before.candidates == after.candidates);
  CHECK_EQ(before.generation, after.generation);
  CHECK(s.drums.pad(0).sound == s.drums.pad(1).sound);
  CHECK(s.drums.pad(0).name == "Hats 1");
  CHECK(s.drums.pad(1).name == "Hats 2");
  const auto loopsBefore = s.loops.litter().state().candidates;
  s.loops.keep(1, 0);
  CHECK(s.loops.litter().state().candidates == loopsBefore);
}

// 7. Locks, and breeding that visibly changes unlocked lanes.
TEST("the lock scenario: lock, breed, keep, unlock, Again gives every child a new kick") {
  for (std::uint32_t seed = 1; seed <= 30; ++seed) {
    Studio s(seed);
    keepAs(s, Zone::Kick, 0, 0);
    s.loops.fresh();
    s.loops.setLock(KickLane, true);
    s.loops.breedFrom(0);
    s.loops.keep(0, 0);
    s.loops.setLock(KickLane, false);
    for (int again = 0; again < 3; ++again) {
      s.loops.breedFrom(0); // Again: narrows the spread each time
      const auto& parent = s.loops.litter().candidate(0).lanes[KickLane];
      for (int i = 1; i < Loops::kLitter; ++i) CHECK(visiblyDiffers(parent, s.loops.litter().candidate(i).lanes[KickLane]));
    }
    CHECK(s.loops.litter().spread() < 0.05);
  }
}

TEST("a locked lane stays identical through breed, cross and Again") {
  Studio s(37);
  keepAs(s, Zone::Snare, 0, 0);
  s.loops.fresh();
  s.loops.setLock(SnareLane, true);
  const LaneData held = s.loops.litter().candidate(1).lanes[SnareLane];
  s.loops.breedFrom(1);
  s.loops.breedFrom(1);
  s.loops.breedFrom(2);
  s.loops.cross(2, 3);
  s.loops.breedFrom(2);
  for (int i = 0; i < Loops::kLitter; ++i) CHECK(s.loops.litter().candidate(i).lanes[SnareLane] == held);
}

// 8. Saving.
TEST("saving and loading version 2 gives back everything; version 1 is rejected") {
  Studio a(13);
  keepAs(a, Zone::Kick, 1, 2);
  keepAs(a, Zone::Perc, 3, 5);
  a.drums.setSpread(0.33);
  a.loops.setStyle(Style::None);
  a.loops.setLock(HatLane, true);
  a.loops.breedFrom(2);
  a.loops.keep(1, 4);
  a.loops.playSlot(4);
  a.loops.addHit(SnareLane, 6.71, 0.4f);
  a.loops.setSound(SnareLane, 0, 5);
  a.loops.setSound(SnareLane, 1, 5);
  a.loops.cycleLayer(SnareLane); // Stack to Flam
  a.loops.setFlam(SnareLane, 31.5);
  Studio b(1);
  CHECK(b.load(a.save()));
  CHECK(b.save() == a.save());
  CHECK(b.drums.pad(5).name == "Perc 2");
  CHECK(b.drums.pad(5).zone == Zone::Perc);
  CHECK_EQ(b.loops.playingSlot(), 4);
  CHECK(b.loops.playing().lanes[SnareLane].layer == Flam);
  CHECK(std::fabs(b.loops.playing().lanes[SnareLane].flam - 31.5f) < 1e-6);
  CHECK(b.loops.ops().style == Style::None);
  std::string v1 = a.save();
  v1.replace(0, std::string("bloodline-breaks-2 2").size(), "bloodline-breaks-2 1");
  CHECK(!b.load(v1));
  CHECK(!b.load("bloodline-breaks 1\n"));
}

CHECK_MAIN
