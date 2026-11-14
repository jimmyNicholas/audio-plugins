#include "riff.h"
#include <algorithm>
#include <cmath>

namespace synth {

namespace {
constexpr int kSteps = 32; // two bars of sixteenths

// Steps are { step, length in sixteenths, notes, velocity }. C minor, around C2 for the bass and C3 above.
using S = std::vector<int>;
} // namespace

const char* Riff::patternName(int p) {
  switch (p) {
    case 0: return "Bass";
    case 1: return "Chords";
    default: return "Arp";
  }
}

const std::vector<Riff::Step>& Riff::steps() const {
  static const std::vector<Step> bass = {
    { 0, 3, S{ 36 }, 1.0f }, { 3, 1, S{ 36 }, 0.6f }, { 6, 2, S{ 48 }, 0.8f }, { 10, 2, S{ 39 }, 0.9f },
    { 14, 2, S{ 41 }, 0.7f }, { 16, 3, S{ 34 }, 1.0f }, { 22, 2, S{ 46 }, 0.8f }, { 26, 4, S{ 43 }, 0.9f },
  };
  static const std::vector<Step> chords = {
    { 0, 6, S{ 48, 51, 55, 58 }, 0.9f }, { 7, 2, S{ 48, 51, 55, 58 }, 0.6f },
    { 16, 6, S{ 46, 50, 53, 56 }, 0.9f }, { 23, 2, S{ 44, 48, 51, 55 }, 0.7f }, { 26, 4, S{ 43, 47, 50, 53 }, 0.8f },
  };
  static const std::vector<Step> arp = [] {
    std::vector<Step> a;
    const int up[8] = { 60, 63, 67, 70, 72, 70, 67, 63 };
    for (int i = 0; i < 16; ++i) a.push_back({ i * 2, 2, S{ up[i % 8] - (i >= 8 ? 2 : 0) }, i % 4 == 0 ? 1.0f : 0.7f });
    return a;
  }();
  switch (pattern_) {
    case 0: return bass;
    case 1: return chords;
    default: return arp;
  }
}

void Riff::setPattern(int p) {
  p = std::clamp(p, 0, kPatterns - 1);
  if (p == pattern_) return;
  pattern_ = p;
  if (on_) stopping_ = true; // end the old pattern's notes; the new one starts on its next step
}

void Riff::setOn(bool on) {
  if (on == on_) return;
  on_ = on;
  if (!on) stopping_ = true;
  else pos_ = 0;
}

void Riff::advance(int n, double bpm, std::vector<Event>& events) {
  if (stopping_) {
    for (int note : sounding_) events.push_back({ 0, note, 0.0f, false });
    sounding_.clear();
    stopping_ = false;
  }
  if (!on_ || n <= 0) return;
  const double perSample = std::clamp(bpm, 20.0, 400.0) / 60.0 * 4.0 / sr_; // sixteenths per sample
  const double end = pos_ + perSample * n;
  // Every step boundary crossed in this block, in order.
  for (int k = static_cast<int>(std::ceil(pos_)); k < end; ++k) {
    const int offset = std::clamp(static_cast<int>((k - pos_) / perSample), 0, n - 1);
    const int step = k % kSteps;
    for (const Step& s : steps()) {
      if ((s.step + s.length) % kSteps == step)
        for (int note : s.notes) {
          auto it = std::find(sounding_.begin(), sounding_.end(), note);
          if (it != sounding_.end()) { sounding_.erase(it); events.push_back({ offset, note, 0.0f, false }); }
        }
    }
    for (const Step& s : steps())
      if (s.step == step)
        for (int note : s.notes) { events.push_back({ offset, note, s.velocity, true }); sounding_.push_back(note); }
  }
  pos_ = end;
  if (pos_ >= kSteps * 64) pos_ -= kSteps * 64; // keep it small; a multiple of the pattern
}

} // namespace synth
