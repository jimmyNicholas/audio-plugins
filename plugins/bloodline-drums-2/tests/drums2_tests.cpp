// PROTOTYPE (throwaway spike). Just enough checks to trust keeping, moving and saving.
#include "../../shared/test/check.h"
#include "kit.h"

using namespace drums2;

TEST("keeping puts the candidate on the pad, names it, and starts a fresh litter") {
  Kit k(5);
  k.setZone(Zone::Snare);
  const Values v = k.litter().candidate(3);
  k.keep(3, 6);
  CHECK(k.pad(6).full);
  CHECK(k.pad(6).sound == v);
  CHECK(k.pad(6).name == "Snare 1");
  CHECK_EQ(k.litter().parent(), -1);
  CHECK(k.litter().candidate(3) != v);
  CHECK(k.back());
  CHECK(k.litter().candidate(3) == v); // Back returns the litter; the pad stays kept
  CHECK(k.pad(6).full);
}

TEST("swapping, clearing and breeding from a pad") {
  Kit k(8);
  k.keep(0, 0);
  const Values v = k.pad(0).sound;
  k.swapPads(0, 7);
  CHECK(!k.pad(0).full);
  CHECK(k.pad(7).sound == v);
  k.breedFromPad(7);
  CHECK(k.heard() == v);
  CHECK_EQ(k.litter().parent(), k.litter().selected());
  k.clearPad(7);
  CHECK(!k.pad(7).full);
}

TEST("saving and loading gives back the litter, pads, names, zone and locks") {
  Kit a(13);
  a.setZone(Zone::Kick);
  a.setLock(2, true);
  a.keep(1, 2);
  a.keep(4, 5);
  a.breedFrom(2);
  Kit b(1);
  CHECK(b.load(a.save()));
  CHECK(b.save() == a.save());
  CHECK(b.pad(5).name == "Kick 2");
  CHECK(!b.load("bloodline-breaks 1\n"));
}

TEST("the loop: four quarters in a bar") {
  Loop l;
  l.prepare(48000);
  l.setPattern(0);
  l.setOn(true);
  std::vector<Loop::Hit> hits;
  for (int i = 0; i < 190; ++i) l.advance(500, 120.0, hits);
  CHECK_EQ((int) hits.size(), 4);
}

CHECK_MAIN
