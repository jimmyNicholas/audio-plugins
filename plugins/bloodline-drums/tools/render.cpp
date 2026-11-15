// Offline renders for listening without a DAW, made through the same Instrument the plugin uses.
//
//   bloodline_render <out dir> [seed]
//
// writes, at 48 kHz 16-bit stereo:
//   zones.wav       eight fresh sounds from each zone in turn, half a second apart: Kick, Snare, Hats, then None
//   candidates.wav  a fresh workbench's A, B and C in each zone, Kick first
//   beat.wav        a scripted session: breed a kick, a snare and hats on the workbench, place them in
//                   slots 1 to 3, then two bars of boom bap at 90 BPM, the hats on Random
#include "instrument.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using namespace bloodline;

namespace {

constexpr double kRate = 48000.0;

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

// Appends `seconds` of the instrument's output.
void run(Instrument& inst, Stereo& out, double seconds) {
  const size_t n = static_cast<size_t>(seconds * kRate), start = out.l.size();
  out.l.resize(start + n);
  out.r.resize(start + n);
  for (size_t i = 0; i < n; i += 256)
    inst.render(out.l.data() + start + i, out.r.data() + start + i, (int) std::min<size_t>(256, n - i));
}

// Breeds the workbench in a zone by a fixed script: hear each pad, then choose it.
void breed(Instrument& inst, Stereo& out, Zone zone) {
  inst.setZone(kBench, zone);
  inst.reset(kBench); // fresh candidates from the zone
  for (int pad : { 0, 1, 1, 2, 2, 0, 0 }) {
    inst.noteOn(noteFor(kBench, pad), 0.9f);
    run(inst, out, 0.35);
    inst.noteOn(noteFor(kBench, 3), 0.9f);
    run(inst, out, 0.35);
  }
}

} // namespace

int main(int argc, char** argv) {
  if (argc < 2) { std::fprintf(stderr, "usage: %s <out dir> [seed]\n", argv[0]); return 2; }
  const std::string dir = argv[1];
  const std::uint32_t seed = argc > 2 ? (std::uint32_t) std::strtoul(argv[2], nullptr, 10) : 1;
  bool ok = true;

  { // zones.wav: a fresh row's first candidate in each zone, straight from Row and Voice.
    Stereo out;
    for (Zone z : { Zone::Kick, Zone::Snare, Zone::Hats, Zone::None }) {
      for (std::uint32_t k = 0; k < 8; ++k) {
        Row row(seed * 1000 + k, z);
        Voice voice;
        voice.prepare(kRate);
        voice.trigger(row.candidate(0), 0.9f, row.padPan(0), row.padLevel(0));
        std::vector<float> l(static_cast<size_t>(0.5 * kRate), 0.0f), r = l;
        voice.render(l.data(), r.data(), (int) l.size());
        out.l.insert(out.l.end(), l.begin(), l.end());
        out.r.insert(out.r.end(), r.begin(), r.end());
      }
      out.l.insert(out.l.end(), static_cast<size_t>(0.5 * kRate), 0.0f);
      out.r.insert(out.r.end(), static_cast<size_t>(0.5 * kRate), 0.0f);
    }
    ok = writeWav(dir + "/zones.wav", out) && ok;
  }

  { // candidates.wav
    Instrument inst(seed);
    inst.prepare(kRate);
    Stereo out;
    for (Zone z : { Zone::Kick, Zone::Snare, Zone::Hats }) {
      inst.setZone(kBench, z);
      inst.reset(kBench);
      for (int pad = 0; pad < 3; ++pad) { inst.noteOn(noteFor(kBench, pad), 0.9f); run(inst, out, 0.6); }
    }
    ok = writeWav(dir + "/candidates.wav", out) && ok;
  }

  { // beat.wav
    Instrument inst(seed);
    inst.prepare(kRate);
    Stereo out;
    const Zone zones[kNumSlots] = { Zone::Kick, Zone::Snare, Zone::Hats };
    for (int line = 1; line <= kNumSlots; ++line) {
      breed(inst, out, zones[line - 1]);
      inst.place(line);
    }
    run(inst, out, 1.0);

    const double sixteenth = 60.0 / 90.0 / 4.0;
    const char* kick = "x.....x...x.....";
    const char* snare = "....x.......x...";
    for (int bar = 0; bar < 2; ++bar)
      for (int i = 0; i < 16; ++i) {
        if (kick[i] == 'x') inst.noteOn(noteFor(1, 0), 1.0f);
        if (snare[i] == 'x') inst.noteOn(noteFor(2, 0), 0.95f);
        if (i % 2 == 0) inst.noteOn(noteFor(3, 3), i % 4 == 0 ? 0.8f : 0.55f); // Random
        run(inst, out, sixteenth);
      }
    run(inst, out, 1.5);
    ok = writeWav(dir + "/beat.wav", out) && ok;
  }

  if (!ok) { std::fprintf(stderr, "could not write to %s\n", dir.c_str()); return 1; }
  std::printf("wrote zones.wav, candidates.wav and beat.wav to %s\n", dir.c_str());
  return 0;
}
