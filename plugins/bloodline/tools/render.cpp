// Offline renders for listening without a DAW, made through the same Instrument the plugin uses.
//
//   bloodline_render <out dir> [seed]
//
// writes, at 48 kHz 16-bit mono:
//   zones.wav       eight fresh sounds from each zone in turn, half a second apart: Kick, Snare, Hats, then None
//   candidates.wav  each row's starting A, B and C, row 3 (Kick) first
//   beat.wav        a scripted breeding session in every row, then two bars of boom bap at 90 BPM
//                   on the bottom pads (36 kick, 37 snare, 38 hats)
#include "instrument.h"
#include <cstdint>
#include <cstdio>
#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

using namespace bloodline;

namespace {

constexpr double kRate = 48000.0;

bool writeWav(const std::string& path, const std::vector<float>& x) {
  FILE* f = std::fopen(path.c_str(), "wb");
  if (!f) return false;
  auto u32 = [f](std::uint32_t v) { std::fwrite(&v, 4, 1, f); };
  auto u16 = [f](std::uint16_t v) { std::fwrite(&v, 2, 1, f); };
  const std::uint32_t bytes = static_cast<std::uint32_t>(x.size() * 2);
  std::fwrite("RIFF", 1, 4, f); u32(36 + bytes); std::fwrite("WAVEfmt ", 1, 8, f);
  u32(16); u16(1); u16(1); u32((std::uint32_t) kRate); u32((std::uint32_t) kRate * 2); u16(2); u16(16);
  std::fwrite("data", 1, 4, f); u32(bytes);
  for (float s : x) {
    const float c = s < -1.0f ? -1.0f : (s > 1.0f ? 1.0f : s);
    u16(static_cast<std::uint16_t>(static_cast<std::int16_t>(c * 32767.0f)));
  }
  return std::fclose(f) == 0;
}

// Appends `seconds` of the instrument's output.
void run(Instrument& inst, std::vector<float>& out, double seconds) {
  const size_t n = static_cast<size_t>(seconds * kRate), start = out.size();
  out.resize(start + n);
  for (size_t i = 0; i < n; i += 256) inst.render(out.data() + start + i, (int) std::min<size_t>(256, n - i));
}

} // namespace

int main(int argc, char** argv) {
  if (argc < 2) { std::fprintf(stderr, "usage: %s <out dir> [seed]\n", argv[0]); return 2; }
  const std::string dir = argv[1];
  const std::uint32_t seed = argc > 2 ? (std::uint32_t) std::strtoul(argv[2], nullptr, 10) : 1;
  bool ok = true;

  { // zones.wav: a fresh row's first candidate, in each zone. Straight from Row and Voice, since an
    // Instrument's rows keep their candidates when the zone changes.
    std::vector<float> out;
    for (Zone z : { Zone::Kick, Zone::Snare, Zone::Hats, Zone::None }) {
      for (std::uint32_t k = 0; k < 8; ++k) {
        Row row(seed * 1000 + k, z);
        Voice voice;
        voice.prepare(kRate);
        voice.trigger(row.candidate(0), 0.9f);
        std::vector<float> hit(static_cast<size_t>(0.5 * kRate), 0.0f);
        voice.render(hit.data(), (int) hit.size());
        out.insert(out.end(), hit.begin(), hit.end());
      }
      out.insert(out.end(), static_cast<size_t>(0.5 * kRate), 0.0f);
    }
    ok = writeWav(dir + "/zones.wav", out) && ok;
  }

  Instrument inst(seed);
  inst.prepare(kRate);

  { // candidates.wav
    std::vector<float> out;
    for (int row : { 2, 1, 0 })
      for (int pad = 0; pad < 3; ++pad) { inst.noteOn(candidateNote(row, pad), 0.9f); run(inst, out, 0.6); }
    ok = writeWav(dir + "/candidates.wav", out) && ok;
  }

  { // beat.wav: breed each row by a fixed script, then play the bottom pads.
    std::vector<float> out;
    const int script[] = { 0, 1, 1, 2, 2, 0, 0 };
    for (int row : { 2, 1, 0 })
      for (int pad : script) {
        inst.noteOn(candidateNote(row, pad), 0.9f); // hear it, then choose it
        run(inst, out, 0.35);
        inst.noteOn(chooseNote(row), 0.9f);
        run(inst, out, 0.35);
      }
    run(inst, out, 1.0);

    const double sixteenth = 60.0 / 90.0 / 4.0;
    const char* kick = "x.....x...x.....";
    const char* snare = "....x.......x...";
    for (int bar = 0; bar < 2; ++bar)
      for (int i = 0; i < 16; ++i) {
        if (kick[i] == 'x') inst.noteOn(bottomNote(2), 1.0f);
        if (snare[i] == 'x') inst.noteOn(bottomNote(1), 0.95f);
        if (i % 2 == 0) inst.noteOn(bottomNote(0), i % 4 == 0 ? 0.8f : 0.55f);
        run(inst, out, sixteenth);
      }
    run(inst, out, 1.5);
    ok = writeWav(dir + "/beat.wav", out) && ok;
  }

  if (!ok) { std::fprintf(stderr, "could not write to %s\n", dir.c_str()); return 1; }
  std::printf("wrote zones.wav, candidates.wav and beat.wav to %s\n", dir.c_str());
  return 0;
}
