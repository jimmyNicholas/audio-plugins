// Offline renders for listening without a DAW, through the same Breeder, Engine and Riff the plugin uses.
//
//   synth_render <out dir> [seed]
//
// writes, at 48 kHz 16-bit stereo:
//   litter.wav   each riff pattern over a fresh litter in each zone: one bar per candidate, A to H
//   breed.wav    a scripted session on the bass riff: breed from C, then E, cross with B, breed again
#include "breeder.h"
#include "engine.h"
#include "riff.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using namespace synth;

namespace {

constexpr double kRate = 48000.0, kBpm = 92.0;

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

// Plays the riff over the heard candidate for `bars` bars.
void play(Engine& e, Riff& riff, const Breeder& b, Stereo& out, double bars) {
  const size_t n = static_cast<size_t>(bars * 4 * 60.0 / kBpm * kRate);
  const size_t start = out.l.size();
  out.l.resize(start + n);
  out.r.resize(start + n);
  std::vector<Riff::Event> events;
  for (size_t i = 0; i < n; i += 256) {
    const int block = (int) std::min<size_t>(256, n - i);
    events.clear();
    riff.advance(block, kBpm, events);
    int done = 0;
    for (const auto& ev : events) {
      if (ev.offset > done) { e.render(b.heard(), out.l.data() + start + i + done, out.r.data() + start + i + done, ev.offset - done); done = ev.offset; }
      if (ev.on) e.noteOn(ev.note, ev.velocity, b.heard());
      else e.noteOff(ev.note);
    }
    if (block > done) e.render(b.heard(), out.l.data() + start + i + done, out.r.data() + start + i + done, block - done);
  }
}

} // namespace

int main(int argc, char** argv) {
  if (argc < 2) { std::fprintf(stderr, "usage: %s <out dir> [seed]\n", argv[0]); return 2; }
  const std::string dir = argv[1];
  const std::uint32_t seed = argc > 2 ? (std::uint32_t) std::strtoul(argv[2], nullptr, 10) : 1;
  bool ok = true;

  { // litter.wav: Bass riff in Bass, Chords in Keys, Arp in Lead.
    Stereo out;
    const Zone zones[Riff::kPatterns] = { Zone::Bass, Zone::Keys, Zone::Lead };
    for (int pattern = 0; pattern < Riff::kPatterns; ++pattern) {
      Breeder b(seed + (std::uint32_t) pattern);
      b.setZone(zones[pattern]);
      b.fresh();
      Engine e;
      e.prepare(kRate);
      Riff riff;
      riff.prepare(kRate);
      riff.setPattern(pattern);
      riff.setOn(true);
      for (int k = 0; k < Breeder::kLitter; ++k) { b.select(k); play(e, riff, b, out, 1.0); }
      riff.setOn(false);
      play(e, riff, b, out, 1.0);
    }
    ok = writeWav(dir + "/litter.wav", out) && ok;
  }

  { // breed.wav
    Breeder b(seed);
    b.setZone(Zone::Bass);
    b.fresh();
    Engine e;
    e.prepare(kRate);
    Riff riff;
    riff.prepare(kRate);
    riff.setOn(true);
    Stereo out;
    b.select(2); play(e, riff, b, out, 1);
    b.breedFrom(2);
    for (int k : { 0, 4 }) { b.select(k); play(e, riff, b, out, 1); }
    b.breedFrom(4);
    b.select(1); play(e, riff, b, out, 1);
    b.cross(4, 1);
    for (int k : { 0, 5 }) { b.select(k); play(e, riff, b, out, 1); }
    b.breedFrom(5);
    b.select(5); play(e, riff, b, out, 2);
    riff.setOn(false);
    play(e, riff, b, out, 1);
    ok = writeWav(dir + "/breed.wav", out) && ok;
  }

  if (!ok) { std::fprintf(stderr, "could not write to %s\n", dir.c_str()); return 1; }
  std::printf("wrote litter.wav and breed.wav to %s\n", dir.c_str());
  return 0;
}
