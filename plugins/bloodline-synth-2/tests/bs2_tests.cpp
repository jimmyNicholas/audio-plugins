// Bloodline Synth 2's core: traits, ranges, time locks, mutation, sound choice, bars, keeping and
// saving. No JUCE.
#include "../../shared/test/check.h"
#include "studio.h"
#include <algorithm>
#include <cmath>
#include <set>

using namespace bs2;

namespace {

std::array<double, kNumTraits> mid() {
  std::array<double, kNumTraits> t{};
  t.fill(0.5);
  return t;
}

Loop drawn(const LoopOps& ops, std::array<double, kNumTraits> t, std::uint32_t seed) {
  Loop l;
  l.bars = ops.bars;
  l.traits = t;
  l.seed = seed;
  l.notes = ops.draw(t, ops.bars, seed);
  ops.tidy(l, false);
  return l;
}

double meanLength(const Loop& l) {
  double s = 0;
  for (const Note& n : l.notes) s += n.length;
  return l.notes.empty() ? 0 : s / (double) l.notes.size();
}

// The mean distance in semitones between neighbouring notes, chord notes included.
double meanInterval(const Loop& l) {
  double s = 0;
  for (size_t i = 1; i < l.notes.size(); ++i) s += std::abs(l.notes[i].pitch - l.notes[i - 1].pitch);
  return l.notes.size() < 2 ? 0 : s / (double) (l.notes.size() - 1);
}

bool inRange(const Loop& l, const LoopOps& ops) {
  for (const Note& n : l.notes) if (n.pitch < ops.lowNote() || n.pitch > ops.highNote()) return false;
  return true;
}

// The notes that start in [a, b).
std::vector<Note> startingIn(const Loop& l, double a, double b) {
  std::vector<Note> out;
  for (const Note& n : l.notes) if (n.start >= a && n.start < b) out.push_back(n);
  return out;
}

std::array<int, kKitPads> padsOf(std::initializer_list<int> zones) {
  std::array<int, kKitPads> p{ -1, -1, -1, -1, -1, -1, -1, -1 };
  int i = 0;
  for (int z : zones) p[(size_t) i++] = z;
  return p;
}

} // namespace

// 1. Traits steer.

TEST("dense loops have more notes than sparse ones") {
  LoopOps ops;
  double sparse = 0, dense = 0;
  for (std::uint32_t s = 1; s <= 60; ++s) {
    auto t = mid();
    t[Dense] = 0.0; sparse += (double) drawn(ops, t, s).notes.size();
    t[Dense] = 1.0; dense += (double) drawn(ops, t, s).notes.size();
  }
  CHECK(dense > sparse * 3);
}

TEST("mono loops never sound two notes at once") {
  LoopOps ops;
  breed::Rng rng(5);
  for (std::uint32_t s = 1; s <= 80; ++s) {
    auto t = mid();
    t[Poly] = 0.0;
    t[Long] = rng.uniform(); // long notes would overlap if nothing stopped them
    t[Dense] = rng.uniform();
    CHECK_EQ(peakVoices(drawn(ops, t, s)), 1);
  }
  // Through breeding too.
  for (std::uint32_t s = 1; s <= 20; ++s) {
    auto t = mid();
    t[Poly] = 0.0;
    LoopOps locked = ops;
    locked.traitLocks[Poly] = true;
    Loop l = drawn(ops, t, s);
    for (int g = 0; g < 5; ++g) l = locked.mutate(l, 0.4, rng);
    CHECK_EQ(peakVoices(l), 1);
  }
}

TEST("poly loops reach chords, up to five notes") {
  LoopOps ops;
  int peak = 0;
  for (std::uint32_t s = 1; s <= 40; ++s) {
    auto t = mid();
    t[Poly] = 1.0;
    peak = std::max(peak, peakVoices(drawn(ops, t, s)));
    CHECK(peakVoices(drawn(ops, t, s)) <= 5);
  }
  CHECK(peak >= 3);
}

TEST("tonal loops use only the scale's notes") {
  for (int scale = 0; scale < kNumScales; ++scale) {
    LoopOps ops;
    ops.key = 3;
    ops.scale = (Scale) scale;
    for (std::uint32_t s = 1; s <= 25; ++s) {
      breed::Rng rng(s);
      auto t = mid();
      for (double& x : t) x = rng.uniform();
      t[Tonal] = 0.0;
      ops.bars = kBarChoices[s % 3];
      const Loop l = drawn(ops, t, s);
      for (const Note& n : l.notes) CHECK(inScale(n.pitch, ops.key, ops.scale));
    }
  }
}

TEST("atonal loops leave the scale") {
  LoopOps ops;
  int outside = 0;
  for (std::uint32_t s = 1; s <= 30; ++s) {
    auto t = mid();
    t[Tonal] = 1.0;
    for (const Note& n : drawn(ops, t, s).notes) outside += !inScale(n.pitch, ops.key, ops.scale);
  }
  CHECK(outside > 20);
}

TEST("long loops have longer notes than short ones") {
  LoopOps ops;
  double shortL = 0, longL = 0;
  for (std::uint32_t s = 1; s <= 60; ++s) {
    auto t = mid();
    t[Long] = 0.0; shortL += meanLength(drawn(ops, t, s));
    t[Long] = 1.0; longL += meanLength(drawn(ops, t, s));
  }
  CHECK(longL > shortL * 2);
}

TEST("open loops have wider intervals than clustered ones") {
  LoopOps ops;
  double clustered = 0, open = 0;
  for (std::uint32_t s = 1; s <= 60; ++s) {
    auto t = mid();
    t[Open] = 0.0; clustered += meanInterval(drawn(ops, t, s));
    t[Open] = 1.0; open += meanInterval(drawn(ops, t, s));
  }
  CHECK(open > clustered * 1.3);
}

TEST("dynamic loops spread velocity wider than even ones") {
  LoopOps ops;
  auto spreadOf = [](const Loop& l) {
    float lo = 1, hi = 0;
    for (const Note& n : l.notes) { lo = std::min(lo, n.velocity); hi = std::max(hi, n.velocity); }
    return l.notes.empty() ? 0.0 : (double) (hi - lo);
  };
  double even = 0, dyn = 0;
  for (std::uint32_t s = 1; s <= 40; ++s) {
    auto t = mid();
    t[Dynamic] = 0.0; even += spreadOf(drawn(ops, t, s));
    t[Dynamic] = 1.0; dyn += spreadOf(drawn(ops, t, s));
  }
  CHECK(dyn > even * 2);
}

TEST("off beat loops start more notes off the beat") {
  LoopOps ops;
  auto offShare = [](const Loop& l) {
    int off = 0;
    for (const Note& n : l.notes) off += std::fmod(n.start, 1.0) > 0.01;
    return l.notes.empty() ? 0.0 : off / (double) l.notes.size();
  };
  double on = 0, off = 0;
  for (std::uint32_t s = 1; s <= 60; ++s) {
    auto t = mid();
    t[OffBeat] = 0.0; on += offShare(drawn(ops, t, s));
    t[OffBeat] = 1.0; off += offShare(drawn(ops, t, s));
  }
  CHECK(off > on * 1.3);
}

// 2. Octave range.

TEST("no note falls outside the octave range, through fresh, breed, cross and redraws") {
  LoopOps ops;
  ops.octLow = 4;
  ops.octHigh = 5;
  breed::Rng rng(11);
  for (int k = 0; k < 30; ++k) {
    Loop a = ops.fresh(rng), b = ops.fresh(rng);
    CHECK(inRange(a, ops));
    Loop m = ops.mutate(a, 0.5, rng);
    CHECK(inRange(m, ops));
    CHECK(inRange(ops.cross(a, b, rng), ops));
    a.traits[Open] = 1.0;
    CHECK(inRange(ops.redraw(a), ops));
  }
  // Notes outside after a change of range are folded in on the next breed.
  Loops loops(3);
  loops.setOctaves(1, 2);
  loops.breedFrom(0);
  for (int i = 1; i < loops.litter().size(); ++i) CHECK(inRange(loops.litter().candidate(i), loops.ops())); // the parent stays put
  loops.setOctaves(6, 6); // at least an octave apart
  CHECK(loops.ops().octHigh - loops.ops().octLow >= 1);
}

// 3. Time locks.

TEST("time locks keep their notes through breed, cross and redrawing; nothing new starts there") {
  LoopOps ops;
  ops.bars = 4;
  ops.timeLocks = { { 4.0, 8.0 } }; // the second bar
  breed::Rng rng(21);
  for (int k = 0; k < 30; ++k) {
    auto t = mid();
    t[Dense] = 0.7;
    t[Long] = 1.0;
    t[Poly] = 0.0;
    const Loop a = drawn(ops, t, 100 + (std::uint32_t) k), b = ops.fresh(rng);
    const auto kept = startingIn(a, 4.0, 8.0);
    for (const Loop& child : { ops.mutate(a, 0.5, rng), ops.mutate(a, 0.02, rng), ops.cross(a, b, rng), ops.redraw(a) }) {
      CHECK(startingIn(child, 4.0, 8.0) == kept);
      // Notes from the unlocked parts that would run into the lock end at its edge.
      for (const Note& n : child.notes)
        if (n.start < 4.0) CHECK(n.start + n.length <= 4.0 + 1e-9);
    }
  }
}

TEST("locks are made, merged and removed on the strip") {
  Loops loops(4);
  loops.addTimeLock(1.0, 2.0);
  loops.addTimeLock(1.5, 3.0);
  CHECK_EQ((int) loops.ops().timeLocks.size(), 1);
  CHECK_NEAR(loops.ops().timeLocks[0].end, 3.0, 1e-9);
  loops.addTimeLock(6.0, 5.0);
  CHECK_EQ((int) loops.ops().timeLocks.size(), 2);
  CHECK(loops.removeTimeLockAt(5.5));
  CHECK(!loops.removeTimeLockAt(5.5));
  CHECK_EQ((int) loops.ops().timeLocks.size(), 1);
}

TEST("locks past the loop's end have no effect") {
  LoopOps ops;
  ops.bars = 2;
  ops.timeLocks = { { 10.0, 14.0 } };
  CHECK(!ops.lockedAt(1.0, 8.0));
  breed::Rng rng(2);
  const Loop a = ops.fresh(rng);
  CHECK(ops.mutate(a, 0.2, rng) != a);
}

// 4. Mutation is audible.

TEST("every child differs from its parent, at the lowest spread too") {
  LoopOps ops;
  breed::Rng rng(31);
  for (int bars : { 2, 4, 8 }) {
    ops.bars = bars;
    for (int k = 0; k < 40; ++k) {
      const Loop a = ops.fresh(rng);
      for (double spread : { 0.02, 0.1, 0.5 }) {
        const Loop child = ops.mutate(a, spread, rng);
        CHECK(child.notes != a.notes);
      }
    }
  }
  // And through the litter: Again at the smallest spread.
  Loops loops(8);
  loops.setSpread(0.02);
  loops.breedFrom(0);
  for (int g = 0; g < 6; ++g) loops.breedFrom(0);
  for (int i = 1; i < loops.litter().size(); ++i) CHECK(loops.litter().candidate(i).notes != loops.litter().candidate(0).notes);
}

TEST("trait locks hold through breeding") {
  LoopOps ops;
  ops.traitLocks[Dense] = ops.traitLocks[Tonal] = true;
  breed::Rng rng(41);
  const Loop a = ops.fresh(rng), b = ops.fresh(rng);
  for (int k = 0; k < 20; ++k) {
    const Loop m = ops.mutate(a, 0.5, rng);
    CHECK(m.traits[Dense] == a.traits[Dense] && m.traits[Tonal] == a.traits[Tonal]);
    const Loop c = ops.cross(a, b, rng);
    CHECK(c.traits[Dense] == a.traits[Dense]);
  }
}

TEST("a trait moved by hand redraws related notes from the loop's own seed") {
  LoopOps ops;
  breed::Rng rng(51);
  Loop a = ops.fresh(rng);
  CHECK(ops.redraw(a).notes == ops.redraw(a).notes); // the seed makes it repeatable
  Loop b = a;
  b.traits[Dense] = std::min(1.0, a.traits[Dense] + 0.4);
  CHECK(ops.redraw(b).notes != ops.redraw(a).notes);
}

// 5. Sound choice.

TEST("the breeder chooses a Bass pad for low ranges, and nothing when nothing is kept") {
  using synth::Zone;
  LoopOps ops;
  breed::Rng rng(61);
  CHECK_EQ(ops.fresh(rng).sound, -1);
  ops.pads = padsOf({ (int) Zone::Keys, (int) Zone::Pad, (int) Zone::Bass, (int) Zone::Lead });
  ops.octLow = 1;
  ops.octHigh = 3;
  for (int k = 0; k < 20; ++k) CHECK_EQ(ops.fresh(rng).sound, 2);
  // Mono and long: Lead.
  ops.octLow = 3;
  ops.octHigh = 5;
  Loop l = ops.fresh(rng);
  l.traits[Poly] = 0.1;
  l.traits[Long] = 0.9;
  CHECK_EQ(ops.chooseSound(l, rng), 3);
  // Poly and long: Pad or Keys.
  l.traits[Poly] = 0.9;
  const int s = ops.chooseSound(l, rng);
  CHECK(s == 0 || s == 1);
  // Then None-labelled sounds, then any.
  ops.pads = padsOf({ -1, (int) Zone::Bass, (int) Zone::None });
  CHECK_EQ(ops.chooseSound(l, rng), 2);
  ops.pads = padsOf({ -1, (int) Zone::Bass });
  CHECK_EQ(ops.chooseSound(l, rng), 1);
}

TEST("a cleared sound is chosen again on breeding") {
  Studio st(7);
  st.sounds.setZone(synth::Zone::Keys);
  st.keepSound(0, 0);
  st.keepSound(1, 1);
  CHECK(st.loops.litter().candidate(0).sound >= 0); // keeping the first sound gives silent loops one
  const int was = st.loops.litter().candidate(0).sound;
  st.clearPad(was);
  CHECK(!st.loops.ops().plays(st.loops.litter().candidate(0).sound));
  st.loops.breedFrom(0);
  for (int i = 1; i < Loops::kLitter; ++i) CHECK_EQ(st.loops.litter().candidate(i).sound, 1 - was);
}

TEST("swapping pads moves every loop's sound with it") {
  Studio st(9);
  st.keepSound(0, 0);
  st.loops.setSound(0);
  st.loops.keep(st.loops.litter().selected(), 3);
  st.swapPads(0, 5);
  CHECK_EQ(st.loops.playing().sound, 5);
  CHECK_EQ(st.loops.slot(3).loop.sound, 5);
}

// 6. Bars.

TEST("switching bars: 2 to 4 repeats, 4 to 2 cuts") {
  Loops loops(12);
  const Loop two = loops.litter().candidate(0);
  CHECK_EQ(two.bars, 2);
  loops.setBars(4);
  const Loop four = loops.litter().candidate(0);
  CHECK_EQ(four.bars, 4);
  CHECK_EQ(four.notes.size(), two.notes.size() * 2);
  CHECK(startingIn(four, 8.0, 16.0).size() == two.notes.size());
  for (size_t i = 0; i < two.notes.size(); ++i) CHECK_NEAR(four.notes[i + two.notes.size()].start, two.notes[i].start + 8.0, 1e-9);
  loops.setBars(2);
  CHECK(loops.litter().candidate(0).notes.size() == two.notes.size());
  loops.setBars(3); // not a choice
  CHECK_EQ(loops.ops().bars, 2);
  loops.setBars(8);
  loops.breedFrom(1);
  for (int i = 0; i < Loops::kLitter; ++i) CHECK_EQ(loops.litter().candidate(i).bars, 8);
}

TEST("the player loops on the loop's own length against the host's position") {
  Loop l;
  l.bars = 2;
  l.notes = { { 0.0, 0.5, 60, 0.9f }, { 4.0, 0.5, 62, 0.9f } };
  Player p;
  p.prepare(1000.0);
  std::vector<Player::Event> ev;
  // 120 BPM at 1000 Hz: 500 samples a beat. Host at beat 8 (bar 3) is bar 1 of a two bar loop.
  p.advance(l, 0, 100, 120.0, 8.0, true, ev);
  CHECK(!ev.empty() && ev[0].on && ev[0].pitch == 60 && ev[0].offset == 0);
  ev.clear();
  // Beat 12 is bar 2 of the loop: 62.
  Player q;
  q.prepare(1000.0);
  q.advance(l, 0, 100, 120.0, 12.0, true, ev);
  CHECK(!ev.empty() && ev[0].pitch == 62);
  // Four bars long: beat 8 is bar 3 of the loop, which is empty.
  l.bars = 4;
  ev.clear();
  Player r;
  r.prepare(1000.0);
  r.advance(l, 0, 100, 120.0, 8.0, true, ev);
  CHECK(ev.empty());
}

TEST("the player ends its notes and sends them to the engine they started on") {
  Loop l;
  l.bars = 2;
  l.notes = { { 0.0, 1.0, 60, 0.9f } };
  Player p;
  p.prepare(1000.0);
  std::vector<Player::Event> ev;
  p.advance(l, 3, 100, 120.0, -1.0, true, ev); // free-running from the top
  CHECK(ev.size() == 1 && ev[0].on && ev[0].tag == 3);
  ev.clear();
  p.advance(l, 4, 600, 120.0, -1.0, true, ev); // off at beat 1, sample 500, on the first engine
  CHECK(ev.size() == 1 && !ev[0].on && ev[0].tag == 3 && ev[0].offset == 400);
  ev.clear();
  p.advance(l, 4, 3400, 120.0, -1.0, true, ev); // round to the top of the loop again (beat 8)
  CHECK_EQ((int) ev.size(), 1);
  CHECK(!ev.empty() && ev[0].on && std::abs(ev[0].offset - 3300) <= 1);
  ev.clear();
  p.advance(l, 4, 10, 120.0, -1.0, false, ev); // Playing off ends it
  CHECK(ev.size() == 1 && !ev[0].on && ev[0].tag == 4);
  CHECK(!p.running());
}

// 7. Keeping and spreads.

TEST("keeping leaves both litters as they are") {
  Studio st(13);
  const Patch before = st.sounds.litter().candidate(2);
  const int gen = st.sounds.litter().generation();
  st.keepSound(2, 0);
  st.keepSound(2, 1); // the same tile twice
  CHECK(st.sounds.litter().candidate(2) == before);
  CHECK_EQ(st.sounds.litter().generation(), gen);
  CHECK(st.sounds.pad(0).sound == st.sounds.pad(1).sound);
  CHECK(st.sounds.pad(0).name != st.sounds.pad(1).name);
  CHECK(st.sounds.pad(0).name.rfind("Synth", 0) == 0); // None is labelled Synth
  const Loop loop = st.loops.litter().candidate(1);
  st.loops.keep(1, 4);
  CHECK(st.loops.litter().candidate(1) == loop);
  CHECK(st.loops.slot(4).full);
}

TEST("new resets both spreads; the spread can be set by hand within limits") {
  Studio st(14);
  st.setSoundSpread(0.45);
  st.setLoopSpread(0.4);
  CHECK_NEAR(st.sounds.litter().spread(), 0.45, 1e-9);
  st.setLoopSpread(5.0);
  CHECK_NEAR(st.loops.litter().spread(), 0.5, 1e-9);
  st.sounds.fresh();
  st.loops.fresh();
  CHECK_NEAR(st.sounds.litter().spread(), 0.25, 1e-9);
  CHECK_NEAR(st.loops.litter().spread(), 0.2, 1e-9);
}

TEST("edits by hand change the kept loop when a slot is playing") {
  Loops loops(15);
  loops.keep(0, 2);
  loops.playSlot(2);
  const size_t was = loops.slot(2).loop.notes.size();
  const int i = loops.addNote(1.0, 60);
  CHECK(i >= 0);
  CHECK_EQ(loops.slot(2).loop.notes.size(), was + 1);
  loops.removeNote(i);
  CHECK_EQ(loops.slot(2).loop.notes.size(), was);
  CHECK(loops.litter().candidate(0).notes.size() == was);
}

// 8. Saving.

TEST("saving and loading gives everything back; anything else is rejected") {
  Studio a(17);
  a.sounds.setZone(synth::Zone::Bass);
  a.sounds.setLock(3, true);
  a.keepSound(1, 2);
  a.sounds.breedFrom(1);
  a.loops.setBars(4);
  a.loops.setKey(5);
  a.loops.setScale(Scale::Dorian);
  a.loops.setOctaves(1, 4);
  a.loops.setTraitLock(Long, true);
  a.loops.addTimeLock(2.0, 3.5);
  a.loops.breedFrom(2);
  a.loops.keep(3, 6);
  a.loops.playSlot(6);
  const std::string text = a.save();

  Studio b(99);
  CHECK(b.load(text));
  CHECK(b.save() == text);
  CHECK(b.sounds.pad(2).name == a.sounds.pad(2).name);
  CHECK(b.loops.playingSlot() == 6);
  CHECK(b.loops.ops().timeLocks.size() == 1);
  CHECK(b.loops.litter().candidate(1) == a.loops.litter().candidate(1));

  const std::string good = b.save();
  CHECK(!b.load(""));
  CHECK(!b.load("bloodline-synth-2 2\n" + text.substr(text.find('\n') + 1)));
  CHECK(!b.load("bloodline-breaks-2 1\n" + text.substr(text.find('\n') + 1)));
  CHECK(!b.load(text.substr(0, text.size() / 2)));
  CHECK(b.save() == good); // a failed load changes nothing
}

CHECK_MAIN
