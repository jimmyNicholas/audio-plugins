// The Engine seam: notes and a patch in, stereo out. And the Riff and the saved state.
#include "../../shared/test/check.h"
#include "breeder.h"
#include "engine.h"
#include "riff.h"
#include <algorithm>
#include <vector>

using namespace synth;

constexpr double kRate = 48000.0;

struct Stereo { std::vector<float> l, r; };

static Stereo run(Engine& e, const Patch& p, double seconds) {
  Stereo out{ std::vector<float>(static_cast<size_t>(seconds * kRate)), {} };
  out.r = out.l;
  for (size_t i = 0; i < out.l.size(); i += 256)
    e.render(p, out.l.data() + i, out.r.data() + i, (int) std::min<size_t>(256, out.l.size() - i));
  return out;
}

static double peak(const std::vector<float>& x) {
  double m = 0;
  for (float s : x) m = std::max(m, (double) std::fabs(s));
  return m;
}

// A plain, sustained saw: open filter, no grit, no motion, centred.
static Patch plain() {
  Patch p{};
  p.fill(0.0);
  p[ShapeA] = toNorm(ShapeA, 0.66);
  p[TuneB] = toNorm(TuneB, 0);
  p[Cutoff] = toNorm(Cutoff, 8000);
  p[EnvAmount] = toNorm(EnvAmount, 0);
  p[Attack] = 0; p[Decay] = 0.5; p[Sustain] = 0.8; p[Release] = toNorm(Release, 0.05);
  p[LfoRate] = 0.5;
  p[Glide] = 0;
  p[Bits] = 0;
  p[Level] = 1;
  return p;
}

TEST("silent until a note, then a finite sound within full scale that stops after release") {
  Engine e;
  e.prepare(kRate);
  const Patch p = plain();
  CHECK_EQ(peak(run(e, p, 0.1).l), 0.0);
  e.noteOn(60, 1.0f, p);
  const Stereo held = run(e, p, 0.3);
  CHECK(peak(held.l) > 0.05 && peak(held.l) <= 1.0);
  e.noteOff(60);
  run(e, p, 0.5);
  CHECK_EQ(e.sounding(), 0);
  CHECK_EQ(peak(run(e, p, 0.1).l), 0.0);
}

TEST("width 0 is the same both sides; width 1 is not") {
  Engine e;
  e.prepare(kRate);
  Patch p = plain();
  e.noteOn(57, 1.0f, p);
  const Stereo narrow = run(e, p, 0.2);
  CHECK(narrow.l == narrow.r);
  p[Width] = 1.0;
  const Stereo wide = run(e, p, 0.2);
  CHECK(wide.l != wide.r);
}

// Brightness: how big the sample-to-sample steps are next to the signal itself.
static double brightness(const std::vector<float>& x) {
  double d = 0, e = 0;
  for (size_t i = 1; i < x.size(); ++i) { d += (x[i] - x[i - 1]) * (x[i] - x[i - 1]); e += x[i] * x[i]; }
  return std::sqrt(d / std::max(e, 1e-12));
}

TEST("a held note follows the patch: closing the filter darkens it at once") {
  Engine e;
  e.prepare(kRate);
  Patch p = plain();
  p[ShapeA] = toNorm(ShapeA, 1.0); // square: lots of harmonics
  e.noteOn(48, 1.0f, p);
  const Stereo bright = run(e, p, 0.2);
  p[Cutoff] = toNorm(Cutoff, 200);
  run(e, p, 0.02);
  const Stereo dark = run(e, p, 0.2);
  CHECK(peak(dark.l) > 0.01); // still sounding
  CHECK(brightness(dark.l) < 0.5 * brightness(bright.l));
}

TEST("eight voices at most: a ninth note takes the oldest") {
  Engine e;
  e.prepare(kRate);
  const Patch p = plain();
  for (int n = 0; n < 9; ++n) e.noteOn(48 + n, 0.8f, p);
  CHECK_EQ(e.sounding(), Engine::kVoices);
}

TEST("anywhere in the full range, the sound stays finite and within full scale") {
  breed::Rng rng(11);
  PatchOps ops;
  for (int trial = 0; trial < 40; ++trial) {
    Patch p = ops.fresh(rng);
    if (trial == 0) p.fill(0.0);
    if (trial == 1) p.fill(1.0);
    Engine e;
    e.prepare(kRate);
    for (int n : { 24, 48, 60, 67, 96 }) e.noteOn(n, 1.0f, p);
    const Stereo x = run(e, p, 0.4);
    bool ok = true;
    for (size_t i = 0; i < x.l.size(); ++i) ok = ok && std::isfinite(x.l[i]) && std::fabs(x.l[i]) <= 1.0f && std::fabs(x.r[i]) <= 1.0f;
    CHECK(ok);
  }
}

TEST("the riff: notes on the sixteenths at the tempo, and its notes end when it stops") {
  Riff riff;
  riff.prepare(kRate);
  riff.setPattern(2); // arp: a note every eighth
  riff.setOn(true);
  std::vector<Riff::Event> events;
  const int block = 512;
  int ons = 0, offs = 0;
  for (int b = 0; b < (int) (kRate * 2.0 / block); ++b) { // two seconds at 120 BPM: one bar
    events.clear();
    riff.advance(block, 120.0, events);
    for (const auto& ev : events) { (ev.on ? ons : offs) += 1; CHECK(ev.offset >= 0 && ev.offset < block); }
  }
  CHECK(ons >= 8 && ons <= 9);
  riff.setOn(false);
  events.clear();
  riff.advance(block, 120.0, events);
  int ended = 0;
  for (const auto& ev : events) ended += !ev.on;
  CHECK_EQ(ons, offs + ended);
}

TEST("the breeder saves and loads exactly: candidates, heard, parent, spread, zone and locks") {
  Breeder a(5);
  a.setZone(Zone::Pad);
  a.setLock(Attack, true);
  a.breedFrom(3);
  a.cross(3, 6);
  a.select(2);
  a.setValue(Cutoff, 0.25);
  Breeder b(77);
  CHECK(b.load(a.save()));
  CHECK(a.save() == b.save());
  CHECK(b.zone() == Zone::Pad && b.locked(Attack));
  CHECK_EQ(b.heard()[Cutoff], 0.25);
  a.breedFrom(2);
  b.breedFrom(2);
  CHECK(a.save() == b.save());
  const std::string before = b.save();
  CHECK(!b.load("bloodline-synth 2\n"));
  CHECK(!b.load(before.substr(0, before.size() / 2)));
  CHECK(b.save() == before);
}

CHECK_MAIN
