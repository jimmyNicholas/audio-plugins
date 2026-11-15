#include "studio.h"
#include <iomanip>
#include <locale>
#include <sstream>

namespace bs2 {

Studio::Studio(std::uint32_t seed) : sounds(seed), loops(seed * 2654435761U + 7) { sync(); }

void Studio::sync() {
  std::array<int, kKitPads> zones{};
  for (int p = 0; p < kKitPads; ++p) zones[(size_t) p] = sounds.pad(p).full ? (int) sounds.pad(p).zone : -1;
  loops.setPads(zones);
}

void Studio::keepSound(int tile, int pad) { sounds.keep(tile, pad); sync(); }

void Studio::swapPads(int a, int b) {
  if (a == b) return;
  sounds.swapPads(a, b);
  loops.swapPadIndex(a, b);
  sync();
}

void Studio::clearPad(int pad) { sounds.clearPad(pad); sync(); }

// bloodline-synth-2 1, then the sounds, then the loops.
std::string Studio::save() const {
  std::ostringstream out;
  out.imbue(std::locale::classic());
  out << std::setprecision(17) << "bloodline-synth-2 1\n";
  sounds.save(out);
  loops.save(out);
  return out.str();
}

bool Studio::load(const std::string& text) {
  std::istringstream in(text);
  in.imbue(std::locale::classic());
  std::string word;
  int version = 0;
  if (!(in >> word >> version) || word != "bloodline-synth-2" || version != 1) return false;
  Sounds s = sounds;
  Loops l = loops;
  if (!s.load(in) || !l.load(in)) return false;
  sounds = s;
  loops = l;
  sync();
  return true;
}

} // namespace bs2
