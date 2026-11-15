#include "studio.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>

namespace bb2 {

Studio::Studio(std::uint32_t seed) : drums(seed), loops(seed * 2654435761U + 7) { sync(); }

void Studio::sync() {
  std::array<int, kKitPads> zones{};
  for (int p = 0; p < kKitPads; ++p) zones[(size_t) p] = drums.pad(p).full ? (int) drums.pad(p).zone : -1;
  loops.setPads(zones);
}

void Studio::keepDrum(int tile, int pad) { drums.keep(tile, pad); sync(); }

void Studio::swapPads(int a, int b) {
  if (a == b) return;
  drums.swapPads(a, b);
  loops.swapPads(a, b);
  sync();
}

void Studio::clearPad(int pad) { drums.clearPad(pad); sync(); }

// bloodline-breaks-2 2, then the drums, then the loops.
std::string Studio::save() const {
  std::ostringstream out;
  out.imbue(std::locale::classic());
  out << std::setprecision(17) << "bloodline-breaks-2 2\n";
  drums.save(out);
  loops.save(out);
  return out.str();
}

bool Studio::load(const std::string& text) {
  std::istringstream in(text);
  in.imbue(std::locale::classic());
  std::string word;
  int version = 0;
  if (!(in >> word >> version) || word != "bloodline-breaks-2" || version != 2) return false;
  Drums d = drums;
  Loops l = loops;
  if (!d.load(in) || !l.load(in)) return false;
  drums = d;
  loops = l;
  sync();
  return true;
}

int laneFor(Zone z) {
  switch (z) {
    case Zone::Kick: return KickLane;
    case Zone::Snare: return SnareLane;
    case Zone::Hats: return HatLane;
    default: return PercLane;
  }
}

void Player::advance(const Pattern& p, const PatternOps& ops, int n, double bpm, double hostBeat, bool on, std::vector<Event>& out) {
  const bool host = hostBeat >= 0.0;
  // Flams still due from the last block land first, wherever the loop has got to.
  int keep = 0;
  for (int i = 0; i < pending_; ++i) {
    Later l = later_[(size_t) i];
    if (l.samples < n) {
      l.e.offset = std::clamp((int) l.samples, 0, n - 1);
      if (on) out.push_back(l.e);
    } else {
      l.samples -= n;
      later_[(size_t) keep++] = l;
    }
  }
  pending_ = keep;
  if (!on) { running_ = wasFree_ = false; pending_ = 0; return; }
  if (!host && !wasFree_) freeBeat_ = 0.0; // from the top of the bar
  wasFree_ = !host;
  running_ = true;

  const double perSample = std::clamp(bpm, 20.0, 400.0) / 60.0 / sr_;
  const double start = host ? hostBeat : freeBeat_;
  const double end = start + n * perSample;
  if (!host) freeBeat_ = std::fmod(end, 4.0);
  beat_ = std::fmod(std::fmod(end, 4.0) + 4.0, 4.0);

  const int count = hitsOf(p, ops, hits_.data(), kMaxHits);
  for (int i = 0; i < count; ++i) {
    const Hit& h = hits_[(size_t) i];
    for (double t = h.beat + 4.0 * std::ceil((start - h.beat) / 4.0); t < end; t += 4.0) {
      const double at = (t - start) / perSample + h.delayMs * 0.001 * sr_;
      const Event e{ 0, h.lane, h.pad, h.velocity, h.primary };
      if (at < n) {
        out.push_back(e);
        out.back().offset = std::clamp((int) std::lround(at), 0, n - 1);
      } else if (pending_ < (int) later_.size()) {
        later_[(size_t) pending_++] = { at - n, e };
      }
    }
  }
}

} // namespace bb2
