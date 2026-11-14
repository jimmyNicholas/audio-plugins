// The Voice seam: trigger a setting set, render samples, and listen with numbers.
#include "check.h"
#include "voice.h"
#include <algorithm>
#include <vector>

using namespace bloodline;

constexpr double kRate = 48000.0;

static std::vector<float> renderOf(const Values& v, float velocity, double seconds) {
  Voice voice;
  voice.prepare(kRate);
  voice.trigger(v, velocity);
  std::vector<float> out(static_cast<size_t>(seconds * kRate), 0.0f);
  for (size_t i = 0; i < out.size(); i += 256) voice.render(out.data() + i, (int) std::min<size_t>(256, out.size() - i));
  return out;
}

static double peak(const std::vector<float>& x, size_t from = 0, size_t to = 0) {
  if (to == 0) to = x.size();
  double p = 0;
  for (size_t i = from; i < to; ++i) p = std::max(p, (double) std::fabs(x[i]));
  return p;
}

// The middle of a zone on every setting.
static Values centre(Zone z) {
  Values v{};
  for (int i = 0; i < kNumSettings; ++i) { Range r = zoneRange(z, i); v[i] = (r.lo + r.hi) / 2; }
  return v;
}

TEST("a voice is silent until triggered, then makes a finite sound no louder than full scale") {
  Voice voice;
  voice.prepare(kRate);
  float before[512] = {};
  voice.render(before, 512);
  CHECK(!voice.active());
  CHECK_EQ(peak(std::vector<float>(before, before + 512)), 0.0);

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

TEST("Decay sets how long the sound lasts, and the voice goes quiet and inactive after it") {
  Values shortV = centre(Zone::Kick), longV = shortV;
  shortV[Decay] = toNorm(Decay, 0.1);
  longV[Decay] = toNorm(Decay, 0.8);
  const double a = tail(renderOf(shortV, 1.0f, 2.0)), b = tail(renderOf(longV, 1.0f, 2.0));
  CHECK(a > 0.03 && a < 0.15);
  CHECK(b > 0.35 && b < 1.0);

  Voice voice;
  voice.prepare(kRate);
  voice.trigger(shortV, 1.0f);
  std::vector<float> x(static_cast<size_t>(kRate), 0.0f);
  voice.render(x.data(), (int) x.size());
  CHECK(!voice.active());
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

// Click is an instant transient by design, so it is off here to hear only the cut.
TEST("triggering again cuts the voice's own tail without a jump") {
  Voice voice;
  voice.prepare(kRate);
  Values v = centre(Zone::Kick);
  v[Click] = 0.0;
  voice.trigger(v, 1.0f);
  std::vector<float> x(4800, 0.0f);
  voice.render(x.data(), 2400);
  voice.trigger(v, 0.5f);
  voice.render(x.data() + 2400, 2400);
  CHECK(std::fabs(x[2400] - x[2399]) < 0.1);
  CHECK(peak(x) <= 1.0);
}

TEST("anywhere in the full range, at any common sample rate, the sound is finite and within full scale") {
  std::uint32_t r = 12345;
  auto next = [&r] { r = r * 1664525u + 1013904223u; return (r >> 8) / 16777216.0; };
  for (double rate : { 44100.0, 48000.0, 96000.0 }) {
    for (int trial = 0; trial < 60; ++trial) {
      Values v{};
      for (auto& x : v) x = trial < 2 ? trial : next(); // all zeros, all ones, then random
      Voice voice;
      voice.prepare(rate);
      voice.trigger(v, 1.0f);
      std::vector<float> x(static_cast<size_t>(rate * 0.3), 0.0f);
      voice.render(x.data(), (int) x.size());
      bool ok = std::all_of(x.begin(), x.end(), [](float s) { return std::isfinite(s) && std::fabs(s) <= 1.0f; });
      CHECK(ok);
    }
  }
}

CHECK_MAIN
