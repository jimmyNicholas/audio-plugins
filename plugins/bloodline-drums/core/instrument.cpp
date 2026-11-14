#include "instrument.h"
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>

namespace bloodline {

int noteFor(int line, int pad) { return kFirstNote + line * 4 + pad; }

// Each new row gets the next seed, so a fresh workbench after placing differs from the last one.
Instrument::Instrument(std::uint32_t seed) : bench_(seed, Zone::None), nextSeed_(seed + 1) {}

Row Instrument::freshRow() { return Row(nextSeed_++, Zone::None); }

void Instrument::prepare(double sr) { for (auto& v : voices_) v.prepare(sr); }

Pad Instrument::padFor(int note) const {
  Pad p;
  const int i = note - kFirstNote;
  if (i < 0 || i >= kNumPads) return p;
  p.line = i / 4;
  const int col = i % 4;
  if (p.line != kBench && !slots_[p.line - 1]) { p.kind = col == 3 ? Pad::Place : Pad::Blank; return p; }
  if (col < 3) { p.kind = Pad::Candidate; p.pad = col; return p; }
  p.kind = isOpen(p.line) ? Pad::Choose : Pad::Random;
  return p;
}

const Row* Instrument::row(int line) const {
  if (line == kBench) return &bench_;
  if (line < 1 || line > kNumSlots || !slots_[line - 1]) return nullptr;
  return &*slots_[line - 1];
}

Row* Instrument::any(int line) { return const_cast<Row*>(static_cast<const Instrument*>(this)->row(line)); }
Row* Instrument::editable(int line) { return isOpen(line) ? any(line) : nullptr; }

bool Instrument::isOpen(int line) const {
  if (line == kBench) return true;
  return line >= 1 && line <= kNumSlots && slots_[line - 1] && open_[line - 1];
}

// xorshift32, apart from the rows' own breeding, so playing Random never changes what is bred.
double Instrument::draw() {
  pick_ ^= pick_ << 13; pick_ ^= pick_ >> 17; pick_ ^= pick_ << 5;
  return pick_ / 4294967296.0;
}

void Instrument::trigger(int note, int line, int pad, float velocity) {
  const Row& r = *row(line);
  voices_[note - kFirstNote].trigger(r.candidate(pad), velocity, r.padPan(pad), r.padLevel(pad));
}

void Instrument::noteOn(int note, float velocity) {
  const Pad p = padFor(note);
  switch (p.kind) {
    case Pad::Candidate:
      any(p.line)->press(p.pad);
      trigger(note, p.line, p.pad, velocity);
      break;
    case Pad::Choose:
      if (any(p.line)->choose()) trigger(note, p.line, row(p.line)->favourite(), velocity);
      break;
    case Pad::Random:
      trigger(note, p.line, any(p.line)->random(draw()), velocity);
      break;
    case Pad::Place:
      place(p.line);
      break;
    default:
      break;
  }
}

void Instrument::render(float* left, float* right, int n) {
  for (int i = 0; i < n; ++i) left[i] = right[i] = 0.0f;
  for (auto& v : voices_) if (v.active()) v.render(left, right, n);
}

bool Instrument::place(int line) {
  if (line < 1 || line > kNumSlots || slots_[line - 1]) return false;
  slots_[line - 1] = bench_;
  open_[line - 1] = false;
  bench_ = freshRow(); // completely fresh: None, starting spread, no locks (#44)
  return true;
}

bool Instrument::setOpen(int line, bool open) {
  if (line < 1 || line > kNumSlots || !slots_[line - 1]) return false;
  open_[line - 1] = open;
  return true;
}

bool Instrument::move(int line, int by) {
  const int to = line + by;
  if (line < 1 || line > kNumSlots || to < 1 || to > kNumSlots || (by != 1 && by != -1)) return false;
  if (!slots_[line - 1]) return false; // moving a blank is moving its neighbour; do that instead
  std::swap(slots_[line - 1], slots_[to - 1]);
  std::swap(open_[line - 1], open_[to - 1]);
  return true;
}

bool Instrument::reset(int line) {
  Row* r = editable(line);
  if (!r) return false;
  r->reset();
  return true;
}

bool Instrument::clear(int line) {
  if (line < 1 || line > kNumSlots || !slots_[line - 1]) return false;
  slots_[line - 1].reset();
  open_[line - 1] = false;
  return true;
}

void Instrument::startAgain() {
  for (int s = 0; s < kNumSlots; ++s) { slots_[s].reset(); open_[s] = false; }
  bench_ = freshRow();
}

bool Instrument::setZone(int line, Zone zone) {
  Row* r = editable(line);
  if (!r) return false;
  r->setZone(zone);
  return true;
}

bool Instrument::setLock(int line, int setting, bool on) {
  Row* r = editable(line);
  if (!r || setting < 0 || setting >= kNumSettings) return false;
  r->setLock(setting, on);
  return true;
}

bool Instrument::setValue(int line, int setting, double norm) {
  Row* r = editable(line);
  if (!r || r->lastPlayed() < 0 || setting < 0 || setting >= kNumSettings) return false;
  r->setValue(r->lastPlayed(), setting, norm);
  return true;
}

bool Instrument::setPadMono(int line, int pad, bool on) {
  Row* r = editable(line);
  if (!r || pad < 0 || pad >= Row::kPads) return false;
  r->setPadMono(pad, on);
  return true;
}

bool Instrument::setRowPan(int line, double pan) {
  Row* r = any(line);
  if (!r) return false;
  r->setRowPan(pan);
  return true;
}

bool Instrument::setRowLevel(int line, double db) {
  Row* r = any(line);
  if (!r) return false;
  r->setRowLevel(db);
  return true;
}

bool Instrument::setRowMono(int line, bool on) {
  Row* r = any(line);
  if (!r) return false;
  r->setRowMono(on);
  return true;
}

// A header line with the next seed, then one line for the workbench and one per slot:
//   bench <row>
//   slot blank
//   slot <open 0 or 1> <row>
// where <row> is: seed zone favourite lastPressed lastPlayed spread newInARow locks padMono rowPan
// rowLevel rowMono, then 3 x 19 values. Locks and pad Mono are written as 0s and 1s. Numbers in
// the C locale, doubles to 17 significant digits so they come back exactly.
namespace {

void writeRow(std::ostream& out, const Row& r) {
  const Row::State s = r.state();
  out << s.seed << ' ' << static_cast<int>(s.zone) << ' ' << s.favourite << ' ' << s.lastPressed << ' '
      << s.lastPlayed << ' ' << s.spread << ' ' << s.newCount << ' ';
  for (bool b : s.locks) out << (b ? '1' : '0');
  out << ' ';
  for (bool b : s.padMono) out << (b ? '1' : '0');
  out << ' ' << s.rowPan << ' ' << s.rowLevel << ' ' << (s.rowMono ? 1 : 0);
  for (const auto& pad : s.pads)
    for (double v : pad) out << ' ' << v;
}

template <size_t N>
bool readBits(const std::string& text, std::array<bool, N>& bits) {
  if (text.size() != N) return false;
  for (size_t i = 0; i < N; ++i) {
    if (text[i] != '0' && text[i] != '1') return false;
    bits[i] = text[i] == '1';
  }
  return true;
}

bool readRow(std::istream& in, Row& row) {
  Row::State s;
  int zone = 0, mono = 0;
  std::string locks, padMono;
  if (!(in >> s.seed >> zone >> s.favourite >> s.lastPressed >> s.lastPlayed >> s.spread >> s.newCount >> locks >>
        padMono >> s.rowPan >> s.rowLevel >> mono))
    return false;
  if (zone < 0 || zone >= kNumZones || !readBits(locks, s.locks) || !readBits(padMono, s.padMono)) return false;
  s.zone = static_cast<Zone>(zone);
  s.rowMono = mono != 0;
  for (auto& pad : s.pads)
    for (double& v : pad)
      if (!(in >> v) || !std::isfinite(v)) return false;
  row.restore(s);
  return true;
}

} // namespace

std::string Instrument::save() const {
  std::ostringstream out;
  out.imbue(std::locale::classic());
  out << std::setprecision(17) << "bloodline 2 " << nextSeed_ << "\nbench ";
  writeRow(out, bench_);
  out << '\n';
  for (int s = 0; s < kNumSlots; ++s) {
    out << "slot ";
    if (!slots_[s]) out << "blank";
    else { out << (open_[s] ? 1 : 0) << ' '; writeRow(out, *slots_[s]); }
    out << '\n';
  }
  return out.str();
}

bool Instrument::load(const std::string& text) {
  std::istringstream in(text);
  in.imbue(std::locale::classic());
  std::string word;
  int version = 0;
  std::uint32_t nextSeed = 0;
  if (!(in >> word >> version >> nextSeed) || word != "bloodline" || version != 2) return false;

  Row bench = bench_;
  if (!(in >> word) || word != "bench" || !readRow(in, bench)) return false;
  std::array<std::optional<Row>, kNumSlots> slots;
  std::array<bool, kNumSlots> open{};
  for (int s = 0; s < kNumSlots; ++s) {
    if (!(in >> word) || word != "slot" || !(in >> word)) return false;
    if (word == "blank") continue;
    if (word != "0" && word != "1") return false;
    Row r = bench_;
    if (!readRow(in, r)) return false;
    slots[s] = r;
    open[s] = word == "1";
  }
  bench_ = bench;
  slots_ = slots;
  open_ = open;
  nextSeed_ = nextSeed;
  return true;
}

} // namespace bloodline
