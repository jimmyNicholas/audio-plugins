#include "instrument.h"
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
namespace bloodline {
// Grid rows from the bottom: 0 is the bottom row (36 to 39), 3 is row 1 (48 to 51).
Pad padFor(int note) {
  Pad p;
  const int i = note - kFirstNote;
  if (i < 0 || i >= kNumPads) return p;
  const int gridRow = i / 4, col = i % 4;
  if (gridRow == 0) {
    if (col == 3) { p.kind = Pad::Empty; return p; }
    p.kind = Pad::Bottom;
    p.row = 2 - col; // 36 plays row 3, 37 row 2, 38 row 1
    return p;
  }
  p.row = 3 - gridRow;
  if (col == 3) p.kind = Pad::Choose;
  else { p.kind = Pad::Candidate; p.pad = col; }
  return p;
}
int candidateNote(int row, int pad) { return kFirstNote + (3 - row) * 4 + pad; }
int chooseNote(int row) { return candidateNote(row, 3); }
int bottomNote(int row) { return kFirstNote + (2 - row); }
Instrument::Instrument(std::uint32_t seed) : rows_{ Row(seed, Zone::Hats), Row(seed + 1, Zone::Snare), Row(seed + 2, Zone::Kick) } {}
void Instrument::prepare(double sr) { for (auto& v : voices_) v.prepare(sr); }
void Instrument::noteOn(int note, float velocity) {
  const Pad p = padFor(note);
  if (p.kind == Pad::None || p.kind == Pad::Empty) return;
  Voice& voice = voices_[note - kFirstNote];
  switch (p.kind) {
    case Pad::Candidate:
      voice.trigger(rows_[p.row].press(p.pad), velocity);
      break;
    case Pad::Choose:
      if (rows_[p.row].choose()) voice.trigger(rows_[p.row].chosen(), velocity);
      break;
    case Pad::Bottom:
      if (rows_[p.row].hasChosen()) voice.trigger(rows_[p.row].chosen(), velocity);
      break;
    default:
      break;
  }
}

void Instrument::render(float* out, int n) {
  for (int i = 0; i < n; ++i) out[i] = 0.0f;
  for (auto& v : voices_) if (v.active()) v.render(out, n);
}
// One header line, then one line per row:
//   row <seed> <zone> <favourite> <last pressed> <spread> <new in a row> <locks as 0s and 1s> <24 values>
// Numbers in the C locale, doubles to 17 significant digits so they come back exactly.
std::string Instrument::save() const {
  std::ostringstream out;
  out.imbue(std::locale::classic());
  out << std::setprecision(17) << "bloodline 1\n";
  for (const Row& r : rows_) {
    out << "row " << r.seed() << ' ' << static_cast<int>(r.zone()) << ' ' << r.favourite() << ' '
        << r.lastPressed() << ' ' << r.spread() << ' ' << r.newInARow() << ' ';
    for (int i = 0; i < kNumSettings; ++i) out << (r.locked(i) ? '1' : '0');
    for (int pad = 0; pad < Row::kPads; ++pad)
      for (double v : r.candidate(pad)) out << ' ' << v;
    out << '\n';
  }
  return out.str();
}

bool Instrument::load(const std::string& text) {
  std::istringstream in(text);
  in.imbue(std::locale::classic());
  std::string word;
  int version = 0;
  if (!(in >> word >> version) || word != "bloodline" || version != 1) return false;

  std::array<Row, kNumRows> rows = rows_;
  for (Row& row : rows) {
    std::uint32_t seed = 0;
    int zone = 0, favourite = -1, last = -1, newCount = 0;
    double spread = 0;
    std::string lockBits;
    if (!(in >> word >> seed >> zone >> favourite >> last >> spread >> newCount >> lockBits)) return false;
    if (word != "row" || zone < 0 || zone >= kNumZones || lockBits.size() != (size_t) kNumSettings) return false;
    std::array<bool, kNumSettings> locks{};
    for (int i = 0; i < kNumSettings; ++i) {
      if (lockBits[i] != '0' && lockBits[i] != '1') return false;
      locks[i] = lockBits[i] == '1';
    }
    std::array<Values, Row::kPads> pads{};
    for (auto& pad : pads)
      for (double& v : pad)
        if (!(in >> v) || !std::isfinite(v)) return false;
    row.restore(seed, static_cast<Zone>(zone), pads, favourite, last, spread, newCount, locks);
  }
  rows_ = rows;
  return true;
}
}
