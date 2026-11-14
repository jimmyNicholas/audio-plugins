// The Instrument seam: MIDI notes and on-screen actions in, audio and state out.
#include "check.h"
#include "instrument.h"
#include <algorithm>
#include <vector>

using namespace bloodline;

constexpr double kRate = 48000.0;

struct Stereo { std::vector<float> l, r; };

static Stereo play(Instrument& inst, double seconds) {
  Stereo out{ std::vector<float>(static_cast<size_t>(seconds * kRate), 0.0f), {} };
  out.r = out.l;
  for (size_t i = 0; i < out.l.size(); i += 512)
    inst.render(out.l.data() + i, out.r.data() + i, (int) std::min<size_t>(512, out.l.size() - i));
  return out;
}

static double peak(const Stereo& x) {
  double p = 0;
  for (float s : x.l) p = std::max(p, (double) std::fabs(s));
  for (float s : x.r) p = std::max(p, (double) std::fabs(s));
  return p;
}

static Instrument ready(std::uint32_t seed = 5) {
  Instrument inst(seed);
  inst.prepare(kRate);
  return inst;
}

// What a lone Voice makes of a row's pad: the Voice seam is the independent reference here.
static Stereo voiceOf(const Row& row, int pad, float velocity, double seconds) {
  Voice voice;
  voice.prepare(kRate);
  voice.trigger(row.candidate(pad), velocity, row.padPan(pad), row.padLevel(pad));
  Stereo out{ std::vector<float>(static_cast<size_t>(seconds * kRate), 0.0f), {} };
  out.r = out.l;
  for (size_t i = 0; i < out.l.size(); i += 512)
    voice.render(out.l.data() + i, out.r.data() + i, (int) std::min<size_t>(512, out.l.size() - i));
  return out;
}

// Lets every sounding pad ring out (Decay and Noise decay reach 4 s).
static void ringOut(Instrument& inst) { play(inst, 9.0); }

static bool same(const Stereo& a, const Stereo& b) { return a.l == b.l && a.r == b.r; }

// Press a pad and choose it, on the workbench.
static void pickOnBench(Instrument& inst, int pad) {
  inst.noteOn(noteFor(kBench, pad), 1.0f);
  inst.noteOn(noteFor(kBench, 3), 1.0f);
}

TEST("a new instrument: an open workbench with None, and three blank slots") {
  Instrument inst = ready();
  CHECK(inst.row(kBench) != nullptr);
  CHECK(inst.isOpen(kBench));
  CHECK(inst.row(kBench)->zone() == Zone::None);
  for (int line = 1; line <= kNumSlots; ++line) { CHECK(inst.row(line) == nullptr); CHECK(!inst.isOpen(line)); }
}

TEST("the pads: the workbench on 36 to 39, slots 1 to 3 above, blank slots show only Place") {
  Instrument inst = ready();
  CHECK_EQ(noteFor(kBench, 0), 36);
  CHECK_EQ(noteFor(3, 3), 51);
  CHECK(inst.padFor(36).kind == Pad::Candidate && inst.padFor(36).line == kBench && inst.padFor(36).pad == 0);
  CHECK(inst.padFor(39).kind == Pad::Choose && inst.padFor(39).line == kBench);
  CHECK(inst.padFor(40).kind == Pad::Blank && inst.padFor(40).line == 1);
  CHECK(inst.padFor(43).kind == Pad::Place && inst.padFor(43).line == 1);
  CHECK(inst.padFor(51).kind == Pad::Place && inst.padFor(51).line == 3);
  CHECK(inst.padFor(35).kind == Pad::None && inst.padFor(52).kind == Pad::None);
  inst.noteOn(43, 1.0f); // place into slot 1
  CHECK(inst.padFor(40).kind == Pad::Candidate);
  CHECK(inst.padFor(43).kind == Pad::Random);
  inst.setOpen(1, true);
  CHECK(inst.padFor(43).kind == Pad::Choose);
}

TEST("pressing a candidate plays it with its pan and level, and marks it in its own row only") {
  Instrument inst = ready();
  inst.noteOn(noteFor(kBench, 1), 0.8f);
  CHECK_EQ(inst.row(kBench)->lastPressed(), 1);
  CHECK(same(play(inst, 0.3), voiceOf(*inst.row(kBench), 1, 0.8f, 0.3)));
}

TEST("Place moves the workbench row into that slot, placed, and the workbench starts completely fresh") {
  Instrument inst = ready();
  inst.setZone(kBench, Zone::Snare);
  inst.setLock(kBench, Metal, true);
  pickOnBench(inst, 2);
  const Row::State bred = inst.row(kBench)->state();
  inst.noteOn(noteFor(2, 3), 1.0f); // Place on slot 2
  CHECK(inst.row(2) != nullptr);
  CHECK(!inst.isOpen(2));
  CHECK(inst.row(2)->candidate(0) == bred.pads[0]);
  CHECK_EQ(inst.row(2)->favourite(), 2);
  CHECK(inst.row(2)->zone() == Zone::Snare);

  const Row* bench = inst.row(kBench);
  CHECK(bench->zone() == Zone::None);
  CHECK_EQ(bench->favourite(), -1);
  CHECK(!bench->locked(Metal));
  CHECK_NEAR(bench->spread(), 0.25, 1e-12);
  CHECK(bench->candidate(0) != bred.pads[0]);
  CHECK(!inst.place(2)); // not blank now
}

TEST("a row can be placed before any choice, and the kit fills up") {
  Instrument inst = ready();
  for (int line = 1; line <= kNumSlots; ++line) CHECK(inst.place(line));
  for (int line = 1; line <= kNumSlots; ++line) CHECK(!inst.place(line));
  CHECK(inst.row(1)->candidate(0) != inst.row(2)->candidate(0));
  CHECK(inst.row(2)->candidate(0) != inst.row(3)->candidate(0));
  CHECK(inst.clear(2));
  CHECK(inst.row(2) == nullptr);
  CHECK(inst.place(2));
}

TEST("Random on a placed row plays one of its pads, never the same twice, and breeds nothing") {
  Instrument inst = ready();
  pickOnBench(inst, 0);
  inst.place(1);
  const Row::State before = inst.row(1)->state();
  int last = -1;
  for (int i = 0; i < 50; ++i) {
    inst.noteOn(noteFor(1, 3), 1.0f);
    const int played = inst.row(1)->lastPlayed();
    CHECK(played >= 0 && played != last);
    last = played;
  }
  for (int pad = 0; pad < 3; ++pad) CHECK(inst.row(1)->candidate(pad) == before.pads[pad]);
  CHECK_EQ(inst.row(1)->favourite(), before.favourite);

  Instrument fresh = ready();
  fresh.place(1);
  fresh.noteOn(noteFor(1, 3), 0.9f);
  const int pad = fresh.row(1)->lastPlayed();
  CHECK(same(play(fresh, 0.3), voiceOf(*fresh.row(1), pad, 0.9f, 0.3)));
}

TEST("a placed row is read-only until opened: zone, locks, values, pad Mono and reset are refused") {
  Instrument inst = ready();
  inst.place(1);
  inst.noteOn(noteFor(1, 0), 1.0f);
  const std::string before = inst.save();
  CHECK(!inst.setZone(1, Zone::Hats));
  CHECK(!inst.setLock(1, Pitch, true));
  CHECK(!inst.setValue(1, Pitch, 0.1));
  CHECK(!inst.setPadMono(1, 0, true));
  CHECK(!inst.reset(1));
  CHECK(inst.save() == before);
  CHECK(inst.setOpen(1, true));
  CHECK(inst.setZone(1, Zone::Hats));
  CHECK(inst.setValue(1, Pitch, 0.1));
  CHECK_EQ(inst.row(1)->candidate(0)[Pitch], 0.1);
}

TEST("an open slot breeds in place, and closing turns Choose back into Random") {
  Instrument inst = ready();
  inst.place(3);
  inst.setOpen(3, true);
  inst.noteOn(noteFor(3, 1), 1.0f);
  inst.noteOn(noteFor(3, 3), 1.0f);
  CHECK_EQ(inst.row(3)->favourite(), 1);
  inst.setOpen(3, false);
  inst.noteOn(noteFor(3, 1), 1.0f);
  inst.noteOn(noteFor(3, 3), 1.0f); // Random now
  CHECK_EQ(inst.row(3)->lastPressed(), 1);   // pressed, but not chosen
  CHECK(inst.row(3)->lastPlayed() != 1);
}

TEST("turning a setting by hand changes the last played pad of an open row") {
  Instrument inst = ready();
  CHECK(!inst.setValue(kBench, Decay, 0.3)); // nothing played yet
  inst.noteOn(noteFor(kBench, 2), 1.0f);
  CHECK(inst.setValue(kBench, Decay, 0.3));
  CHECK_EQ(inst.row(kBench)->candidate(2)[Decay], 0.3);
  inst.noteOn(noteFor(kBench, 3), 1.0f); // choose C: still the last played
  CHECK(inst.setValue(kBench, Decay, 0.4));
  CHECK_EQ(inst.row(kBench)->candidate(2)[Decay], 0.4);
}

TEST("moving swaps a slot with its neighbour, blank or not, and the sound follows the position") {
  Instrument inst = ready();
  pickOnBench(inst, 0);
  inst.place(1);
  const Values a = inst.row(1)->candidate(0);
  CHECK(inst.move(1, +1));
  CHECK(inst.row(1) == nullptr);
  CHECK(inst.row(2)->candidate(0) == a);
  CHECK(!inst.move(2, +2));
  CHECK(!inst.move(1, -1));      // a blank, and slot 1 cannot move down into the workbench
  inst.place(1);
  CHECK(!inst.move(1, -1));
  inst.setOpen(2, true);
  CHECK(inst.move(2, +1));
  CHECK(inst.isOpen(3));         // open moves with the row
  CHECK(!inst.move(3, +1));

  // A clip on slot 3's A now plays the row that moved there.
  ringOut(inst);
  inst.noteOn(noteFor(3, 0), 0.7f);
  CHECK(same(play(inst, 0.3), voiceOf(*inst.row(3), 0, 0.7f, 0.3)));
}

TEST("the row's Pan, Level and Mono work placed or open, and move with the row") {
  Instrument inst = ready();
  inst.noteOn(noteFor(kBench, 0), 1.0f);
  inst.setValue(kBench, Pan, 0.0); // pad A hard left
  inst.place(1);
  CHECK(inst.setRowLevel(1, -6));
  CHECK(inst.setRowPan(1, 0.5));
  CHECK_NEAR(inst.row(1)->padPan(0), -0.5, 1e-12);
  CHECK(inst.setRowMono(1, true));
  CHECK_EQ(inst.row(1)->padPan(0), 0.0);
  inst.move(1, +1);
  CHECK_EQ(inst.row(2)->rowLevel(), -6.0);
  CHECK(!inst.setRowPan(1, 0.0)); // blank now

  ringOut(inst);
  inst.noteOn(noteFor(2, 0), 1.0f);
  const Stereo x = play(inst, 0.2);
  CHECK(x.l == x.r); // Mono: centred
}

TEST("reset on an open row: fresh candidates, keeps zone and locks; Start again empties everything") {
  Instrument inst = ready();
  inst.setZone(kBench, Zone::Hats);
  inst.setLock(kBench, Pitch, true);
  pickOnBench(inst, 1);
  const Values before = inst.row(kBench)->candidate(0);
  CHECK(inst.reset(kBench));
  CHECK(inst.row(kBench)->candidate(0) != before);
  CHECK_EQ(inst.row(kBench)->favourite(), -1);
  CHECK(inst.row(kBench)->zone() == Zone::Hats);
  CHECK(inst.row(kBench)->locked(Pitch));

  inst.place(1);
  inst.place(3);
  const Values bench = inst.row(kBench)->candidate(0);
  inst.startAgain();
  for (int line = 1; line <= kNumSlots; ++line) CHECK(inst.row(line) == nullptr);
  CHECK(inst.row(kBench)->candidate(0) != bench);
  CHECK(inst.row(kBench)->zone() == Zone::None);
  CHECK(!inst.row(kBench)->locked(Pitch));
}

TEST("choose with nothing pressed is silent; notes off the grid and blank pads make no sound and change nothing") {
  Instrument inst = ready();
  inst.noteOn(noteFor(kBench, 3), 1.0f);
  CHECK(!inst.row(kBench)->hasChosen());
  const std::string before = inst.save();
  for (int note : { 0, 35, 40, 41, 42, 52, 127 }) inst.noteOn(note, 1.0f);
  CHECK_EQ(peak(play(inst, 0.2)), 0.0);
  CHECK(inst.save() == before);
}

TEST("pads overlap: two pads at once sound together") {
  Instrument inst = ready();
  inst.place(1);
  inst.noteOn(noteFor(1, 0), 1.0f);
  inst.noteOn(noteFor(kBench, 0), 1.0f);
  const Stereo both = play(inst, 0.2), a = voiceOf(*inst.row(1), 0, 1.0f, 0.2), b = voiceOf(*inst.row(kBench), 0, 1.0f, 0.2);
  double worst = 0;
  for (size_t i = 0; i < both.l.size(); ++i) {
    worst = std::max(worst, (double) std::fabs(both.l[i] - (a.l[i] + b.l[i])));
    worst = std::max(worst, (double) std::fabs(both.r[i] - (a.r[i] + b.r[i])));
  }
  CHECK(worst < 1e-6);
}

// Breed a little everywhere: a placed row, an open one, a blank, and a pressed-but-not-chosen pad.
static void breedSome(Instrument& inst) {
  inst.setZone(kBench, Zone::Kick);
  inst.setLock(kBench, Decay, true);
  for (int pad : { 0, 1, 1, 2 }) pickOnBench(inst, pad);
  inst.noteOn(noteFor(kBench, 1), 1.0f);
  inst.setValue(kBench, Pan, 0.1);
  inst.setPadMono(kBench, 2, true);
  inst.setRowLevel(kBench, -4.5);
  inst.place(3);
  inst.setOpen(3, true);
  inst.setZone(kBench, Zone::Snare);
  pickOnBench(inst, 2);
  inst.place(1);
  inst.noteOn(noteFor(kBench, 0), 1.0f);
}

TEST("saved state loads into a fresh instrument exactly, and breeding carries on the same way") {
  Instrument a = ready(5);
  breedSome(a);
  Instrument b = ready(999);
  CHECK(b.load(a.save()));
  CHECK(a.save() == b.save());
  CHECK(b.isOpen(3) && !b.isOpen(1) && b.row(2) == nullptr);

  for (Instrument* inst : { &a, &b }) {
    inst->noteOn(noteFor(kBench, 3), 1.0f);
    inst->noteOn(noteFor(3, 2), 1.0f);
    inst->noteOn(noteFor(3, 3), 1.0f);
    inst->place(2); // the next fresh workbench comes from the saved seed too
  }
  CHECK(a.save() == b.save());
}

TEST("text that is not a saved state is refused and changes nothing") {
  Instrument inst = ready(5);
  breedSome(inst);
  const std::string before = inst.save();
  std::string truncated = before.substr(0, before.size() / 2);
  std::string oldSpike = "bloodline 1\nrow 1 0 -1 -1 0.25 0 00000000";
  for (const std::string& bad : { std::string(""), std::string("hello"), truncated, oldSpike, std::string("bloodline 99 1\n") })
    CHECK(!inst.load(bad));
  CHECK(inst.save() == before);
}

CHECK_MAIN
