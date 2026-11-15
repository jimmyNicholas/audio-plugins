// The Chain seam: how chains are drawn, bred and crossed, and the saved state.
#include "../../shared/test/check.h"
#include "breeder.h"
#include <algorithm>

using namespace fx;

static int structuralChanges(const Chain& a, const Chain& b) {
  if (a.count != b.count) return 1;
  for (int i = 0; i < a.count; ++i) if (a.units[i].type != b.units[i].type) return 1;
  return 0;
}

TEST("fresh chains have two to five effects and a mix between 0.4 and 1") {
  ChainOps ops;
  breed::Rng rng(3);
  for (int i = 0; i < 500; ++i) {
    const Chain c = ops.fresh(rng);
    CHECK(c.count >= 2 && c.count <= 5);
    CHECK(c.mix >= 0.4 && c.mix <= 1.0);
    for (int k = 0; k < c.count; ++k)
      for (double v : c.units[k].p) CHECK(v >= 0.0 && v <= 1.0);
  }
}

TEST("a character draws most effects from its four") {
  ChainOps ops;
  ops.character = Character::Space;
  breed::Rng rng(4);
  int in = 0, n = 0;
  for (int i = 0; i < 2000; ++i, ++n) {
    const Type t = ops.drawType(rng);
    in += t == Type::Reverb || t == Type::Delay || t == Type::Width || t == Type::Comb;
  }
  CHECK_NEAR(in / double(n), 0.8 + 0.2 * 4 / 12.0, 0.03);
}

TEST("breeding changes settings every time and structure sometimes; Wild changes structure far more") {
  ChainOps calm, wild;
  wild.wild = true;
  breed::Rng a(5), b(5);
  int calmChanges = 0, wildChanges = 0, settingChanges = 0;
  for (int i = 0; i < 1000; ++i) {
    const Chain from = calm.fresh(a);
    const Chain c = calm.mutate(from, 0.2, a), w = wild.mutate(from, 0.2, b);
    calmChanges += structuralChanges(from, c);
    wildChanges += structuralChanges(from, w);
    settingChanges += c != from;
    CHECK(c.count >= 1 && c.count <= kMaxUnits);
    CHECK(w.count >= 1 && w.count <= kMaxUnits);
  }
  CHECK_EQ(settingChanges, 1000);
  CHECK(calmChanges > 100 && calmChanges < 700);
  CHECK(wildChanges > calmChanges + 150);
}

TEST("a narrow spread barely changes the structure") {
  ChainOps ops;
  breed::Rng rng(6);
  int changes = 0;
  for (int i = 0; i < 1000; ++i) {
    const Chain from = ops.fresh(rng);
    changes += structuralChanges(from, ops.mutate(from, 0.02, rng));
  }
  CHECK(changes < 120);
}

TEST("a locked effect is never changed or removed by breeding, even when Wild") {
  ChainOps ops;
  ops.wild = true;
  breed::Rng rng(7);
  Chain c = ops.fresh(rng);
  c.units[1].locked = true;
  const Unit kept = c.units[1];
  for (int i = 0; i < 300; ++i) {
    c = ops.mutate(c, 0.5, rng);
    CHECK(std::count(c.units.begin(), c.units.begin() + c.count, kept) == 1);
  }
}

TEST("crossing takes the front of one chain and the back of the other") {
  ChainOps ops;
  breed::Rng rng(8);
  for (int i = 0; i < 200; ++i) {
    const Chain a = ops.fresh(rng), b = ops.fresh(rng);
    const Chain c = ops.cross(a, b, rng);
    CHECK(c.count >= 1 && c.count <= kMaxUnits);
    CHECK(c.units[0] == a.units[0]); // the front always starts from the first parent
    CHECK_NEAR(c.mix, (a.mix + b.mix) / 2, 1e-12);
  }
}

TEST("by hand: settings, mix, locks and removing an effect change only the heard chain") {
  Breeder br(9);
  br.select(2);
  const Chain other = br.litter().candidate(3);
  br.setSetting(0, 1, 0.123);
  br.setMix(0.5);
  br.setLock(1, true);
  CHECK_EQ(br.heard().units[0].p[1], 0.123);
  CHECK_EQ(br.heard().mix, 0.5);
  CHECK(br.heard().units[1].locked);
  const int count = br.heard().count;
  br.remove(0);
  CHECK_EQ(br.heard().count, count - 1);
  CHECK(br.litter().candidate(3) == other);
  for (int i = 0; i < 10; ++i) br.remove(0);
  CHECK_EQ(br.heard().count, 1); // the last one stays
}

TEST("the breeder saves and loads exactly, and breeds on the same way") {
  Breeder a(10);
  a.setCharacter(Character::Glitch);
  a.setWild(true);
  a.breedFrom(1);
  a.cross(1, 4);
  a.select(3);
  a.setLock(0, true);
  Breeder b(11);
  CHECK(b.load(a.save()));
  CHECK(a.save() == b.save());
  CHECK(b.character() == Character::Glitch && b.wild());
  a.breedFrom(3);
  b.breedFrom(3);
  CHECK(a.save() == b.save());
  const std::string before = b.save();
  CHECK(!b.load("bloodline-effects 9\n"));
  CHECK(!b.load(before.substr(0, before.size() / 2)));
  CHECK(b.save() == before);
}

CHECK_MAIN
