// The Voice seam: trigger a setting set, render samples, and listen with numbers.
#include "check.h"
#include "voice.h"
#include <algorithm>
#include <vector>

using namespace bloodline;

constexpr double kRate = 48000.0;

struct Stereo { std::vector<float> l, r; };

static Stereo stereoOf(const Values& v, float velocity, double seconds, double pan, double db) {
  Voice voice;
  voice.prepare(kRate);
  voice.trigger(v, velocity, pan, db);
  Stereo out{ std::vector<float>(static_cast<size_t>(seconds * kRate), 0.0f), {} };
  out.r = out.l;
  for (size_t i = 0; i < out.l.size(); i += 256)
    voice.render(out.l.data() + i, out.r.data() + i, (int) std::min<size_t>(256, out.l.size() - i));
  return out;
}

static std::vector<float> renderOf(const Values& v, float velocity, double seconds) {
  return stereoOf(v, velocity, seconds, 0.0, 0.0).l;
}

static double peak(const std::vector<float>& x, size_t from = 0, size_t to = 0) {
  if (to == 0) to = x.size();
  double p = 0;
  for (size_t i = from; i < to; ++i) p = std::max(p, (double) std::fabs(x[i]));
  return p;
}

// Grit and Out left clean: no drive, 16 bits, 48 kHz, filters wide open, centred, 0 dB.
static Values clean() {
  Values v{};
  v.fill(0.5);
  v[Drive] = 0; v[Bits] = 0; v[Rate] = 0; v[HighPass] = 0; v[LowPass] = 1; v[Resonance] = 0;
  v[Pan] = 0.5; v[Level] = 1;
  return v;
}

// The middle of a zone on every zoned setting, clean elsewhere.
static Values centre(Zone z) {
  Values v = clean();
  for (int i = 0; i <= ClickTone; ++i) { Range r = zoneRange(z, i); v[i] = (r.lo + r.hi) / 2; }
  return v;
}

TEST("the settings: 19 in five groups, in signal order, Level the one that is not bred") {
  int n = 0;
  for (int g = 0; g < kNumGroups; ++g) { CHECK_EQ(group(g).first, n); n += group(g).count; }
  CHECK_EQ(n, kNumSettings);
  for (int i = 0; i < kNumSettings; ++i) CHECK(info(i).bred == (i != Level));
  CHECK_NEAR(toReal(Bits, 0), 16, 1e-9);
  CHECK_NEAR(toReal(Bits, 1), 2, 1e-9);
  CHECK_NEAR(toReal(Rate, 1), 1000, 1e-6);
  CHECK_NEAR(toNorm(Rate, 6928.2), 0.5, 1e-3);
  CHECK_NEAR(toReal(Pitch, 0), 20, 1e-9);
  CHECK_NEAR(toReal(Pitch, 1), 2000, 1e-6);
}

TEST("zones centre only Body, Noise and Click") {
  for (Zone z : { Zone::Kick, Zone::Snare, Zone::Hats })
    for (int i = 0; i < kNumSettings; ++i) {
      const Range r = zoneRange(z, i);
      if (i <= ClickTone) CHECK(r.hi - r.lo < 1.0);
      else CHECK(r.lo == 0.0 && r.hi == 1.0);
    }
}

TEST("a voice is silent until triggered, then makes a finite sound no louder than full scale") {
  Voice voice;
  voice.prepare(kRate);
  float l[512] = {}, r[512] = {};
  voice.render(l, r, 512);
  CHECK(!voice.active());
  CHECK_EQ(peak(std::vector<float>(l, l + 512)), 0.0);

  for (Zone z : { Zone::Kick, Zone::Snare, Zone::Hats }) {
    auto x = renderOf(centre(z), 1.0f, 0.5);
    bool finite = std::all_of(x.begin(), x.end(), [](float s) { return std::isfinite(s); });
    CHECK(finite);
    CHECK(peak(x) > 0.1);
    CHECK(peak(x) <= 1.0);
  }
}

// Seconds until the sound stays below -40 dB of its peak.
static double tail(const std::vector<float>& x) {
  const double limit = peak(x) * 0.01;
  for (size_t i = x.size(); i-- > 0;) if (std::fabs(x[i]) > limit) return i / kRate;
  return 0;
}

// Zero crossings per second: a rough brightness.
static double crossings(const std::vector<float>& x, size_t to) {
  int n = 0;
  for (size_t i = 1; i < to; ++i) if ((x[i - 1] < 0) != (x[i] < 0)) ++n;
  return n / (to / kRate);
}

TEST("Decay sets how long the body lasts, and the voice goes quiet and inactive after it") {
  Values shortV = centre(Zone::Kick), longV = shortV;
  shortV[BodyNoise] = longV[BodyNoise] = 0;
  shortV[Decay] = toNorm(Decay, 0.1);
  longV[Decay] = toNorm(Decay, 0.8);
  const double a = tail(renderOf(shortV, 1.0f, 2.0)), b = tail(renderOf(longV, 1.0f, 2.0));
  CHECK(a > 0.03 && a < 0.15);
  CHECK(b > 0.35 && b < 1.0);

  Voice voice;
  voice.prepare(kRate);
  voice.trigger(shortV, 1.0f);
  std::vector<float> l(static_cast<size_t>(kRate), 0.0f), r = l;
  voice.render(l.data(), r.data(), (int) l.size());
  CHECK(!voice.active());
}

TEST("Noise decay is the noise's own tail, apart from the body's") {
  Values v = centre(Zone::Snare);
  v[BodyNoise] = 1.0; // noise only
  v[Click] = 0;
  v[Decay] = toNorm(Decay, 0.05);
  v[NoiseDecay] = toNorm(NoiseDecay, 0.05);
  const double shortTail = tail(renderOf(v, 1.0f, 2.0));
  v[NoiseDecay] = toNorm(NoiseDecay, 0.6);
  const double longTail = tail(renderOf(v, 1.0f, 2.0));
  CHECK(shortTail < 0.1);
  CHECK(longTail > 0.25);
}

TEST("Attack slows the body's start") {
  Values v = centre(Zone::Kick);
  v[BodyNoise] = 0; v[Click] = 0;
  v[Attack] = 0;
  const double fast = peak(renderOf(v, 1.0f, 0.1), 0, 96); // the first 2 ms
  v[Attack] = 1; // 50 ms
  const double slow = peak(renderOf(v, 1.0f, 0.1), 0, 96);
  CHECK(slow < fast * 0.3);
}

TEST("velocity sets the level") {
  const Values v = centre(Zone::Snare);
  const double soft = peak(renderOf(v, 0.3f, 0.3)), hard = peak(renderOf(v, 1.0f, 0.3));
  CHECK(soft < hard * 0.6);
  CHECK(soft > hard * 0.1);
}

TEST("a kick from the middle of its zone is darker than a snare, and a snare darker than hats") {
  const double kick = crossings(renderOf(centre(Zone::Kick), 1.0f, 0.2), 4800);
  const double snare = crossings(renderOf(centre(Zone::Snare), 1.0f, 0.2), 4800);
  const double hats = crossings(renderOf(centre(Zone::Hats), 1.0f, 0.2), 4800);
  CHECK(kick < snare);
  CHECK(snare < hats);
  CHECK(kick < 400);
  CHECK(hats > 4000);
}

TEST("Low-pass darkens and High-pass thins") {
  Values v = centre(Zone::Hats);
  const double open = crossings(renderOf(v, 1.0f, 0.2), 4800);
  v[LowPass] = toNorm(LowPass, 800);
  const double dark = crossings(renderOf(v, 1.0f, 0.2), 4800);
  CHECK(dark < open * 0.5);

  Values k = centre(Zone::Kick);
  const double full = peak(renderOf(k, 1.0f, 0.3));
  k[HighPass] = toNorm(HighPass, 2000);
  k[Click] = 0;
  const double thin = peak(renderOf(k, 1.0f, 0.3));
  CHECK(thin < full * 0.4);
}

// The filters after the crusher smooth its steps a little, so compare against the clean sound.
static double rmsDiff(const std::vector<float>& a, const std::vector<float>& b) {
  double d = 0, e = 0;
  for (size_t i = 0; i < a.size(); ++i) { d += (a[i] - b[i]) * (a[i] - b[i]); e += b[i] * b[i]; }
  return std::sqrt(d / e);
}

TEST("Bits and Rate crush: few bits or a low rate change the sound a lot, the clean ends not at all") {
  const Values v = centre(Zone::Snare);
  const auto cleanX = renderOf(v, 1.0f, 0.1);
  Values b = v;
  b[Bits] = toNorm(Bits, 3);
  CHECK(rmsDiff(renderOf(b, 1.0f, 0.1), cleanX) > 0.2);
  Values r = v;
  r[Rate] = toNorm(Rate, 2000);
  CHECK(rmsDiff(renderOf(r, 1.0f, 0.1), cleanX) > 0.2);
  b[Bits] = toNorm(Bits, 16);
  CHECK(rmsDiff(renderOf(b, 1.0f, 0.1), cleanX) < 1e-6);
}

TEST("pan and level: centred is equal in both sides, hard left is silent on the right, -12 dB is a quarter") {
  const Values v = centre(Zone::Snare);
  const Stereo mid = stereoOf(v, 1.0f, 0.2, 0.0, 0.0);
  CHECK(mid.l == mid.r);
  const Stereo left = stereoOf(v, 1.0f, 0.2, -1.0, 0.0);
  CHECK(left.l == mid.l);
  CHECK_EQ(peak(left.r), 0.0);
  const Stereo quiet = stereoOf(v, 1.0f, 0.2, 0.0, -12.0);
  CHECK_NEAR(peak(quiet.l) / peak(mid.l), std::pow(10.0, -12.0 / 20.0), 0.01);
}

// Click is an instant transient by design, so it is off here to hear only the cut.
TEST("triggering again cuts the voice's own tail without a jump") {
  Voice voice;
  voice.prepare(kRate);
  Values v = centre(Zone::Kick);
  v[Click] = 0.0;
  voice.trigger(v, 1.0f);
  std::vector<float> x(4800, 0.0f), r(4800, 0.0f);
  voice.render(x.data(), r.data(), 2400);
  voice.trigger(v, 0.5f);
  voice.render(x.data() + 2400, r.data() + 2400, 2400);
  CHECK(std::fabs(x[2400] - x[2399]) < 0.1);
  CHECK(peak(x) <= 1.0);
}

TEST("anywhere in the full range, at any common sample rate and pan, the sound is finite and within full scale") {
  std::uint32_t r = 12345;
  auto next = [&r] { r = r * 1664525u + 1013904223u; return (r >> 8) / 16777216.0; };
  for (double rate : { 44100.0, 48000.0, 96000.0 }) {
    for (int trial = 0; trial < 60; ++trial) {
      Values v{};
      for (auto& x : v) x = trial < 2 ? trial : next(); // all zeros, all ones, then random
      Voice voice;
      voice.prepare(rate);
      voice.trigger(v, 1.0f, next() * 2 - 1, 6.0);
      std::vector<float> l(static_cast<size_t>(rate * 0.3), 0.0f), rr = l;
      voice.render(l.data(), rr.data(), (int) l.size());
      bool ok = std::all_of(l.begin(), l.end(), [](float s) { return std::isfinite(s) && std::fabs(s) <= 1.0f; }) &&
                std::all_of(rr.begin(), rr.end(), [](float s) { return std::isfinite(s) && std::fabs(s) <= 1.0f; });
      CHECK(ok);
    }
  }
}

CHECK_MAIN
