// The Instrument seam: MIDI notes in, audio and state out.
#include "check.h"
#include "instrument.h"
#include <algorithm>
#include <vector>

using namespace bloodline;

constexpr double kRate = 48000.0;

static std::vector<float> play(Instrument& inst, double seconds) {
  std::vector<float> out(static_cast<size_t>(seconds * kRate), 0.0f);
  for (size_t i = 0; i < out.size(); i += 512) inst.render(out.data() + i, (int) std::min<size_t>(512, out.size() - i));
  return out;
}

static double peak(const std::vector<float>& x) {
  double p = 0;
  for (float s : x) p = std::max(p, (double) std::fabs(s));
  return p;
}

static Instrument ready(std::uint32_t seed = 5) {
  Instrument inst(seed);
  inst.prepare(kRate);
  return inst;
}

TEST("the pads: 36 at bottom left, counting left to right and then up") {
  CHECK(padFor(36).kind == Pad::Bottom && padFor(36).row == 2);
  CHECK(padFor(37).kind == Pad::Bottom && padFor(37).row == 1);
  CHECK(padFor(38).kind == Pad::Bottom && padFor(38).row == 0);
  CHECK(padFor(39).kind == Pad::Empty);
  CHECK(padFor(40).kind == Pad::Candidate && padFor(40).row == 2 && padFor(40).pad == 0);
  CHECK(padFor(42).kind == Pad::Candidate && padFor(42).row == 2 && padFor(42).pad == 2);
  CHECK(padFor(43).kind == Pad::Choose && padFor(43).row == 2);
  CHECK(padFor(45).kind == Pad::Candidate && padFor(45).row == 1 && padFor(45).pad == 1);
  CHECK(padFor(48).kind == Pad::Candidate && padFor(48).row == 0 && padFor(48).pad == 0);
  CHECK(padFor(51).kind == Pad::Choose && padFor(51).row == 0);
  CHECK(padFor(35).kind == Pad::None && padFor(52).kind == Pad::None);
  CHECK_EQ(candidateNote(0, 2), 50);
  CHECK_EQ(chooseNote(1), 47);
  CHECK_EQ(bottomNote(0), 38);
}

// What a lone Voice makes of these values: the Voice seam is the independent reference here.
static std::vector<float> voiceOf(const Values& v, float velocity, double seconds) {
  Voice voice;
  voice.prepare(kRate);
  voice.trigger(v, velocity);
  std::vector<float> out(static_cast<size_t>(seconds * kRate), 0.0f);
  for (size_t i = 0; i < out.size(); i += 512) voice.render(out.data() + i, (int) std::min<size_t>(512, out.size() - i));
  return out;
}

TEST("rows start on Hats, Snare and Kick, with nothing chosen") {
  Instrument inst = ready();
  CHECK(inst.row(0).zone() == Zone::Hats);
  CHECK(inst.row(1).zone() == Zone::Snare);
  CHECK(inst.row(2).zone() == Zone::Kick);
  for (int r = 0; r < kNumRows; ++r) CHECK(!inst.row(r).hasChosen());
}

TEST("pressing a candidate plays it and marks it in its own row only") {
  Instrument inst = ready();
  const Values b = inst.row(1).candidate(1);
  inst.noteOn(candidateNote(1, 1), 0.8f);
  CHECK_EQ(inst.row(1).lastPressed(), 1);
  CHECK_EQ(inst.row(0).lastPressed(), -1);
  CHECK_EQ(inst.row(2).lastPressed(), -1);
  CHECK(play(inst, 0.3) == voiceOf(b, 0.8f, 0.3));
}

TEST("choose commits the last pressed candidate and plays it once; with nothing pressed it is silent") {
  Instrument inst = ready();
  inst.noteOn(chooseNote(2), 1.0f);
  CHECK(!inst.row(2).hasChosen());
  CHECK_EQ(peak(play(inst, 0.2)), 0.0);

  const Values c = inst.row(2).candidate(2);
  inst.noteOn(candidateNote(2, 2), 1.0f);
  play(inst, 2.0); // let it ring out
  inst.noteOn(chooseNote(2), 0.9f);
  CHECK_EQ(inst.row(2).favourite(), 2);
  CHECK(play(inst, 0.3) == voiceOf(c, 0.9f, 0.3));
}

TEST("a bottom pad is silent until its row's first choice, then plays that row's chosen sound") {
  Instrument inst = ready();
  inst.noteOn(bottomNote(0), 1.0f);
  CHECK_EQ(peak(play(inst, 0.2)), 0.0);

  inst.noteOn(candidateNote(0, 0), 1.0f);
  inst.noteOn(chooseNote(0), 1.0f);
  play(inst, 2.0);
  inst.noteOn(bottomNote(0), 0.7f);
  CHECK(play(inst, 0.3) == voiceOf(inst.row(0).chosen(), 0.7f, 0.3));
}

TEST("pad 39 and notes off the grid make no sound and change nothing") {
  Instrument inst = ready();
  const std::string before = inst.save();
  for (int note : { 0, 35, 39, 52, 127 }) inst.noteOn(note, 1.0f);
  CHECK_EQ(peak(play(inst, 0.2)), 0.0);
  CHECK(inst.save() == before);
}

TEST("pads overlap: two pads at once sound together") {
  Instrument inst = ready();
  const Values k = inst.row(2).candidate(0), h = inst.row(0).candidate(0);
  inst.noteOn(candidateNote(2, 0), 1.0f);
  inst.noteOn(candidateNote(0, 0), 1.0f);
  auto both = play(inst, 0.2), a = voiceOf(k, 1.0f, 0.2), b = voiceOf(h, 1.0f, 0.2);
  double worst = 0;
  for (size_t i = 0; i < both.size(); ++i) worst = std::max(worst, (double) std::fabs(both[i] - (a[i] + b[i])));
  CHECK(worst < 1e-6);
}

// Breed a little in every row: zones changed, a lock, a pressed-but-not-chosen candidate.
static void breedSome(Instrument& inst) {
  inst.setZone(1, Zone::None);
  inst.setLock(2, Decay, true);
  for (int r = 0; r < kNumRows; ++r) {
    for (int pad : { 0, 1, 1, 2 }) { inst.noteOn(candidateNote(r, pad), 1.0f); inst.noteOn(chooseNote(r), 1.0f); }
  }
  inst.noteOn(candidateNote(0, 2), 1.0f);
}

static bool sameRow(const Row& a, const Row& b) {
  bool same = a.zone() == b.zone() && a.favourite() == b.favourite() && a.lastPressed() == b.lastPressed() &&
              a.step() == b.step() && a.newInARow() == b.newInARow() && a.seed() == b.seed();
  for (int pad = 0; pad < Row::kPads; ++pad) same = same && a.candidate(pad) == b.candidate(pad);
  for (int i = 0; i < kNumSettings; ++i) same = same && a.locked(i) == b.locked(i);
  return same;
}

TEST("saved state loads into a fresh instrument exactly, and breeding carries on the same way") {
  Instrument a = ready(5);
  breedSome(a);
  Instrument b = ready(999);
  CHECK(b.load(a.save()));
  for (int r = 0; r < kNumRows; ++r) CHECK(sameRow(a.row(r), b.row(r)));

  a.noteOn(chooseNote(0), 1.0f);
  b.noteOn(chooseNote(0), 1.0f);
  CHECK(sameRow(a.row(0), b.row(0)));
  CHECK(a.save() == b.save());
}

TEST("text that is not a saved state is refused and changes nothing") {
  Instrument inst = ready(5);
  breedSome(inst);
  const std::string before = inst.save();
  std::string truncated = before.substr(0, before.size() / 2);
  for (const std::string& bad : { std::string(""), std::string("hello"), truncated, std::string("bloodline 99\\n") })
    CHECK(!inst.load(bad));
  CHECK(inst.save() == before);
}

CHECK_MAIN
