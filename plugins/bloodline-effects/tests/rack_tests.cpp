// The Rack seam: audio in, through a chain and the Player's limiter, audio out.
#include "../../shared/test/check.h"
#include "breeder.h"
#include "loop.h"
#include "rack.h"
#include <algorithm>
#include <vector>

using namespace fx;

constexpr double kRate = 48000.0;
constexpr int kBlock = 256;

struct Stereo { std::vector<float> l, r; };

static Stereo loopOf(double seconds) {
  Loop loop;
  loop.prepare(kRate);
  Stereo x{ std::vector<float>(static_cast<size_t>(seconds * kRate)), {} };
  x.r = x.l;
  loop.render(x.l.data(), x.r.data(), (int) x.l.size(), 90.0);
  return x;
}

static void run(Player& p, Stereo& x) {
  for (size_t i = 0; i < x.l.size(); i += kBlock)
    p.process(x.l.data() + i, x.r.data() + i, (int) std::min<size_t>(kBlock, x.l.size() - i));
}

static bool bounded(const Stereo& x) {
  for (size_t i = 0; i < x.l.size(); ++i)
    if (!std::isfinite(x.l[i]) || !std::isfinite(x.r[i]) || std::fabs(x.l[i]) > 1.0f || std::fabs(x.r[i]) > 1.0f) return false;
  return true;
}

static double peak(const std::vector<float>& x, size_t from = 0) {
  double m = 0;
  for (size_t i = from; i < x.size(); ++i) m = std::max(m, (double) std::fabs(x[i]));
  return m;
}

TEST("the loop plays: kicks, snares and hats at a sensible level") {
  const Stereo x = loopOf(2.0);
  CHECK(peak(x.l) > 0.3 && peak(x.l) < 1.0);
  CHECK(x.l == x.r);
}

TEST("every effect, at its extremes and anywhere between, stays finite and within full scale after the limiter") {
  breed::Rng rng(21);
  for (int t = 0; t < kNumTypes; ++t)
    for (int trial = 0; trial < 6; ++trial) {
      Chain c;
      c.count = 1;
      c.units[0].type = static_cast<Type>(t);
      for (double& v : c.units[0].p) v = trial == 0 ? 0.0 : trial == 1 ? 1.0 : rng.uniform();
      c.mix = 1.0;
      Player p;
      p.prepare(kRate, kBlock);
      p.use(std::make_unique<Rack>(c, kRate, kBlock));
      Stereo x = loopOf(1.5);
      for (float& s : x.l) s *= 2.0f; // hot input
      run(p, x);
      CHECK(bounded(x));
    }
}

TEST("random chains, Wild and fresh, stay finite and within full scale") {
  ChainOps ops;
  ops.wild = true;
  breed::Rng rng(22);
  for (int trial = 0; trial < 25; ++trial) {
    Chain c = ops.fresh(rng);
    for (int k = 0; k < 3; ++k) c = ops.mutate(c, 0.5, rng);
    Player p;
    p.prepare(kRate, kBlock);
    p.use(std::make_unique<Rack>(c, kRate, kBlock));
    Stereo x = loopOf(1.0);
    run(p, x);
    CHECK(bounded(x));
  }
}

TEST("mix 0 passes the input through, give or take the DC blocker") {
  Chain c;
  c.count = 1;
  c.units[0].type = Type::Ring;
  c.units[0].p = { 0.5, 0.5, 0.5, 1.0 };
  c.mix = 0.0;
  Player p;
  p.prepare(kRate, kBlock);
  p.use(std::make_unique<Rack>(c, kRate, kBlock));
  const Stereo in = loopOf(1.0);
  Stereo x = in;
  run(p, x);
  double worst = 0;
  for (size_t i = 4800; i < x.l.size(); ++i) worst = std::max(worst, (double) std::fabs(x.l[i] - in.l[i]));
  CHECK(worst < 0.05);
}

TEST("bypass fades to the untouched input") {
  Chain c;
  c.count = 1;
  c.units[0].type = Type::Gate;
  c.units[0].p = { 0.8, 0.2, 1.0, 0.0 };
  Player p;
  p.prepare(kRate, kBlock);
  p.use(std::make_unique<Rack>(c, kRate, kBlock));
  p.setBypass(true);
  const Stereo in = loopOf(1.0);
  Stereo x = in;
  run(p, x);
  double worst = 0;
  for (size_t i = 4800; i < x.l.size(); ++i) worst = std::max(worst, (double) std::fabs(x.l[i] - in.l[i]));
  CHECK(worst < 0.05);
}

TEST("switching racks mid-stream stays within full scale, and hands back only the rack before last") {
  ChainOps ops;
  breed::Rng rng(23);
  Player p;
  p.prepare(kRate, kBlock);
  CHECK(p.use(std::make_unique<Rack>(ops.fresh(rng), kRate, kBlock)) == nullptr);
  Stereo x = loopOf(1.0);
  for (size_t i = 0, block = 0; i < x.l.size(); i += kBlock, ++block) {
    if (block == 90) CHECK(p.use(std::make_unique<Rack>(ops.fresh(rng), kRate, kBlock)) == nullptr); // the first now fades
    p.process(x.l.data() + i, x.r.data() + i, (int) std::min<size_t>(kBlock, x.l.size() - i));
  }
  CHECK(bounded(x));
  CHECK(p.use(std::make_unique<Rack>(ops.fresh(rng), kRate, kBlock)) != nullptr); // the first comes back to be deleted
}

TEST("new settings on the same effects carry over without a new rack; a different shape is refused") {
  ChainOps ops;
  breed::Rng rng(24);
  const Chain c = ops.fresh(rng);
  Player p;
  p.prepare(kRate, kBlock);
  p.use(std::make_unique<Rack>(c, kRate, kBlock));
  Chain tweak = c;
  tweak.units[0].p[0] = 0.01;
  CHECK(p.setSettings(tweak));
  CHECK_EQ(p.current()->chain().units[0].p[0], 0.01);
  Chain other = c;
  other.units[0].type = c.units[0].type == Type::Drive ? Type::Crush : Type::Drive;
  CHECK(!p.setSettings(other));
}

TEST("replace plays the new rack at once and drops both the current and the fading one") {
  ChainOps ops;
  breed::Rng rng(26);
  Player p;
  p.prepare(kRate, kBlock);
  p.use(std::make_unique<Rack>(ops.fresh(rng), kRate, kBlock));
  p.use(std::make_unique<Rack>(ops.fresh(rng), kRate, kBlock));
  const Chain c = ops.fresh(rng);
  auto dropped = p.replace(std::make_unique<Rack>(c, kRate, kBlock));
  CHECK(dropped[0] != nullptr && dropped[1] != nullptr);
  CHECK(p.current()->chain() == c);
  CHECK(p.use(std::make_unique<Rack>(ops.fresh(rng), kRate, kBlock)) == nullptr); // nothing was fading
}

TEST("a blocksize bigger than prepared is split, not overrun") {
  ChainOps ops;
  breed::Rng rng(25);
  Player p;
  p.prepare(kRate, 64);
  p.use(std::make_unique<Rack>(ops.fresh(rng), kRate, 64));
  Stereo x = loopOf(0.2);
  p.process(x.l.data(), x.r.data(), (int) x.l.size());
  CHECK(bounded(x));
}

CHECK_MAIN
