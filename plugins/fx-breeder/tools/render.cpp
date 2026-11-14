// Offline renders for listening without a DAW, through the same Breeder, Rack and Player the plugin uses.
//
//   fx_render <out dir> [seed]
//
// writes, at 48 kHz 16-bit stereo, all fed by the built-in drum loop at 90 BPM:
//   dry.wav      the loop on its own
//   litter.wav   a fresh litter in each character, two bars per chain, A to F
//   breed.wav    a scripted session: breed from B, then D, cross with A, then Wild for two rounds
// and prints each chain, so you can tell what you are hearing.
#include "breeder.h"
#include "loop.h"
#include "rack.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using namespace fx;

namespace {

constexpr double kRate = 48000.0, kBpm = 90.0;
constexpr int kBlock = 512;

struct Stereo { std::vector<float> l, r; };

bool writeWav(const std::string& path, const Stereo& x) {
  FILE* f = std::fopen(path.c_str(), "wb");
  if (!f) return false;
  auto u32 = [f](std::uint32_t v) { std::fwrite(&v, 4, 1, f); };
  auto u16 = [f](std::uint16_t v) { std::fwrite(&v, 2, 1, f); };
  const std::uint32_t bytes = static_cast<std::uint32_t>(x.l.size() * 4);
  std::fwrite("RIFF", 1, 4, f); u32(36 + bytes); std::fwrite("WAVEfmt ", 1, 8, f);
  u32(16); u16(1); u16(2); u32((std::uint32_t) kRate); u32((std::uint32_t) kRate * 4); u16(4); u16(16);
  std::fwrite("data", 1, 4, f); u32(bytes);
  auto pcm = [&](float s) { u16(static_cast<std::uint16_t>(static_cast<std::int16_t>(std::clamp(s, -1.0f, 1.0f) * 32767.0f))); };
  for (size_t i = 0; i < x.l.size(); ++i) { pcm(x.l[i]); pcm(x.r[i]); }
  return std::fclose(f) == 0;
}

void describe(char letter, const Chain& c) {
  std::printf("  %c: ", letter);
  for (int i = 0; i < c.count; ++i) std::printf("%s%s", i ? " > " : "", info(c.units[i].type).name);
  std::printf("  (mix %.0f%%)\n", c.mix * 100);
}

struct Session {
  Loop loop;
  Player player;
  Stereo out;
  Session() { loop.prepare(kRate); player.prepare(kRate, kBlock); }
  void hear(const Chain& c) { player.use(std::make_unique<Rack>(c, kRate, kBlock)); }
  void bars(double n) {
    const size_t len = static_cast<size_t>(n * 4 * 60.0 / kBpm * kRate), start = out.l.size();
    out.l.resize(start + len);
    out.r.resize(start + len);
    for (size_t i = 0; i < len; i += kBlock) {
      const int b = (int) std::min<size_t>(kBlock, len - i);
      loop.render(out.l.data() + start + i, out.r.data() + start + i, b, kBpm);
      player.process(out.l.data() + start + i, out.r.data() + start + i, b);
    }
  }
};

} // namespace

int main(int argc, char** argv) {
  if (argc < 2) { std::fprintf(stderr, "usage: %s <out dir> [seed]\n", argv[0]); return 2; }
  const std::string dir = argv[1];
  const std::uint32_t seed = argc > 2 ? (std::uint32_t) std::strtoul(argv[2], nullptr, 10) : 1;
  bool ok = true;

  { Session s; s.player.setBypass(true); s.bars(2); ok = writeWav(dir + "/dry.wav", s.out) && ok; }

  {
    Session s;
    for (Character ch : { Character::Space, Character::Dirt, Character::Motion, Character::Glitch }) {
      Breeder b(seed + static_cast<std::uint32_t>(ch));
      b.setCharacter(ch);
      b.fresh();
      std::printf("litter.wav, %s:\n", characterName(ch));
      for (int k = 0; k < Breeder::kLitter; ++k) {
        describe((char) ('A' + k), b.litter().candidate(k));
        s.hear(b.litter().candidate(k));
        s.bars(2);
      }
    }
    ok = writeWav(dir + "/litter.wav", s.out) && ok;
  }

  {
    Session s;
    Breeder b(seed);
    std::printf("breed.wav:\n");
    auto hear = [&](int k, double bars) { b.select(k); describe((char) ('A' + k), b.heard()); s.hear(b.heard()); s.bars(bars); };
    hear(1, 2);
    b.breedFrom(1);
    hear(3, 2);
    b.breedFrom(3);
    hear(0, 2);
    b.cross(3, 0);
    hear(2, 2);
    b.setWild(true);
    b.breedFrom(2);
    hear(4, 2);
    b.breedFrom(4);
    hear(5, 2);
    ok = writeWav(dir + "/breed.wav", s.out) && ok;
  }

  if (!ok) { std::fprintf(stderr, "could not write to %s\n", dir.c_str()); return 1; }
  std::printf("wrote dry.wav, litter.wav and breed.wav to %s\n", dir.c_str());
  return 0;
}
