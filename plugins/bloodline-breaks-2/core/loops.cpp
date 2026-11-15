#include "loops.h"
#include "drums.h"
#include <algorithm>
#include <cmath>
#include <istream>
#include <ostream>
#include <string>

namespace bb2 {

namespace {

// A first guess at each style, by ear. Digits are tenths: 9 is nearly always, 0 is never (but a
// free draw can still land there). Steps run 1 e & a 2 e & a ... Lanes: kick, snare, hat, perc.
// Strung: loose and drunk, kicks pushed about, snares late. Neo soul: slow, swung, ghost snares.
const StyleInfo kStyles[kNumStyles] = {
  { "None",     { "", "", "", "" }, 0.58, 0.00, 0.00, 0.00, 1.00, 90 },
  { "Boom bap", { "9012003421810130", "0001900200019102", "8383838383838383", "0000000100000030" }, 0.58, 0.02, 0.04, 0.00, 1.00, 90 },
  { "Lo-fi",    { "9002001020900100", "0000900100009001", "7272727272727272", "0000000000000020" }, 0.60, 0.05, 0.05, 0.00, 0.80, 80 },
  { "Strung",   { "8003002050300201", "0000920100009201", "6464646464646464", "0010000000100000" }, 0.54, 0.07, 0.14, 0.04, 0.90, 88 },
  { "Neo soul", { "9001002080020100", "0010903001109031", "7575757575757575", "0000001000000010" }, 0.63, 0.04, 0.07, 0.02, 0.85, 76 },
};

constexpr double kFlat[kLanes] = { 0.2, 0.12, 0.45, 0.1 };  // the chance of a hit with no style at all
constexpr double kLayer[kLanes] = { 0.1, 0.35, 0.15, 0.2 }; // the chance a lane layers two sounds
constexpr double kOther[kLanes] = { 0.15, 0.15, 0.15, 0.4 }; // the chance a lane borrows another lane's pad

// Which zone of pad each lane is for.
constexpr int kLaneZone[kLanes] = { (int) Zone::Kick, (int) Zone::Snare, (int) Zone::Hats, (int) Zone::Perc };
constexpr int kWild = (int) Zone::None;

double styleOdds(Style s, int lane, int step) {
  if (s == Style::None) return kFlat[lane];
  return (kStyles[(int) s].odds[lane][step] - '0') / 10.0;
}
double clampSwing(double s) { return std::clamp(s, kSwingMin, kSwingMax); }
float clampVel(double v) { return (float) std::clamp(v, 0.08, 1.0); }

std::uint8_t drawRoll(const StyleInfo& st, int lane, breed::Rng& rng) {
  if (lane != HatLane || !rng.chance(st.rolls)) return 1;
  return (std::uint8_t) (2 + rng.below(3));
}

double drawNudge(Style s, double pull, breed::Rng& rng) {
  if (s == Style::None) return 0.0;
  const StyleInfo& st = styleInfo(s);
  const double late = st.late * pull, loose = 0.02 + (st.loose - 0.02) * pull;
  return std::clamp(late + rng.gauss() * loose, -0.45, 0.45);
}

int poisson(double lambda, breed::Rng& rng) {
  const double l = std::exp(-lambda);
  int k = 0;
  double p = rng.uniform();
  while (p > l && k < 64) { ++k; p *= rng.uniform(); }
  return k;
}

// The distance between two positions round the bar, in sixteenths.
double gap(double a, double b) {
  const double d = std::fabs(a - b);
  return std::min(d, kSteps - d);
}

bool stepEmpty(const LaneData& l, int step) {
  for (int i = 0; i < l.count; ++i)
    if (gap(l.hits[(size_t) i].pos, step) < 0.5) return false;
  return true;
}

} // namespace

const StyleInfo& styleInfo(Style s) { return kStyles[(int) s]; }

double wrapBar(double pos) {
  if (!std::isfinite(pos)) return 0.0;
  double p = std::fmod(pos, (double) kSteps);
  if (p < 0) p += kSteps;
  return p >= kSteps ? 0.0 : p;
}

double warp(double pos, double swing) {
  pos = wrapBar(pos);
  const double e = 2.0 * std::floor(pos / 2.0), x = pos - e;
  return e + (x < 1.0 ? 2.0 * swing * x : 2.0 * swing + (2.0 - 2.0 * swing) * (x - 1.0));
}

double unwarp(double landed, double swing) {
  landed = wrapBar(landed);
  const double e = 2.0 * std::floor(landed / 2.0), y = landed - e;
  return e + (y < 2.0 * swing ? y / (2.0 * swing) : 1.0 + (y - 2.0 * swing) / (2.0 - 2.0 * swing));
}

bool LaneData::add(double pos, float vel, std::uint8_t roll) {
  if (count >= kMaxLaneHits) return false;
  hits[(size_t) count++] = { wrapBar(pos), clampVel(vel), (std::uint8_t) std::clamp((int) roll, 1, 4) };
  return true;
}

void LaneData::remove(int i) {
  if (i < 0 || i >= count) return;
  for (int k = i; k + 1 < count; ++k) hits[(size_t) k] = hits[(size_t) k + 1];
  hits[(size_t) --count] = Stroke{};
}

void LaneData::tidy() {
  for (int i = 0; i < count; ++i) hits[(size_t) i].pos = wrapBar(hits[(size_t) i].pos);
  std::sort(hits.begin(), hits.begin() + count, [](const Stroke& a, const Stroke& b) { return a.pos < b.pos; });
  // Merge neighbours, round the bar's end too; the louder hit stays.
  int n = 0;
  for (int i = 0; i < count; ++i) {
    if (n > 0 && gap(hits[(size_t) n - 1].pos, hits[(size_t) i].pos) < kMergeGap) {
      if (hits[(size_t) i].vel > hits[(size_t) n - 1].vel) hits[(size_t) n - 1] = hits[(size_t) i];
      continue;
    }
    hits[(size_t) n++] = hits[(size_t) i];
  }
  if (n > 1 && gap(hits[0].pos, hits[(size_t) n - 1].pos) < kMergeGap) {
    if (hits[(size_t) n - 1].vel > hits[0].vel) hits[0] = hits[(size_t) n - 1];
    --n;
  }
  count = n;
  for (int i = count; i < kMaxLaneHits; ++i) hits[(size_t) i] = Stroke{};
}

int LaneData::find(double pos) const {
  for (int i = 0; i < count; ++i)
    if (gap(hits[(size_t) i].pos, pos) < kMergeGap) return i;
  return -1;
}

bool LaneData::operator==(const LaneData& o) const {
  if (count != o.count || first != o.first || second != o.second || layer != o.layer || flam != o.flam || flamLevel != o.flamLevel)
    return false;
  for (int i = 0; i < count; ++i)
    if (!(hits[(size_t) i] == o.hits[(size_t) i])) return false;
  return true;
}

int Pattern::hits() const {
  int n = 0;
  for (const auto& l : lanes) n += l.count;
  return n;
}

double PatternOps::odds(int lane, int step) const {
  const double flat = kFlat[lane];
  if (style == Style::None) return flat;
  return std::clamp(flat + (styleOdds(style, lane, step) - flat) * pull, 0.0, 0.97);
}

// Louder on the beat than on the e and a; a snare off the backbeat is a ghost note.
float PatternOps::velocityFor(int lane, int step, breed::Rng& rng) const {
  const double metric = step % 4 == 0 ? 1.0 : step % 2 == 0 ? 0.8 : 0.6;
  double v = 0.7;
  switch (lane) {
    case KickLane: v = 0.7 + 0.25 * metric; break;
    case SnareLane:
      if (style == Style::None) v = 0.45 + 0.45 * metric;
      else v = styleOdds(style, lane, step) >= 0.5 ? 0.95 : 0.35;
      break;
    case HatLane: v = 0.35 + 0.55 * (metric - 0.5) * 2.0; break;
    default: v = 0.7; break;
  }
  v *= styleInfo(style).softness;
  v *= 0.88 + 0.24 * rng.uniform();
  return clampVel(v);
}

int PatternOps::choices(int lane, int* out) const {
  int n = 0;
  for (int p = 0; p < kKitPads; ++p)
    if (pads[(size_t) p] == kLaneZone[lane]) out[n++] = p;
  for (int p = 0; p < kKitPads; ++p)
    if (pads[(size_t) p] == kWild) out[n++] = p;
  for (int p = 0; p < kKitPads; ++p)
    if (pads[(size_t) p] >= 0 && pads[(size_t) p] != kLaneZone[lane] && pads[(size_t) p] != kWild) out[n++] = p;
  return n;
}

bool PatternOps::anyKept() const {
  for (int z : pads)
    if (z >= 0) return true;
  return false;
}

namespace {
// The pads a lane draws from this time: its own label's, else the wildcards; sometimes another
// lane's instead.
int pool(const PatternOps& ops, int lane, breed::Rng& rng, int* out) {
  int own = 0, wild = 0, other = 0;
  int ownP[kKitPads], wildP[kKitPads], otherP[kKitPads];
  for (int p = 0; p < kKitPads; ++p) {
    const int z = ops.pads[(size_t) p];
    if (z < 0) continue;
    if (z == kLaneZone[lane]) ownP[own++] = p;
    else if (z == kWild) wildP[wild++] = p;
    else otherP[other++] = p;
  }
  const int* from = own ? ownP : wildP;
  int n = own ? own : wild;
  if (other > 0 && rng.chance(kOther[lane])) { from = otherP; n = other; }
  std::copy(from, from + n, out);
  return n;
}

void drawLayer(const PatternOps& ops, LaneData& l, int lane, breed::Rng& rng) {
  l.second = -1;
  l.layer = One;
  l.flam = (float) (kFlamMin + (kFlamMax - kFlamMin) * rng.uniform());
  l.flamLevel = (float) (kFlamLevelMin + (kFlamLevelMax - kFlamLevelMin) * rng.uniform());
  if (l.first < 0 || !rng.chance(kLayer[lane])) return;
  int c[kKitPads];
  const int n = pool(ops, lane, rng, c);
  if (n == 0) return;
  const bool flam = rng.chance(lane == SnareLane || lane == PercLane ? 0.6 : 0.5);
  if (flam) {
    l.second = (std::int8_t) c[rng.below(n)]; // may be the same pad: a flam of one drum
    l.layer = Flam;
    return;
  }
  int k = 0;
  int others[kKitPads];
  for (int i = 0; i < n; ++i)
    if (c[i] != l.first) others[k++] = c[i];
  if (k == 0) return; // nothing else to stack with
  l.second = (std::int8_t) others[rng.below(k)];
  l.layer = Stack;
}

// Whether b differs from a in a way you can see: a hit added or removed, or moved by more than
// half a sixteenth.
bool visiblyDiffers(const LaneData& a, const LaneData& b) {
  if (a.count != b.count) return true;
  for (int i = 0; i < a.count; ++i) {
    double best = 99.0;
    for (int j = 0; j < b.count; ++j) best = std::min(best, gap(a.hits[(size_t) i].pos, b.hits[(size_t) j].pos));
    if (best > 0.5) return true;
  }
  return false;
}

// One change you can see and hear: a hit added on an empty step (likely steps first), removed
// (unlikely ones first), or moved by more than half a sixteenth.
void realChange(const PatternOps& ops, LaneData& l, int lane, double spread, breed::Rng& rng) {
  int empty[kSteps], ne = 0;
  double weight[kSteps], total = 0.0;
  for (int s = 0; s < kSteps; ++s)
    if (stepEmpty(l, s)) { empty[ne] = s; weight[ne] = ops.odds(lane, s) + 0.02; total += weight[ne]; ++ne; }
  const bool canAdd = ne > 0 && l.count < kMaxLaneHits, canTake = l.count > 0;
  int op = canTake ? rng.below(3) : 0; // 0 add, 1 remove, 2 move
  if (op == 0 && !canAdd) op = 2;
  if (!canAdd && !canTake) return;
  const StyleInfo& st = styleInfo(ops.style);
  if (op == 0) {
    double r = rng.uniform() * total;
    int k = 0;
    while (k < ne - 1 && r >= weight[k]) r -= weight[k++];
    const int s = empty[k];
    l.add(s + drawNudge(ops.style, ops.pull, rng), ops.velocityFor(lane, s, rng), drawRoll(st, lane, rng));
  } else if (op == 1) {
    double w[kMaxLaneHits], sum = 0.0;
    for (int i = 0; i < l.count; ++i) {
      const int step = (int) std::lround(l.hits[(size_t) i].pos) % kSteps;
      w[i] = 1.0 - ops.odds(lane, step) + 0.05;
      sum += w[i];
    }
    double r = rng.uniform() * sum;
    int k = 0;
    while (k < l.count - 1 && r >= w[k]) r -= w[k++];
    l.remove(k);
  } else {
    Stroke& h = l.hits[(size_t) rng.below(l.count)];
    const double d = (rng.chance(0.5) ? -1.0 : 1.0) * (0.55 + std::fabs(rng.gauss()) * spread * 2.0);
    h.pos = wrapBar(h.pos + d);
  }
}
} // namespace

void PatternOps::drawSound(LaneData& l, int lane, breed::Rng& rng) const {
  int c[kKitPads];
  const int n = pool(*this, lane, rng, c);
  l.first = n > 0 ? (std::int8_t) c[rng.below(n)] : (std::int8_t) -1;
  drawLayer(*this, l, lane, rng);
}

Pattern PatternOps::fresh(breed::Rng& rng) const {
  const StyleInfo& st = styleInfo(style);
  Pattern p;
  const double free = kSwingMin + (kSwingMax - kSwingMin) * rng.uniform();
  p.swing = clampSwing(free + (swingDial - free) * (0.4 + 0.6 * pull) + rng.gauss() * 0.01);
  for (int l = 0; l < kLanes; ++l) {
    LaneData& lane = p.lanes[(size_t) l];
    drawSound(lane, l, rng);
    for (int s = 0; s < kSteps; ++s)
      if (rng.chance(odds(l, s))) lane.add(s + drawNudge(style, pull, rng), velocityFor(l, s, rng), drawRoll(st, l, rng));
    lane.tidy();
  }
  return p;
}

// Every unlocked lane makes at least one real change, and about 1 + spread * 8 on average; every
// hit also drifts a little. A lane's sound is sometimes redrawn, and always when its first pad is
// no longer kept. Locked lanes copy whole.
Pattern PatternOps::mutate(const Pattern& from, double spread, breed::Rng& rng) const {
  const StyleInfo& st = styleInfo(style);
  Pattern p = from;
  double s = from.swing + rng.gauss() * spread * 0.08;
  s += (swingDial - s) * pull * 0.3;
  p.swing = clampSwing(s);
  for (int l = 0; l < kLanes; ++l) {
    if (locks[(size_t) l]) continue;
    LaneData& lane = p.lanes[(size_t) l];
    if (!kept(lane.first) || rng.chance(std::min(0.6, spread * 1.5))) drawSound(lane, l, rng);
    else {
      lane.flam = (float) std::clamp(lane.flam + rng.gauss() * spread * (kFlamMax - kFlamMin), kFlamMin, kFlamMax);
      lane.flamLevel = (float) std::clamp(lane.flamLevel + rng.gauss() * spread * (kFlamLevelMax - kFlamLevelMin), kFlamLevelMin, kFlamLevelMax);
      if (rng.chance(spread * 0.5)) drawLayer(*this, lane, l, rng);
    }
    for (int i = 0; i < lane.count; ++i) {
      Stroke& h = lane.hits[(size_t) i];
      h.pos = wrapBar(h.pos + rng.gauss() * spread * 0.6);
      h.vel = clampVel(h.vel + rng.gauss() * spread * 0.3);
      if (rng.chance(spread * 0.3)) h.roll = drawRoll(st, l, rng);
    }
    const int changes = 1 + poisson(spread * 8.0, rng);
    for (int c = 0; c < changes; ++c) realChange(*this, lane, l, spread, rng);
    lane.tidy();
    // Changes can cancel out (a hit added, then removed): keep going until one shows.
    for (int tries = 0; tries < 8 && !visiblyDiffers(from.lanes[(size_t) l], lane); ++tries) {
      realChange(*this, lane, l, spread, rng);
      lane.tidy();
    }
  }
  return p;
}

Pattern PatternOps::cross(const Pattern& a, const Pattern& b, breed::Rng& rng) const {
  Pattern p = a;
  for (int l = 0; l < kLanes; ++l)
    if (!locks[(size_t) l] && rng.chance(0.5)) p.lanes[(size_t) l] = b.lanes[(size_t) l];
  p.swing = rng.chance(0.5) ? a.swing : rng.chance(0.5) ? b.swing : (a.swing + b.swing) / 2;
  return p;
}

int hitsOf(const Pattern& p, const PatternOps& ops, Hit* out, int max) {
  int n = 0;
  for (int l = 0; l < kLanes; ++l) {
    const LaneData& lane = p.lanes[(size_t) l];
    int a = ops.kept(lane.first) ? lane.first : -1;
    int b = lane.layer != One && ops.kept(lane.second) ? lane.second : -1;
    if (a < 0) { a = b; b = -1; }
    for (int i = 0; i < lane.count; ++i) {
      const Stroke& h = lane.hits[(size_t) i];
      const int rolls = std::clamp((int) h.roll, 1, 4);
      for (int r = 0; r < rolls; ++r) {
        const double beat = warp(h.pos + r / (double) rolls, p.swing) / 4.0;
        const float v = r == 0 ? h.vel : h.vel * 0.8f;
        if (n < max) out[n++] = { beat, l, a, v, 0.0, true };
        if (b < 0 || n >= max) continue;
        if (lane.layer == Stack) out[n++] = { beat, l, b, v, 0.0, false };
        else out[n++] = { beat, l, b, v * lane.flamLevel, (double) lane.flam, false };
      }
    }
  }
  std::stable_sort(out, out + n, [](const Hit& x, const Hit& y) { return x.beat < y.beat; });
  return n;
}

namespace {
breed::Litter<Pattern> makeLitter(std::uint32_t seed, PatternOps& ops) { return breed::Litter<Pattern>(Loops::kLitter, seed, ops); }
bool validSlot(int s) { return s >= 0 && s < Loops::kSlots; }
bool validLane(int l) { return l >= 0 && l < kLanes; }
} // namespace

Loops::Loops(std::uint32_t seed) : litter_(makeLitter(seed, ops_)) {}

const Pattern& Loops::playing() const {
  return playingSlot_ >= 0 && slots_[(size_t) playingSlot_].full ? slots_[(size_t) playingSlot_].loop : litter_.heard();
}

template <class F> void Loops::change(F f) {
  if (playingSlot_ >= 0 && slots_[(size_t) playingSlot_].full) { f(slots_[(size_t) playingSlot_].loop); return; }
  Pattern p = litter_.heard();
  f(p);
  litter_.set(litter_.selected(), p);
}

void Loops::setStyle(Style s) {
  ops_.style = s;
  if (s != Style::None) ops_.swingDial = styleInfo(s).swing;
}
void Loops::setSwingDial(double s) { ops_.swingDial = clampSwing(s); }
void Loops::setPull(double p) { ops_.pull = std::clamp(p, 0.0, 1.0); }

void Loops::swapPads(int a, int b) {
  if (a < 0 || b < 0 || a >= kKitPads || b >= kKitPads || a == b) return;
  auto one = [=](std::int8_t& x) { if (x == a) x = (std::int8_t) b; else if (x == b) x = (std::int8_t) a; };
  auto fix = [&](Pattern& p) { for (auto& l : p.lanes) { one(l.first); one(l.second); } };
  for (int i = 0; i < kLitter; ++i) {
    Pattern p = litter_.candidate(i);
    fix(p);
    litter_.set(i, p);
  }
  for (Slot& s : slots_) fix(s.loop);
}

int Loops::addHit(int lane, double pos, float vel) {
  if (!validLane(lane)) return -1;
  int at = -1;
  change([&](Pattern& p) {
    LaneData& l = p.lanes[(size_t) lane];
    if (l.find(pos) >= 0 || !l.add(pos, vel)) return;
    at = l.count - 1;
  });
  return at;
}

void Loops::moveHit(int lane, int i, double pos, float vel) {
  if (!validLane(lane)) return;
  change([=](Pattern& p) {
    LaneData& l = p.lanes[(size_t) lane];
    if (i < 0 || i >= l.count) return;
    l.hits[(size_t) i].pos = wrapBar(pos);
    l.hits[(size_t) i].vel = clampVel(vel);
  });
}

void Loops::removeHit(int lane, int i) {
  if (validLane(lane)) change([=](Pattern& p) { p.lanes[(size_t) lane].remove(i); });
}

void Loops::tidy(int lane) {
  if (validLane(lane)) change([=](Pattern& p) { p.lanes[(size_t) lane].tidy(); });
}

void Loops::setSwing(double s) { change([=](Pattern& p) { p.swing = clampSwing(s); }); }

namespace {
// The next in the lane's choices, then none, then round again.
int nextChoice(const PatternOps& ops, int lane, int current) {
  int c[kKitPads];
  const int n = ops.choices(lane, c);
  int at = n; // none
  for (int i = 0; i < n; ++i)
    if (c[i] == current) at = i;
  const int next = (at + 1) % (n + 1);
  return next == n ? -1 : c[next];
}
} // namespace

void Loops::cycleSound(int lane) {
  if (!validLane(lane)) return;
  change([&](Pattern& p) { auto& l = p.lanes[(size_t) lane]; l.first = (std::int8_t) nextChoice(ops_, lane, ops_.kept(l.first) ? l.first : -1); });
}

void Loops::cycleSecond(int lane) {
  if (!validLane(lane)) return;
  change([&](Pattern& p) {
    auto& l = p.lanes[(size_t) lane];
    l.second = (std::int8_t) nextChoice(ops_, lane, ops_.kept(l.second) ? l.second : -1);
    if (l.second >= 0 && l.layer == One) l.layer = Stack;
  });
}

void Loops::setSound(int lane, int which, int pad) {
  if (!validLane(lane) || pad < -1 || pad >= kKitPads) return;
  change([=](Pattern& p) {
    auto& l = p.lanes[(size_t) lane];
    if (which == 0) l.first = (std::int8_t) pad;
    else {
      l.second = (std::int8_t) pad;
      if (pad >= 0 && l.layer == One) l.layer = Stack;
    }
  });
}

// One, Stack, Flam. With no second sound, Flam flams the first with itself.
void Loops::cycleLayer(int lane) {
  if (!validLane(lane)) return;
  change([=](Pattern& p) {
    auto& l = p.lanes[(size_t) lane];
    if (l.second < 0) {
      if (l.first < 0) return;
      l.second = l.first;
      l.layer = Flam;
      return;
    }
    l.layer = (std::uint8_t) ((l.layer + 1) % 3);
  });
}

void Loops::setFlam(int lane, double ms) {
  if (validLane(lane)) change([=](Pattern& p) { p.lanes[(size_t) lane].flam = (float) std::clamp(ms, kFlamMin, kFlamMax); });
}

void Loops::keep(int tile, int s) {
  if (!validSlot(s) || tile < 0 || tile >= kLitter) return;
  slots_[(size_t) s] = { true, litter_.candidate(tile) };
}

void Loops::swapSlots(int a, int b) {
  if (!validSlot(a) || !validSlot(b)) return;
  std::swap(slots_[(size_t) a], slots_[(size_t) b]);
  if (playingSlot_ == a) playingSlot_ = b;
  else if (playingSlot_ == b) playingSlot_ = a;
}

void Loops::clearSlot(int s) {
  if (!validSlot(s)) return;
  slots_[(size_t) s] = Slot{};
  if (playingSlot_ == s) playingSlot_ = -1;
}

void Loops::playSlot(int s) { if (validSlot(s) && slots_[(size_t) s].full) playingSlot_ = s; }

void Loops::breedFromSlot(int s) {
  if (!validSlot(s) || !slots_[(size_t) s].full) return;
  const int at = litter_.selected();
  litter_.set(at, slots_[(size_t) s].loop);
  litter_.breed(at, ops_);
  playingSlot_ = -1;
}

namespace {
// <swing>, then per lane: <first> <second> <layer> <flam ms> <flam level> <count> and <pos vel roll> per hit.
void writePattern(std::ostream& out, const Pattern& p) {
  out << p.swing;
  for (const LaneData& l : p.lanes) {
    out << "  " << (int) l.first << ' ' << (int) l.second << ' ' << (int) l.layer << ' ' << l.flam << ' ' << l.flamLevel << ' ' << l.count;
    for (int i = 0; i < l.count; ++i) out << ' ' << l.hits[(size_t) i].pos << ' ' << l.hits[(size_t) i].vel << ' ' << (int) l.hits[(size_t) i].roll;
  }
  out << '\n';
}
bool readPattern(std::istream& in, Pattern& p) {
  p = Pattern{};
  if (!(in >> p.swing) || !std::isfinite(p.swing)) return false;
  p.swing = clampSwing(p.swing);
  for (LaneData& l : p.lanes) {
    int first = -1, second = -1, layer = 0, count = 0;
    float flam = 20.0f, level = 0.6f;
    if (!(in >> first >> second >> layer >> flam >> level >> count)) return false;
    if (count < 0 || count > kMaxLaneHits || layer < 0 || layer > 2 || !std::isfinite(flam) || !std::isfinite(level)) return false;
    l.first = (std::int8_t) std::clamp(first, -1, kKitPads - 1);
    l.second = (std::int8_t) std::clamp(second, -1, kKitPads - 1);
    l.layer = (std::uint8_t) layer;
    l.flam = (float) std::clamp((double) flam, kFlamMin, kFlamMax);
    l.flamLevel = (float) std::clamp((double) level, kFlamLevelMin, kFlamLevelMax);
    for (int i = 0; i < count; ++i) {
      double pos = 0;
      float vel = 0;
      int roll = 1;
      if (!(in >> pos >> vel >> roll) || !std::isfinite(pos) || !std::isfinite(vel)) return false;
      l.add(pos, vel, (std::uint8_t) std::clamp(roll, 1, 4));
    }
  }
  return true;
}
} // namespace

// loops <style> <swing dial> <pull> <4 locks as 0s and 1s> <playing slot>
// litter <rng> <selected> <parent> <newInARow> <generation> <spread>
// then four candidates, then eight slots as <full> and a pattern.
void Loops::save(std::ostream& out) const {
  const auto st = litter_.state();
  out << "loops " << (int) ops_.style << ' ' << ops_.swingDial << ' ' << ops_.pull << ' ';
  for (bool b : ops_.locks) out << (b ? '1' : '0');
  out << ' ' << playingSlot_ << "\nlitter " << st.rng << ' ' << st.selected << ' ' << st.parent << ' ' << st.newCount << ' '
      << st.generation << ' ' << st.spread << '\n';
  for (const Pattern& p : st.candidates) writePattern(out, p);
  for (const Slot& s : slots_) {
    out << (s.full ? 1 : 0) << ' ';
    writePattern(out, s.loop);
  }
}

bool Loops::load(std::istream& in) {
  std::string word, locks;
  int style = 0, playing = -1;
  PatternOps ops = ops_;
  if (!(in >> word >> style >> ops.swingDial >> ops.pull >> locks >> playing) || word != "loops") return false;
  if (style < 0 || style >= kNumStyles || !std::isfinite(ops.swingDial) || !std::isfinite(ops.pull) || locks.size() != (size_t) kLanes)
    return false;
  ops.style = (Style) style;
  ops.swingDial = clampSwing(ops.swingDial);
  ops.pull = std::clamp(ops.pull, 0.0, 1.0);
  for (int l = 0; l < kLanes; ++l) ops.locks[(size_t) l] = locks[(size_t) l] == '1';
  breed::Litter<Pattern>::State st;
  if (!(in >> word >> st.rng >> st.selected >> st.parent >> st.newCount >> st.generation >> st.spread) || word != "litter") return false;
  st.candidates.resize(kLitter);
  for (Pattern& p : st.candidates) if (!readPattern(in, p)) return false;
  std::array<Slot, kSlots> slots{};
  for (Slot& s : slots) {
    int full = 0;
    if (!(in >> full) || !readPattern(in, s.loop)) return false;
    s.full = full == 1;
  }
  breed::Litter<Pattern> litter = litter_;
  if (!litter.restore(st)) return false;
  litter_ = litter;
  ops_ = ops;
  slots_ = slots;
  playingSlot_ = validSlot(playing) && slots_[(size_t) playing].full ? playing : -1;
  return true;
}

} // namespace bb2
